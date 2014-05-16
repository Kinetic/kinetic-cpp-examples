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

    KineticStatus status = blocking_connection->InstantSecureErase(pin);
    bool success = status.ok();

    if (!success) {
        printf("Unable to execute ISE: %d %s\n", static_cast<int>(status.statusCode()), status.message().c_str());
        return 1;
    }

    printf("Finished ISE\n");

    return 0;
}
