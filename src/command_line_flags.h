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

#ifndef COMMAND_LINE_FLAGS_H_
#define COMMAND_LINE_FLAGS_H_

#include "kinetic/kinetic.h"

#include "gflags/gflags.h"

DEFINE_string(host, "localhost", "Kinetic Host");
DEFINE_uint64(port, 8123, "Kinetic Port");
DEFINE_uint64(timeout, 30, "Timeout");
DEFINE_uint64(user_id, 1, "Kinetic User ID");
DEFINE_string(hmac_key, "asdfasdf", "Kinetic User HMAC key");

bool parse_flags(int *argc,
        char*** argv,
        std::shared_ptr<kinetic::NonblockingKineticConnection>& nonblocking_connection,
        std::shared_ptr<kinetic::BlockingKineticConnection>& blocking_connection) {
    google::ParseCommandLineFlags(argc, argv, true);

    kinetic::ConnectionOptions options;
    options.host = FLAGS_host;
    options.port = FLAGS_port;
    options.user_id = FLAGS_user_id;
    options.hmac_key = FLAGS_hmac_key;

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    if (!kinetic_connection_factory.NewNonblockingConnection(options, nonblocking_connection).ok()) {
        printf("Unable to connect\n");
        return false;
    }
    blocking_connection = std::make_shared<kinetic::BlockingKineticConnection>(nonblocking_connection, FLAGS_timeout);

    return true;
}

#endif  // COMMAND_LINE_FLAGS_H_
