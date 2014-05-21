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

// This deletes a file written using write_file_(non)blocking

#include <stdio.h>
#include <string>

#include "kinetic/kinetic.h"
#include "glog/logging.h"

using kinetic::KineticConnectionFactory;
using kinetic::KineticRecord;
using kinetic::KineticStatus;
using kinetic::SimpleCallbackInterface;
using kinetic::Status;

using std::make_shared;
using std::string;
using std::unique_ptr;
using std::shared_ptr;

class DeleteCallback : public SimpleCallbackInterface {
public:
    DeleteCallback(int* remaining) : remaining_(remaining) {};
    void Success() {
        printf(".");
        fflush(stdout);
        (*remaining_)--;
    }

    void Failure(KineticStatus error) {
        printf("Error: %d %s\n", static_cast<int>(error.statusCode()), error.message().c_str());
        exit(1);
    }
private:
    int* remaining_;

};

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    if (argc != 3) {
        printf("%s: <host> <kinetic key>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    const char* kinetic_key = argv[2];

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
    if(!blocking_connection->Get(string(kinetic_key), record).ok()) {
        printf("Unable to get metadata\n");
        return 1;
    }

    long long file_size = std::stoll(*(record->value()));
    printf("Deleting file of size %llu\n", file_size);

    char key_buffer[100];
    int remaining = 0;
    auto callback = make_shared<DeleteCallback>(&remaining);
    for (int64_t i = 0; i < file_size; i += 1024*1024) {
        unsigned int block_length = 1024*1024;
        if (i + block_length > file_size) {
            block_length = file_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10" PRId64, kinetic_key, i);
        remaining++;
        std::string key(key_buffer);
        // Use the Write Back persist mode for the data deletes, and then use Flush for the
        // metadata below to ensure everything is persisted by the time we exit.
        nonblocking_connection->Delete(key, "", kinetic::WriteMode::IGNORE_VERSION,
                callback, kinetic::PersistMode::WRITE_BACK);
    }

    remaining++;
    // Use the Flush persist mode to make sure all previous deletes are persisted
    nonblocking_connection->Delete(kinetic_key, "", kinetic::WriteMode::IGNORE_VERSION,
                callback, kinetic::PersistMode::FLUSH);


    fd_set read_fds, write_fds;
    int num_fds = 0;
    nonblocking_connection->Run(&read_fds, &write_fds, &num_fds);
    while (remaining > 0) {
        nonblocking_connection->Run(&read_fds, &write_fds, &num_fds);
    }

    printf("\nDone!\n");

    google::protobuf::ShutdownProtobufLibrary();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();

    return 0;
}
