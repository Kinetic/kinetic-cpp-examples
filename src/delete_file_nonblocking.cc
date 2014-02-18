// This deletes a file written using write_file_(non)blocking

#include <stdio.h>
#include <string>

#include "kinetic/kinetic.h"
#include "glog/logging.h"

using kinetic::KineticConnectionFactory;
using kinetic::KineticRecord;
using kinetic::KineticStatus;
using kinetic::SimpleCallbackInterface;
using kinetic::Status;

using std::make_shared;
using std::string;
using std::unique_ptr;

class DeleteCallback : public SimpleCallbackInterface {
public:
    DeleteCallback(int* remaining) : remaining_(remaining) {};
    void Success() {
        printf(".");
        fflush(stdout);
        (*remaining_)--;
    }

    void Failure(KineticStatus error) {
        printf("Error: %d %s\n", static_cast<int>(error.statusCode()), error.message().c_str());
        exit(1);
    }
private:
    int* remaining_;

};

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    if (argc != 3) {
        printf("%s: <host> <kinetic key>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    const char* kinetic_key = argv[2];

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = 8123;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    unique_ptr<kinetic::ConnectionHandle> connection;
    if(!kinetic_connection_factory.NewConnection(options, 5, connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    std::unique_ptr<KineticRecord> record;
    if(!connection->blocking().Get(string(kinetic_key), record).ok()) {
        printf("Unable to get metadata\n");
        return 1;
    }

    long long file_size = std::stoll(*(record->value()));
    printf("Deleting file of size %llu\n", file_size);

    char key_buffer[100];
    int remaining = 0;
    auto callback = make_shared<DeleteCallback>(&remaining);
    for (int64_t i = 0; i < file_size; i += 1024*1024) {
        unsigned int block_length = 1024*1024;
        if (i + block_length > file_size) {
            block_length = file_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10" PRId64, kinetic_key, i);
        remaining++;
        std::string key(key_buffer);
        connection->nonblocking().Delete(key, "", kinetic::IGNORE_VERSION, callback);
    }

    remaining++;
    connection->nonblocking().Delete(kinetic_key, "", kinetic::IGNORE_VERSION, callback);


    fd_set read_fds, write_fds;
    int num_fds = 0;
    connection->nonblocking().Run(&read_fds, &write_fds, &num_fds);
    while (remaining > 0) {
        connection->nonblocking().Run(&read_fds, &write_fds, &num_fds);
    }

    printf("\nDone!\n");

    google::protobuf::ShutdownProtobufLibrary();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();

    return 0;
}
