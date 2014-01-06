// This sets some hard-coded ACLs in the given drive

#include <stdio.h>

#include "protobufutil/message_stream.h"

#include "kinetic/connection_options.h"
#include "kinetic/kinetic_connection_factory.h"
#include "value_factory.h"

using com::seagate::kinetic::HmacProvider;
using com::seagate::kinetic::proto::Message;
using com::seagate::kinetic::proto::Message_MessageType_GET;
using com::seagate::kinetic::proto::Message_Algorithm_SHA1;
using com::seagate::kinetic::ValueFactory;
using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;
using kinetic::ACL;
using kinetic::Domain;
using palominolabs::protobufutil::MessageStreamFactory;

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

    kinetic::BlockingKineticConnection* kinetic_connection;
    if (!kinetic_connection_factory.NewBlockingConnection(options, &kinetic_connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    Domain domain1 = {.offset = 0, .value = "", .roles = {kinetic::GETLOG}};
    std::list<Domain> acl1_domains = {
        domain1,
    };
    ACL acl1;
    acl1.client_id = 1000;
    acl1.hmac_key = "foobarbaz";
    acl1.domains = acl1_domains;

    Domain domain2 = {.offset = 0, .value = "", .roles = {
            kinetic::READ,
            kinetic::WRITE,
            kinetic::DELETE,
            kinetic::RANGE,
            kinetic::SETUP,
            kinetic::P2POP,
            kinetic::GETLOG,
            kinetic::SECURITY},
    };
    std::list<Domain> acl2_domains = {
        domain2
    };
    ACL acl2;
    acl2.client_id = 1;
    acl2.hmac_key = "asdfasdf";
    acl2.domains = acl2_domains;

    std::list<ACL> acls;
    acls.push_back(acl1);
    acls.push_back(acl2);

    printf("Setting ACLs...");

    if (kinetic_connection->SetACLs(acls).ok()) {
        printf("Success!\n");
        return 0;
    } else {
        printf("Error\n");
        return 1;
    }
}
