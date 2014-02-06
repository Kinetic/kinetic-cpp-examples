// This allows changing a drive's cluster version

#include <stdio.h>

#include "kinetic/kinetic.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

using std::unique_ptr;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("%s: <host> <port> <new cluster version>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    int port = atoi(argv[2]);
    int new_cluster_version = atoi(argv[3]);

    printf("Setting cluster version of %s:%d to %d\n", host, port, new_cluster_version);

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

    if (!(connection->blocking().SetClusterVersion(new_cluster_version).ok())) {
        printf("Unable to set cluster version\n");
        return 1;
    }

    printf("Finished setting cluster version\n");

    return 0;
}
