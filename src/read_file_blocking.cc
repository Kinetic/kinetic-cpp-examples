// This reads a file stored using write_file_blocking.cc using the blocking API

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <inttypes.h>

#include "protobufutil/message_stream.h"

#include "connection_options.h"
#include "hmac_provider.h"
#include "kinetic_connection_factory.h"
#include "value_factory.h"

using com::seagate::kinetic::HmacProvider;
using com::seagate::kinetic::proto::Message;
using com::seagate::kinetic::proto::Message_MessageType_GET;
using com::seagate::kinetic::proto::Message_Algorithm_SHA1;
using com::seagate::kinetic::ValueFactory;
using kinetic::KineticConnection;
using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;
using palominolabs::protobufutil::MessageStreamFactory;

int main(int argc, char* argv[]) {

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

    HmacProvider hmac_provider;
    ValueFactory value_factory;
    MessageStreamFactory message_stream_factory(NULL, value_factory);
    kinetic::KineticConnectionFactory kinetic_connection_factory(hmac_provider,
            message_stream_factory);

    kinetic::KineticConnection* kinetic_connection;
    if(!kinetic_connection_factory.NewConnection(options, &kinetic_connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }


    std::string value;
    if(!kinetic_connection->Get(kinetic_key, &value, NULL, NULL).ok()) {
        printf("Unable to get metadata\n");
        return 1;
    }

    ssize_t file_size = std::stoll(value);
    printf("Reading file of size %zd\n", file_size);

    int file = open(output_file_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(!file) {
        printf("Unable to open output file\n");
        return 1;
    }
    if(file_size - 1 != lseek(file, file_size - 1, SEEK_SET)) {
        printf("Unable to seek\n");
        return 1;
    }
    if(write(file, " ", 1) != 1) {
        printf("Unable to resize file\n");
        return 1;
    }
    char* output_buffer = (char*)mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    char key_buffer[100];
    for (int64_t i = 0; i < file_size; i += 1024*1024) {
        int block_length = 1024*1024;
        if (i + block_length > file_size) {
            block_length = file_size - i;
        }

        std::string value;
        sprintf(key_buffer, "%s-%10" PRId64, kinetic_key, i);
        std::string key(key_buffer);

        if(!kinetic_connection->Get(key, &value, NULL, NULL).ok()) {
            printf("Unable to get chunk\n");
            return 1;
        }
        value.copy(output_buffer + i, block_length);
        printf(".");
        fflush(stdout);
    }
    printf("\n");

    if(close(file)) {
        printf("Unable to close file\n");
        return 1;
    }


    printf("Done!\n");

    return 0;
}
