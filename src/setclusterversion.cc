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

// This allows changing a drive's cluster version

#include <stdio.h>

#include "kinetic/kinetic.h"
#include "gflags/gflags.h"

using kinetic::Status;
using kinetic::KineticRecord;

using std::unique_ptr;

DEFINE_int64(new_cluster_version, 1, "New cluster version");

int example_main(
        std::shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection,
        std::shared_ptr<kinetic::BlockingKineticConnection> blocking_connection,
        int argc,
        char** argv) {
    printf("Setting cluster version to %" PRId64 "\n", FLAGS_new_cluster_version);

    kinetic::KineticStatus status = blocking_connection->SetClusterVersion(FLAGS_new_cluster_version);
    if (!status.ok()) {
        printf("Unable to set cluster version");
        if (status.statusCode() == kinetic::StatusCode::REMOTE_CLUSTER_VERSION_MISMATCH) {
          printf(". Incorrect cluster version; should be %" PRId64, status.expected_cluster_version());
        }
        printf("\n");
        return 1;
    }

    printf("Finished setting cluster version\n");

    // this is not the right cluster version, so the get should fail
    blocking_connection->SetClientClusterVersion(FLAGS_new_cluster_version + 1);

    unique_ptr<KineticRecord> result = unique_ptr<KineticRecord>();
    status = blocking_connection->Get("foo", result);
    if (status.statusCode() != kinetic::StatusCode::REMOTE_CLUSTER_VERSION_MISMATCH) {
        printf("Unexpectedly got %d\n", static_cast<int>(status.statusCode()));
        return 1;
    } else {
        printf("Correctly rejected a GET with incorrect cluster version; should have sent %" PRId64 "\n", status.expected_cluster_version());
    }

    return 0;
}
