// This allows setting and changing a drive's PIN

#include <stdio.h>

#include "kinetic/kinetic.h"

#include "command_line_flags.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

using std::string;
using std::make_shared;
using std::unique_ptr;

DEFINE_string(new_pin, "", "New PIN");
DEFINE_string(old_pin, "", "Old PIN");

int main(int argc, char* argv[]) {
    unique_ptr<kinetic::ConnectionHandle> connection;
    parse_flags(&argc, &argv, connection);

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
