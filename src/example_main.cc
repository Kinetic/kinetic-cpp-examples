#include "kinetic/kinetic.h"
#include "glog/logging.h"

#include "command_line_flags.h"

int example_main(std::unique_ptr<kinetic::ConnectionHandle> connection, int argc, char** argv);

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);

    std::unique_ptr<kinetic::ConnectionHandle> connection;
    if (!parse_flags(&argc, &argv, connection)) {
        return 1;
    }

    int ret = example_main(move(connection), argc, argv);

    google::protobuf::ShutdownProtobufLibrary();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();

    return ret;
}
