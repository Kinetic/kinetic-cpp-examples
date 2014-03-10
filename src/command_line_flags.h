#ifndef COMMAND_LINE_FLAGS_H_
#define COMMAND_LINE_FLAGS_H_

#include "kinetic/kinetic.h"

#include "gflags/gflags.h"

DEFINE_string(host, "localhost", "Kinetic Host");
DEFINE_uint64(port, 8123, "Kinetic Port");
DEFINE_uint64(timeout, 30, "Timeout");

void parse_flags(int *argc, char*** argv, std::unique_ptr<kinetic::ConnectionHandle>& connection) {
    google::ParseCommandLineFlags(argc, argv, true);

    kinetic::ConnectionOptions options;
    options.host = FLAGS_host;
    options.port = FLAGS_port;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    if (!kinetic_connection_factory.NewConnection(options, FLAGS_timeout, connection).ok()) {
        printf("Unable to connect\n");
        exit(1);
    }
}


#endif  // COMMAND_LINE_FLAGS_H_
