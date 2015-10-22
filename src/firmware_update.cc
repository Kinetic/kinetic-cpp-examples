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

// This sends a firmware file to the drive

#include <stdio.h>
#include <fstream>
#include <sstream>

#include "kinetic/kinetic.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

using std::make_shared;
using std::string;
using std::unique_ptr;

int main(int argc, char* argv[]) {

    if (argc != 3) {
        printf("Usage: %s <host> <input file name>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    const char* input_file_name = argv[2];

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = 8123;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    unique_ptr<kinetic::BlockingKineticConnection> blocking_connection;
    if (!kinetic_connection_factory.NewBlockingConnection(options, blocking_connection, 5).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    std::ifstream in(input_file_name, std::ios::in | std::ios::binary);
    std::ostringstream contents;
    if (in.fail()) {
        printf("Unable to read file\n");
        return 1;
    } else {
        contents << in.rdbuf();
    }

    auto value = make_shared<string>(contents.str());
    if (!blocking_connection->UpdateFirmware(value).ok()) {
        printf("Unable to send firmware\n");
        return 1;
    }

    printf("Done!\n");

    return 0;
}
