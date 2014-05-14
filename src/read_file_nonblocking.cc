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

// This reads a file stored using write_file_blocking.cc using the nonblocking API

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <memory>

#include "kinetic/kinetic.h"

#include "glog/logging.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;
using kinetic::ConnectionOptions;
using kinetic::GetCallbackInterface;
using kinetic::NonblockingKineticConnection;
using kinetic::KineticStatus;

using std::shared_ptr;
using std::make_shared;
using std::vector;
using std::unique_ptr;
using std::string;

class Callback : public GetCallbackInterface {
public:
    Callback(char* buffer, unsigned int expected_length, int* remaining) : buffer_(buffer),
        expected_length_(expected_length), remaining_(remaining) {};
    void Success(const std::string &key, std::unique_ptr<KineticRecord> record) {
        if(expected_length_ != record->value()->size()) {
            printf("Received value chunk of wrong size\n");
            exit(1);
        }
        record->value()->copy(buffer_, expected_length_);
        printf(".");
        fflush(stdout);
        (*remaining_)--;
    }
    void Failure(KineticStatus error) {
        printf("Error: %d %s\n", static_cast<int>(error.statusCode()), error.message().c_str());
        exit(1);
    }
private:
    char* buffer_;
    unsigned int expected_length_;
    int* remaining_;
};

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    if (argc != 4) {
        printf("%s: <host> <kinetic key> <output file name>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    const char* kinetic_key = argv[2];
    const char* output_file_name = argv[3];

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = 8123;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection;
    if (!kinetic_connection_factory.NewNonblockingConnection(options, nonblocking_connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    shared_ptr<kinetic::BlockingKineticConnection> blocking_connection =
            make_shared<kinetic::BlockingKineticConnection>(nonblocking_connection, 5);

    std::unique_ptr<KineticRecord> record;
    if(!blocking_connection->Get(kinetic_key, record).ok()) {
        printf("Unable to get metadata\n");
        return 1;
    }

    long long file_size = std::stoll(*(record->value()));
    printf("Reading file of size %llu\n", file_size);


    int file = open(output_file_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(!file) {
        printf("Unable to open output file\n");
        return 1;
    }
    if((off_t)(file_size - 1) != lseek(file, file_size - 1, SEEK_SET)) {
        printf("Unable to seek in file\n");
        return 1;
    }
    if(write(file, " ", 1) != 1) {
        printf("Unable to resize file\n");
        return 1;
    }

    char* output_buffer = (char*)mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    if (output_buffer == MAP_FAILED) {
        printf("Unable to mmap file errno=%d\n", errno);
        return 1;
    }
    char key_buffer[100];
    int remaining = 0;
    fd_set read_fds, write_fds;
    int num_fds = 0;
    vector<shared_ptr<Callback> > callbacks;
    for (int64_t i = 0; i < file_size; i += 1024*1024) {
        unsigned int block_length = 1024*1024;
        if (i + block_length > file_size) {
            block_length = file_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10" PRId64, kinetic_key, i);
        remaining++;
        shared_ptr<Callback> callback(new Callback(output_buffer + i, block_length, &remaining));
        nonblocking_connection->Get(string(key_buffer), callback);
        nonblocking_connection->Run(&read_fds, &write_fds, &num_fds);
        callbacks.push_back(std::move(callback));
    }

    nonblocking_connection->Run(&read_fds, &write_fds, &num_fds);
    while (remaining > 0) {
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        int number_ready_fds = select(num_fds + 1, &read_fds, &write_fds, NULL, &tv);
        if (number_ready_fds < 0) {
            // select() returned an error
            printf("i/o error: %s\n", strerror(errno));
            return 1;
        } else if (number_ready_fds == 0) {
            printf("connection timed out\n");
            return 1;
        }

        nonblocking_connection->Run(&read_fds, &write_fds, &num_fds);
    }

    CHECK(!close(file));

    printf("\nDone!\n");

    google::protobuf::ShutdownProtobufLibrary();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();

    return 0;
}
