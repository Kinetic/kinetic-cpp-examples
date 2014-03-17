#ifndef COMMAND_LINE_FLAGS_H_
#define COMMAND_LINE_FLAGS_H_

#include "kinetic/kinetic.h"

#include "gflags/gflags.h"

DEFINE_string(host, "localhost", "Kinetic Host");
DEFINE_uint64(port, 8123, "Kinetic Port");
DEFINE_uint64(timeout, 30, "Timeout");
DEFINE_uint64(user_id, 1, "Kinetic User ID");
DEFINE_string(hmac_key, "asdfasdf", "Kinetic User HMAC key");

bool parse_flags(int *argc, char*** argv, std::unique_ptr<kinetic::ConnectionHandle>& connection) {
    google::ParseCommandLineFlags(argc, argv, true);

    kinetic::ConnectionOptions options;
    options.host = FLAGS_host;
    options.port = FLAGS_port;
    options.user_id = FLAGS_user_id;
    options.hmac_key = FLAGS_hmac_key;

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    if (!kinetic_connection_factory.NewConnection(options, FLAGS_timeout, connection).ok()) {
        printf("Unable to connect\n");
        return false;
    }

    return true;
}

#endif  // COMMAND_LINE_FLAGS_H_
