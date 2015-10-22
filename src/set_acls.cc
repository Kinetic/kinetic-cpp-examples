/**
 * Copyright 2013-2015 Seagate Technology LLC.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at
 * https://mozilla.org/MP:/2.0/.
 * 
 * This program is distributed in the hope that it will be useful,
 * but is provided AS-IS, WITHOUT ANY WARRANTY; including without 
 * the implied warranty of MERCHANTABILITY, NON-INFRINGEMENT or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the Mozilla Public 
 * License for more details.
 *
 * See www.openkinetic.org for more project information
 */

// This sets some hard-coded ACLs in the given drive

#include <stdio.h>

#include "kinetic/kinetic.h"

using kinetic::Status;
using kinetic::KineticRecord;
using kinetic::ACL;
using kinetic::Scope;

using std::list;
using std::make_shared;
using std::unique_ptr;

int example_main(
        std::shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection,
        std::shared_ptr<kinetic::BlockingKineticConnection> blocking_connection,
        int argc,
        char** argv) {
    Scope scope1 = {.offset = 0, .value = "", .permissions = {kinetic::GETLOG}};
    std::list<Scope> acl1_scopes = {
        scope1,
    };
    ACL acl1;
    acl1.identity = 1000;
    acl1.hmac_key = "foobarbaz";
    acl1.scopes = acl1_scopes;

    Scope scope2 = {.offset = 0, .value = "", .permissions = {
            kinetic::READ,
            kinetic::WRITE,
            kinetic::DELETE,
            kinetic::RANGE,
            kinetic::SETUP,
            kinetic::P2POP,
            kinetic::GETLOG,
            kinetic::SECURITY},
    };
    std::list<Scope> acl2_scopes = {
        scope2
    };
    ACL acl2;
    acl2.identity = 1;
    acl2.hmac_key = "asdfasdf";
    acl2.scopes = acl2_scopes;

    auto acls = make_shared<list<ACL>>();
    acls->push_back(acl2);
    acls->push_back(acl1);

    printf("Setting ACLs...");

    if (blocking_connection->SetACLs(acls).ok()) {
        printf("Success!\n");
        return 0;
    } else {
        printf("Error\n");
        return 1;
    }
}
