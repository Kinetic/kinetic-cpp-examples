// This writes the given file to a drive as a series of 1MB chunks and a metadata key

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <inttypes.h>

#include "protobufutil/message_stream.h"

#include "kinetic/connection_options.h"
#include "kinetic/kinetic_connection_factory.h"

using com::seagate::kinetic::proto::Message_Algorithm_SHA1;
using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    if (argc != 4) {
        printf("Usage: %s <host> <kinetic key> <input file name>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    const char* kinetic_key = argv[2];
    const char* input_file_name = argv[3];

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = 8123;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    kinetic::ConnectionHandle* connection;
    if (!kinetic_connection_factory.NewConnection(options, &connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    int file = open(input_file_name, O_RDONLY);
    struct stat inputfile_stat;
    fstat(file, &inputfile_stat);
    char* inputfile_data = (char*)mmap(0, inputfile_stat.st_size, PROT_READ, MAP_SHARED, file, 0);
    char key_buffer[100];
    for (int64_t i = 0; i < inputfile_stat.st_size; i += 1024*1024) {
        int value_size = 1024*1024;
        if (i + value_size > inputfile_stat.st_size) {
            value_size = inputfile_stat.st_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10" PRId64, kinetic_key, i);

        std::string key(key_buffer);
        std::string value(inputfile_data + i, value_size);
        if(!connection->blocking().Put(
                key,
                "",
                kinetic::IGNORE_VERSION,
                KineticRecord(value, "", "", Message_Algorithm_SHA1)).ok()) {
            printf("Unable to write a chunk\n");
            return 1;
        }
        printf(".");
        fflush(stdout);
    }
    printf("\n");


    if (!connection->blocking().Put(
            kinetic_key,
            "",
            kinetic::IGNORE_VERSION,
            KineticRecord(std::to_string(inputfile_stat.st_size), "", "", Message_Algorithm_SHA1)).ok()) {
        printf("Unable to write metadata\n");
        return 1;
    }

    if (close(file)) {
        printf("Unable to close file\n");
        return 1;
    }

    delete connection;

    printf("Done!\n");

    google::protobuf::ShutdownProtobufLibrary();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();

    return 0;
}
