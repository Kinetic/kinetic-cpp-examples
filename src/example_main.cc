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

#include "kinetic/kinetic.h"
#include "glog/logging.h"

#include "command_line_flags.h"

int example_main(
        std::shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection,
        std::shared_ptr<kinetic::BlockingKineticConnection> blocking_connection,
        int argc,
        char** argv);

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);

    std::shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection;
    std::shared_ptr<kinetic::BlockingKineticConnection> blocking_connection;

    if (!parse_flags(&argc, &argv, nonblocking_connection, blocking_connection)) {
        return 1;
    }

    int ret = example_main(nonblocking_connection, blocking_connection, argc, argv);

    // Manually reset connections before shutting down logging to avoid logging to
    // an uninitialized logger from a destructor
    nonblocking_connection.reset();
    blocking_connection.reset();

    google::protobuf::ShutdownProtobufLibrary();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();

    return ret;
}
