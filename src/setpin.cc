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

// This allows setting and changing a drive's PIN

#include <stdio.h>

#include "kinetic/kinetic.h"
#include "gflags/gflags.h"

using kinetic::Status;
using kinetic::KineticRecord;

using std::string;
using std::make_shared;
using std::unique_ptr;

DEFINE_string(new_pin, "", "New PIN");
DEFINE_string(old_pin, "", "Old PIN");

int example_main(
        std::shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection,
        std::shared_ptr<kinetic::BlockingKineticConnection> blocking_connection,
        int argc,
        char** argv) {
    bool success;

    if (FLAGS_old_pin.empty()) {
        success = blocking_connection->SetErasePIN(make_shared<string>(FLAGS_new_pin)).ok();
    } else {
        auto pin = make_shared<string>(FLAGS_old_pin);
        success = blocking_connection->SetErasePIN(make_shared<string>(FLAGS_new_pin), pin).ok();
    }

    if (success) {
        printf("Finished setting erase pin\n");
        return 0;
    } else {
        printf("Unable to change erase pin\n");
        return 1;
    }
}
