// This sends a firmware file to the drive

#include <stdio.h>
#import <fstream>

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

    HmacProvider hmac_provider;
    ValueFactory value_factory;
    MessageStreamFactory message_stream_factory(NULL, value_factory);
    kinetic::KineticConnectionFactory kinetic_connection_factory(hmac_provider,
            message_stream_factory);

    kinetic::KineticConnection* kinetic_connection;
    if (!kinetic_connection_factory.NewConnection(options, &kinetic_connection).ok()) {
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

    if (!kinetic_connection->FirmwareUpdate(contents.str()).ok()) {
        printf("Unable to send firmware\n");
        return 1;
    }

    printf("Done!\n");

    return 0;
}
