#include <stdio.h>
#include <glog/logging.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

#include "protobufutil/message_stream.h"

#include "connection_options.h"
#include "hmac_provider.h"
#include "kinetic_connection_factory.h"
#include "kinetic_connection.h"
#include "kinetic_record.h"
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
    CHECK(
    kinetic_connection_factory.NewConnection(options, &kinetic_connection).ok());


    std::string value;
    CHECK(kinetic_connection->Get(kinetic_key, &value, NULL, NULL).ok());

    int file_size = std::stoi(value);
    printf("Reading file of size %d\n", file_size);



    int file = open(output_file_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    PCHECK(file);
    PCHECK(file_size - 1 == lseek(file, file_size - 1, SEEK_SET));
    PCHECK(1 == write(file, " ", 1));
    char* output_buffer = (char*)mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    char key_buffer[100];
    for (int i = 0; i < file_size; i += 1024*1024) {
        int block_length = 1024*1024;
        if (i + block_length > file_size) {
            block_length = file_size - i;
        }

        std::string value;
        sprintf(key_buffer, "%s-%10d", kinetic_key, i);
        std::string key(key_buffer);

        CHECK(kinetic_connection->Get(key, &value, NULL, NULL).ok());
        value.copy(output_buffer + i, block_length);
        printf(".");
        fflush(stdout);
    }
    printf("\n");

    CHECK(!close(file));


    printf("Done!\n");

    return 0;
}
