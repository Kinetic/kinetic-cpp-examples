// This deletes a file written using write_file_(non)blocking

#include <stdio.h>

#include "protobufutil/message_stream.h"

#include "kinetic/connection_options.h"
#include "kinetic/kinetic_connection_factory.h"
#include "socket_wrapper.h"

using com::seagate::kinetic::proto::Message_Algorithm_SHA1;
using kinetic::KineticConnectionFactory;
using kinetic::KineticRecord;
using kinetic::NonblockingError;
using kinetic::SimpleCallbackInterface;
using kinetic::Status;


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

    kinetic::ConnectionHandle* connection;
    if(!kinetic_connection_factory.NewConnection(options, &connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    std::unique_ptr<KineticRecord> record;
    if(!connection->blocking().Get(kinetic_key, &record).ok()) {
        printf("Unable to get metadata\n");
        return 1;
    }

    long long file_size = std::stoll(record->value());
    printf("Deleting file of size %llu\n", file_size);

    char key_buffer[100];
    for (int64_t i = 0; i < file_size; i += 1024*1024) {
        unsigned int block_length = 1024*1024;
        if (i + block_length > file_size) {
            block_length = file_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10" PRId64, kinetic_key, i);
        std::string key(key_buffer);
        if (connection->blocking().Delete(key, "", kinetic::IGNORE_VERSION).ok()) {
            printf(".");
        } else {
            printf("X");
        }
        fflush(stdout);
    }

    if (!connection->blocking().Delete(kinetic_key, "", kinetic::IGNORE_VERSION).ok()) {
        printf("Unable to delete metadata\n");
    }

    printf("\nDone!\n");

    delete connection;
    google::protobuf::ShutdownProtobufLibrary();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();

    return 0;
}
