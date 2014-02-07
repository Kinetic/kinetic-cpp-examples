#include <stdio.h>

#include "kinetic/kinetic.h"

static const int kP2PBatchSize = 300;
using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

void dispatch_request(kinetic::BlockingKineticConnection& connection, const kinetic::P2PPushRequest& request) {
    unique_ptr<vector<kinetic::KineticStatus>> statuses(new vector<kinetic::KineticStatus>());
    if (!connection.P2PPush(request, statuses).ok()) {
        printf("Error pushing\n");
        exit(1);
    }

    for (auto it = statuses->begin(); it != statuses->end(); ++it) {
        if (it->ok()) {
            printf(".");
        } else {
            printf("X");
        }
    }
    fflush(stdout);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("%s: <source host> <source port> <destination host> <destination port>\n", argv[0]);
        return 1;
    }

    const char* source_host = argv[1];
    int source_port = atoi(argv[2]);
    const char* dest_host = argv[3];
    int dest_port = atoi(argv[4]);

    printf("Copying from %s:%d -> %s:%d\n", source_host, source_port, dest_host, dest_port);


    kinetic::ConnectionOptions options;
    options.host = source_host;
    options.port = source_port;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    unique_ptr<kinetic::ConnectionHandle> connection;
    if (!kinetic_connection_factory.NewConnection(options, 5, connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    kinetic::P2PPushRequest request;
    request.host = dest_host;
    request.port = dest_port;


    // Build a key consisting of "FFFFFF...". In almost all cases this will come after the last
    // key in the drive
    string last_key;
    for (int i = 0; i < 4*1024; i++) {
        last_key += "\xFF";
    }

    // Iterate over all the keys and print them out
    for (kinetic::KeyRangeIterator it = connection->blocking().IterateKeyRange("", true, last_key, true, 100); it != kinetic::KeyRangeEnd(); ++it) {
        kinetic::P2PPushOperation op;
        op.key = *it;
        op.force = true;
        op.newKey = *it;
        request.operations.push_back(op);

        if (request.operations.size() > kP2PBatchSize) {
            dispatch_request(connection->blocking(), request);
            request.operations.clear();
        }
    }

    dispatch_request(connection->blocking(), request);

    printf("\n");



    return 0;
}

