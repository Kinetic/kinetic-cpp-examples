// This shows how to erase a drive

#include <stdio.h>

#include "kinetic/kinetic.h"
#include "gflags/gflags.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticStatus;
using kinetic::KineticRecord;

using std::shared_ptr;
using std::string;
using std::unique_ptr;

DEFINE_string(pin, "", "PIN");

int example_main(unique_ptr<kinetic::ConnectionHandle> connection, int argc, char* argv[]) {
    shared_ptr<string> pin(new string(FLAGS_pin));

    KineticStatus status = connection->blocking().InstantSecureErase(pin);
    bool success = status.ok();

    if (!success) {
        printf("Unable to execute ISE: %d %s\n", static_cast<int>(status.statusCode()), status.message().c_str());
        return 1;
    }

    printf("Finished ISE\n");

    return 0;
}
