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

#ifndef COMMAND_LINE_FLAGS_H_
#define COMMAND_LINE_FLAGS_H_

#include "kinetic/kinetic.h"

#include "gflags/gflags.h"

DEFINE_string(host, "localhost", "Kinetic Host");
DEFINE_uint64(port, 8123, "Kinetic Port");
DEFINE_uint64(timeout, 30, "Timeout");
DEFINE_uint64(user_id, 1, "Kinetic User ID");
DEFINE_string(hmac_key, "asdfasdf", "Kinetic User HMAC key");
DEFINE_int64(cluster_version, 0, "Kinetic Cluster Version");

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
    if (!kinetic_connection_factory.NewNonblockingConnection(options, nonblocking_connection).ok() ||
        !kinetic_connection_factory.NewBlockingConnection(options, blocking_connection, FLAGS_timeout).ok() ){
        printf("Unable to connect\n");
        return false;
    }
    blocking_connection->SetClientClusterVersion(FLAGS_cluster_version);
    return true;
}

#endif  // COMMAND_LINE_FLAGS_H_
