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
