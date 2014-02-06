// This sends a firmware file to the drive

#include <stdio.h>
#include <fstream>

#include "kinetic/kinetic.h"

using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;

using std::make_shared;
using std::string;
using std::unique_ptr;

int main(int argc, char* argv[]) {

    if (argc != 3) {
        printf("Usage: %s <host> <input file name>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    const char* input_file_name = argv[2];

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = 8123;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    unique_ptr<kinetic::ConnectionHandle> connection;
    if (!kinetic_connection_factory.NewConnection(options, 5, connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    std::ifstream in(input_file_name, std::ios::in | std::ios::binary);
    std::ostringstream contents;
    if (in.fail()) {
        printf("Unable to read file\n");
        return 1;
    } else {
        contents << in.rdbuf();
    }

    auto value = make_shared<string>(contents.str());
    if (!connection->blocking().UpdateFirmware(value).ok()) {
        printf("Unable to send firmware\n");
        return 1;
    }

    printf("Done!\n");

    return 0;
}
