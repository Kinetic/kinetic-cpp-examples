// This allows setting and changing a drive's PIN

#include <stdio.h>

#include "kinetic/kinetic.h"
#include "gflags/gflags.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

using std::string;
using std::make_shared;
using std::unique_ptr;

DEFINE_string(new_pin, "", "New PIN");
DEFINE_string(old_pin, "", "Old PIN");

int example_main(unique_ptr<kinetic::ConnectionHandle> connection, int argc, char* argv[]) {
    bool success;

    if (FLAGS_old_pin.empty()) {
        success = connection->blocking().SetPin(make_shared<string>(FLAGS_new_pin)).ok();
    } else {
        auto pin = make_shared<string>(FLAGS_old_pin);
        success = connection->blocking().SetPin(make_shared<string>(FLAGS_new_pin), pin).ok();
    }

    if (success) {
        printf("Finished setting pin\n");
        return 0;
    } else {
        printf("Unable to change pin\n");
        return 1;
    }
}
