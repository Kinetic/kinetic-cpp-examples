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

    if (!(blocking_connection->SetClusterVersion(FLAGS_new_cluster_version).ok())) {
        printf("Unable to set cluster version\n");
        return 1;
    }

    printf("Finished setting cluster version\n");

    // this is not the right cluster version, so the get should fail
    blocking_connection->SetClientClusterVersion(FLAGS_new_cluster_version + 1);

    unique_ptr<KineticRecord> result = unique_ptr<KineticRecord>();
    kinetic::StatusCode code = blocking_connection->Get("foo", result).statusCode();
    if (code != kinetic::StatusCode::REMOTE_CLUSTER_VERSION_MISMATCH) {
        printf("Unexpectedly got %d\n", static_cast<int>(code));
        return 1;
    } else {
        printf("Correctly rejected a GET with incorrect cluster version\n");
    }

    return 0;
}
