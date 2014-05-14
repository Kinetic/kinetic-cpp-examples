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

#include <stdio.h>

#include "kinetic/kinetic.h"

static const int kP2PBatchSize = 300;
using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

using std::shared_ptr;
using std::make_shared;
using std::string;
using std::unique_ptr;
using std::vector;
using std::pair;
using std::make_pair;

kinetic::P2PPushRequest prepare_request(const vector<kinetic::P2PPushOperation>& operations, const vector<pair<string, int>>& destinations, size_t currentDestination) {
    kinetic::P2PPushRequest request;
    if (currentDestination < destinations.size() - 1) {
        request.requests.push_back(prepare_request(operations, destinations, currentDestination + 1));
    }

    request.host = destinations[currentDestination].first;
    request.port = destinations[currentDestination].second;

    request.operations = operations;

    return request;
}

void dispatch_request(shared_ptr<kinetic::BlockingKineticConnection> connection,
        const vector<kinetic::P2PPushOperation>& operations,
        const vector<pair<string, int>>& destinations) {
    kinetic::P2PPushRequest request = prepare_request(operations, destinations, 0);

    unique_ptr<vector<kinetic::KineticStatus>> statuses(new vector<kinetic::KineticStatus>());
    if (!connection->P2PPush(request, statuses).ok()) {
        printf("Error pushing\n");
        exit(1);
    }

    for (auto it = statuses->begin(); it != statuses->end(); ++it) {
        if (it->ok()) {
            printf(".");
        } else {
            printf("%s", it->message().c_str());
        }
    }
    fflush(stdout);
}

int main(int argc, char* argv[]) {
    if (argc < 5 || argc % 2 != 1) {
        printf("%s: <source host> <source port> <destination host> <destination port> [<additional host> <additional port> ... ]\n", argv[0]);
        return 1;
    }

    const char* source_host = argv[1];
    int source_port = atoi(argv[2]);

    vector<pair<string, int>> destinations;

    printf("Copying from %s:%d", source_host, source_port);

    for (int i = 3; i < argc; i += 2) {
        auto destination = make_pair(argv[i], atoi(argv[i + 1]));
        destinations.push_back(destination);
        printf(" -> %s:%d", destination.first, destination.second);
    }
    printf("\n");


    kinetic::ConnectionOptions options;
    options.host = source_host;
    options.port = source_port;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    shared_ptr<kinetic::BlockingKineticConnection> blocking_connection;
    if(!kinetic_connection_factory.NewBlockingConnection(options, blocking_connection, 5).ok()){
        printf("Unable to connect\n");
        return 1;
    }

    vector<kinetic::P2PPushOperation> operations;

    // Build a key consisting of "FFFFFF...". In almost all cases this will come after the last
    // key in the drive
    string last_key;
    for (int i = 0; i < 4*1024; i++) {
        last_key += "\xFF";
    }

    // Iterate over all the keys and print them out
    for (kinetic::KeyRangeIterator it = blocking_connection->IterateKeyRange("", true, last_key, true, 100); it != kinetic::KeyRangeEnd(); ++it) {
        kinetic::P2PPushOperation op;
        op.key = *it;
        op.force = true;
        op.newKey = *it;
        operations.push_back(op);

        if (operations.size() > kP2PBatchSize) {
            dispatch_request(blocking_connection, operations, destinations);
            operations.clear();
        }
    }

    dispatch_request(blocking_connection, operations, destinations);

    printf("\n");



    return 0;
}

