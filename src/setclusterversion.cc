// This allows changing a drive's cluster version

#include <stdio.h>

#include "kinetic/kinetic.h"
#include "gflags/gflags.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

using std::unique_ptr;

DEFINE_uint64(new_cluster_version, 0, "New cluster version");

int example_main(unique_ptr<kinetic::ConnectionHandle> connection, int argc, char* argv[]) {
    printf("Setting cluster version of to %" PRIu64 "\n", FLAGS_new_cluster_version);

    if (!(connection->blocking().SetClusterVersion(FLAGS_new_cluster_version).ok())) {
        printf("Unable to set cluster version\n");
        return 1;
    }

    printf("Finished setting cluster version\n");

    return 0;
}
