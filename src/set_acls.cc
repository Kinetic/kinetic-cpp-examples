// This sets some hard-coded ACLs in the given drive

#include <stdio.h>

#include "kinetic/kinetic.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;
using kinetic::ACL;
using kinetic::Domain;

using std::list;
using std::make_shared;
using std::unique_ptr;

int example_main(unique_ptr<kinetic::ConnectionHandle> connection, int argc, char* argv[]) {
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

    auto acls = make_shared<list<ACL>>();
    acls->push_back(acl2);
    acls->push_back(acl1);

    printf("Setting ACLs...");

    if (connection->blocking().SetACLs(acls).ok()) {
        printf("Success!\n");
        return 0;
    } else {
        printf("Error\n");
        return 1;
    }
}
