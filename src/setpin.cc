// This allows setting and changing a drive's PIN

#include <stdio.h>

#include "protobufutil/message_stream.h"

#include "kinetic/connection_options.h"
#include "kinetic/kinetic_connection_factory.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

int main(int argc, char* argv[]) {
    if (argc != 4 && argc != 5) {
        printf("%s: <host> <port> <new pin>\n", argv[0]);
        printf("%s: <host> <port> <old pin> <new pin>\n", argv[0]);
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

    bool success;

    if (argc == 4) {
        success = connection->blocking().SetPin(argv[3]).ok();
    } else {
        const std::string pin(argv[3]);
        success = connection->blocking().SetPin(argv[4], &pin).ok();
    }

    if (success) {
        printf("Finished setting pin\n");
        return 0;
    } else {
        printf("Unable to change pin\n");
        return 1;
    }
}
