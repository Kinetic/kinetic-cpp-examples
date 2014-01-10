// This shows how to erase a drive

#include <stdio.h>

#include "protobufutil/message_stream.h"

#include "kinetic/connection_options.h"
#include "kinetic/kinetic_connection_factory.h"
#include "value_factory.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

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

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    kinetic::ConnectionHandle* connection;
    if (!kinetic_connection_factory.NewConnection(options, &connection).ok()) {
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

    bool success = connection->blocking().InstantSecureErase(pin).ok();

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
