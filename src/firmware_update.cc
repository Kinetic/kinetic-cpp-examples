// This sends a firmware file to the drive

#include <stdio.h>
#include <fstream>

#include "protobufutil/message_stream.h"

#include "kinetic/connection_options.h"
#include "kinetic/kinetic_connection_factory.h"
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

    if (argc != 3) {
        printf("Usage: %s <host> <input file name>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    const char* input_file_name = argv[2];

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = 8123;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    kinetic::BlockingKineticConnection* kinetic_connection;
    if (!kinetic_connection_factory.NewBlockingConnection(options, &kinetic_connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    std::ifstream in(input_file_name, std::ios::in | std::ios::binary);
    std::ostringstream contents;
    if (in.fail()) {
        printf("Unable to read file\n");
        return 1;
    } else {
        contents << in.rdbuf();
    }

    if (!kinetic_connection->UpdateFirmware(contents.str()).ok()) {
        printf("Unable to send firmware\n");
        return 1;
    }

    printf("Done!\n");

    return 0;
}
