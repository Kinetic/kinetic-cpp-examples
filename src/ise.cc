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

// This shows how to erase a drive

#include <stdio.h>

#include "kinetic/kinetic.h"
#include "gflags/gflags.h"

using kinetic::Status;
using kinetic::KineticStatus;
using kinetic::KineticRecord;

using std::shared_ptr;
using std::string;

DEFINE_string(pin, "", "PIN");

int example_main(
        std::shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection,
        std::shared_ptr<kinetic::BlockingKineticConnection> blocking_connection,
        int argc,
        char** argv) {
    shared_ptr<string> pin(new string(FLAGS_pin));

    KineticStatus status = blocking_connection->InstantErase(pin);
    bool success = status.ok();

    if (!success) {
        printf("Unable to execute InstantErase: %d %s\n", static_cast<int>(status.statusCode()), status.message().c_str());
        return 1;
    }

    status = blocking_connection->SecureErase(pin);
    success = status.ok();

    if (!success) {
        printf("Unable to execute SecureErase: %d %s\n", static_cast<int>(status.statusCode()), status.message().c_str());
        return 1;
    }

    printf("Finished ISE\n");

    return 0;
}
