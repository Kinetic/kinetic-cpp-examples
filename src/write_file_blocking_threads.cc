/*
 * kinetic-cpp-examples
 * Copyright (C) 2014 Seagate Technology.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

// This writes the given file to a drive as a series of 1MB chunks and a metadata key

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <iostream>
#include <vector>
#include <thread>

#include "glog/logging.h"

#include "kinetic/kinetic.h"

using com::seagate::kinetic::client::proto::Message_Algorithm_SHA1;
using kinetic::KineticConnectionFactory;
using kinetic::BlockingKineticConnection;
using kinetic::Status;
using kinetic::KineticStatus;
using kinetic::KineticRecord;
using kinetic::ConnectionOptions;

using std::make_shared;
using std::move;
using std::shared_ptr;
using std::unique_ptr;
using std::cout;
using std::endl;
using std::vector;
using std::thread;

void put_range(int64_t start, int64_t end, int64_t total_size, const char* kinetic_key,
        const char* input_file_name, shared_ptr<kinetic::BlockingKineticConnection> blocking_connection);

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    if (argc != 4) {
        printf("Usage: %s <host> <kinetic key> <input file name>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    const char* kinetic_key = argv[2];
    const char* input_file_name = argv[3];

    ConnectionOptions options;
    options.host = host;
    options.port = 8123;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    shared_ptr<kinetic::BlockingKineticConnection> blocking_connection;
    if (!kinetic_connection_factory.NewThreadsafeBlockingConnection(options, blocking_connection, 5).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    int file = open(input_file_name, O_RDONLY);
    struct stat inputfile_stat;
    fstat(file, &inputfile_stat);

    if (close(file)) {
        printf("Unable to close file\n");
        return 1;
    }

    // the definition of off_t varies a lot between OSs
    printf("file is %lld\n", (long long) inputfile_stat.st_size);

    vector<unique_ptr<thread>> threads;
    int chunk = 100 * 1024 * 1024;
    for (int64_t i = 0; i < inputfile_stat.st_size; i += chunk ) {
        int64_t end = i + chunk;
        if (end > inputfile_stat.st_size) {
            end = inputfile_stat.st_size;
        }
        unique_ptr<thread> t(new thread(put_range, i, end, inputfile_stat.st_size, kinetic_key,
                    input_file_name, blocking_connection));
        threads.push_back(move(t));
    }

    printf("%" PRIuPTR " threads spawned\n", threads.size());

    for (auto ci = threads.begin(); ci != threads.end(); ++ci) {
        (*ci)->join();
    }

    printf("All threads joined\n");

    // Use the Flush persist mode to make sure all previous writes are persisted
    if (!blocking_connection->Put(
            kinetic_key,
            "",
            kinetic::WriteMode::IGNORE_VERSION,
            KineticRecord(std::to_string(inputfile_stat.st_size), "", "", Message_Algorithm_SHA1),
            kinetic::PersistMode::FLUSH).ok()) {
        printf("Unable to write metadata\n");
        return 1;
    }

    printf("Done!\n");

    google::protobuf::ShutdownProtobufLibrary();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();

    return 0;
}

void put_range(int64_t start, int64_t end, int64_t total_size, const char* kinetic_key,
        const char* input_file_name, shared_ptr<kinetic::BlockingKineticConnection> blocking_connection) {
    printf("thread writing %" PRId64 " to %" PRId64 "\n", start, end);

    int file = open(input_file_name, O_RDONLY);
    struct stat inputfile_stat;
    fstat(file, &inputfile_stat);
    char* inputfile_data = (char*)mmap(0, total_size, PROT_READ, MAP_SHARED, file, 0);
    char key_buffer[100];
    for (int64_t i = start; i < end; i += 1024*1024) {
        int64_t value_size = 1024*1024;
        if (i + value_size > end) {
            value_size = end - i + 1;
        }

        sprintf(key_buffer, "%s-%10" PRId64, kinetic_key, i);

        std::string key(key_buffer);
        std::string value(inputfile_data + i, value_size);
        // Use the Write Back persist mode for the data puts, and then use Flush for the metadata
        // below to ensure everything is persisted by the time we exit.
        KineticStatus status = blocking_connection->Put(
                    key,
                    "",
                    kinetic::WriteMode::IGNORE_VERSION,
                    KineticRecord(value, "", "", Message_Algorithm_SHA1),
                    kinetic::PersistMode::WRITE_BACK);
        if(!status.ok()) {
            printf("Unable to write chunk: %d %s\n", static_cast<int>(status.statusCode()),
                status.message().c_str());
            return;
        }
        printf(".");
        fflush(stdout);
    }
    printf("\nThread done\n");

    if (close(file)) {
        printf("Unable to close file\n");
    }
}
