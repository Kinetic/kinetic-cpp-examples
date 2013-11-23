// This shows how to erase a drive

#include <stdio.h>

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
    if (argc != 3 && argc != 4) {
        printf("%s: <host> <port>\n", argv[0]);
        printf("%s: <host> <port> <pin>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    int port = atoi(argv[2]);

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = port;
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

    std::string* pin = NULL;

    if (argc == 3) {
        printf("Performing ISE on %s:%d\n", host, port);
    } else {
        pin = new std::string(argv[3]);
        printf("Performing ISE on %s:%d with pin %s\n", host, port, argv[3]);
    }

    bool success = kinetic_connection->InstantSecureErase(pin).ok();

    if (pin) {
        delete pin;
    }

    if (!success) {
        printf("Unable to execute ISE\n");
        return 1;
    }

    printf("Finished ISE\n");

    return 0;
}
