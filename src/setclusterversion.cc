// This allows changing a drive's cluster version

#include <stdio.h>

#include "kinetic/kinetic.h"
#include "gflags/gflags.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

using std::unique_ptr;

DEFINE_int64(new_cluster_version, 1, "New cluster version");

int example_main(unique_ptr<kinetic::ConnectionHandle> connection, int argc, char* argv[]) {
    printf("Setting cluster version to %" PRId64 "\n", FLAGS_new_cluster_version);

    if (!(connection->blocking().SetClusterVersion(FLAGS_new_cluster_version).ok())) {
        printf("Unable to set cluster version\n");
        return 1;
    }

    printf("Finished setting cluster version\n");

    // this is not the right cluster version, so the get should fail
    connection->blocking().SetClientClusterVersion(FLAGS_new_cluster_version + 1);

    unique_ptr<KineticRecord> result = unique_ptr<KineticRecord>();
    kinetic::StatusCode code = connection->blocking().Get("foo", result).statusCode();
    if (code != kinetic::StatusCode::REMOTE_CLUSTER_VERSION_MISMATCH) {
        printf("Unexpectedly got %d\n", static_cast<int>(code));
        return 1;
    } else {
        printf("Correctly rejected a GET with incorrect cluster version\n");
    }

    return 0;
}
