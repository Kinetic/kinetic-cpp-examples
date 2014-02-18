// This shows how to erase a drive

#include <stdio.h>

#include "kinetic/kinetic.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticStatus;
using kinetic::KineticRecord;

using std::shared_ptr;
using std::string;
using std::unique_ptr;

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

    unique_ptr<kinetic::ConnectionHandle> connection;
    if (!kinetic_connection_factory.NewConnection(options, 5, connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    shared_ptr<string> pin;

    if (argc == 3) {
        printf("Performing ISE on %s:%d\n", host, port);
    } else {
        pin.reset(new std::string(argv[3]));
        printf("Performing ISE on %s:%d with pin %s\n", host, port, argv[3]);
    }

    KineticStatus status = connection->blocking().InstantSecureErase(pin);
    bool success = status.ok();

    if (!success) {
        printf("Unable to execute ISE: %d %s\n", static_cast<int>(status.statusCode()), status.message().c_str());
        return 1;
    }

    printf("Finished ISE\n");

    return 0;
}
