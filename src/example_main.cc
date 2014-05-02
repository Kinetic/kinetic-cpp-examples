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

int example_main(std::unique_ptr<kinetic::ConnectionHandle> connection, int argc, char** argv);

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);

    std::unique_ptr<kinetic::ConnectionHandle> connection;
    if (!parse_flags(&argc, &argv, connection)) {
        return 1;
    }

    int ret = example_main(move(connection), argc, argv);

    google::protobuf::ShutdownProtobufLibrary();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();

    return ret;
}
