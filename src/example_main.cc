#include "kinetic/kinetic.h"

#include "command_line_flags.h"

int example_main(std::unique_ptr<kinetic::ConnectionHandle> connection, int argc, char** argv);

int main(int argc, char** argv) {
    std::unique_ptr<kinetic::ConnectionHandle> connection;
    if (!parse_flags(&argc, &argv, connection)) {
        return 1;
    }

    return example_main(move(connection), argc, argv);
}
