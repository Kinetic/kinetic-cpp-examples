// This allows changing a drive's cluster version

#include <stdio.h>

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

    if (!(kinetic_connection->SetClusterVersion(new_cluster_version).ok())) {
        printf("Unable to set cluster version\n");
        return 1;
    }

    printf("Finished setting cluster version\n");

    return 0;
}
