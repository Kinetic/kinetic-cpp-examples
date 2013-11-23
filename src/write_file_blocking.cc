#include <stdio.h>
#include <glog/logging.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

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

    HmacProvider hmac_provider;
    ValueFactory value_factory;
    MessageStreamFactory message_stream_factory(NULL, value_factory);
    kinetic::KineticConnectionFactory kinetic_connection_factory(hmac_provider,
            message_stream_factory);

    kinetic::KineticConnection* kinetic_connection;
    CHECK(
    kinetic_connection_factory.NewConnection(options, &kinetic_connection).ok());

    int file = open(input_file_name, O_RDONLY);
    struct stat inputfile_stat;
    fstat(file, &inputfile_stat);
    char* inputfile_data = (char*)mmap(0, inputfile_stat.st_size, PROT_READ, MAP_SHARED, file, 0);
    char key_buffer[100];
    for (int i = 0; i < inputfile_stat.st_size; i += 1024*1024) {
        int value_size = 1024*1024;
        if (i + value_size > inputfile_stat.st_size) {
            value_size = inputfile_stat.st_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10d", kinetic_key, i);

        std::string key(key_buffer);
        std::string value(inputfile_data + i, value_size);
        CHECK(
        kinetic_connection->Put(key, "",
                KineticRecord(value, "", "", Message_Algorithm_SHA1)).ok());
        printf(".");
        fflush(stdout);
    }
    printf("\n");


    CHECK(kinetic_connection->Put(kinetic_key, "", KineticRecord(std::to_string(inputfile_stat.st_size), "", "", Message_Algorithm_SHA1)).ok());

    CHECK(!close(file));

    printf("Done!\n");

    return 0;
}
