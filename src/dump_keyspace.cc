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

// This shows how to iterate over the keyspace

#include <stdio.h>

#include "kinetic/kinetic.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

using std::unique_ptr;
using std::string;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("%s: <host> <port>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    int port = atoi(argv[2]);

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = port;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    unique_ptr<kinetic::BlockingKineticConnection> blocking_connection;
    if (!kinetic_connection_factory.NewBlockingConnection(options, blocking_connection, 5).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    // Build a key consisting of "FFFFFF...". In almost all cases this will come after the last
    // key in the drive
    string last_key;
    for (int i = 0; i < 800*1024; i++) {
        last_key += "\xFF";
    }

    // Iterate over all the keys and print them out
    for (kinetic::KeyRangeIterator it = blocking_connection->IterateKeyRange("", true, last_key, true, 100); it != kinetic::KeyRangeEnd(); ++it) {
        printf("%s\n", it->c_str());
    }

    return 0;
}
