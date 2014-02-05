// This shows how to iterate over the keyspace

#include <stdio.h>

#include "kinetic/kinetic.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

using std::shared_ptr;
using std::string;
using std::unique_ptr;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("%s: <host> <port>\n", argv[0]);
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
    if (!kinetic_connection_factory.NewConnection(options, connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    // Build a key consisting of "FFFFFF...". In almost all cases this will come after the last
    // key in the drive
    string last_key;
    for (int i = 0; i < 800*1024; i++) {
        last_key += "\xFF";
    }

    // Iterate over all the keys and print them out
    for (kinetic::KeyRangeIterator it = connection->blocking().IterateKeyRange("", true, last_key, true, 100); it != kinetic::KeyRangeEnd(); ++it) {
        printf("%s\n", it->c_str());
    }

    return 0;
}
