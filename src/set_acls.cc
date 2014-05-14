/*
 * kinetic-cpp-examples
 * Copyright (C) 2014 Seagate Technology.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
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
