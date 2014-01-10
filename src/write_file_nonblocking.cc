// This writes the given file to a drive as a series of 1MB chunks and a metadata
// key using the nonblocking API

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <inttypes.h>

#include "protobufutil/message_stream.h"

#include "kinetic/connection_options.h"
#include "kinetic/kinetic_connection_factory.h"
#include "value_factory.h"

using com::seagate::kinetic::proto::Message_Algorithm_SHA1;
using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;
using kinetic::PutCallbackInterface;
using kinetic::NonblockingError ;

class PutCallback : public PutCallbackInterface {
public:
    PutCallback(int* remaining) : remaining_(remaining) {};
    void Success() {
        printf(".");
        fflush(stdout);
        (*remaining_)--;
    }
    void Failure(NonblockingError error) {
        printf("Error!\n");
        exit(1);
    }
private:
    int* remaining_;
};


int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    if (argc != 4) {
        printf("Usage: %s <host> <kinetic key> <input file name>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    const char* kinetic_key = argv[2];
    const char* input_file_name = argv[3];

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = 8123;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    kinetic::ConnectionHandle* connection;
    if (!kinetic_connection_factory.NewConnection(options, &connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    int file = open(input_file_name, O_RDONLY);
    struct stat inputfile_stat;
    fstat(file, &inputfile_stat);
    char* inputfile_data = (char*)mmap(0, inputfile_stat.st_size, PROT_READ, MAP_SHARED, file, 0);
    char key_buffer[100];
    int remaining = 0;
    fd_set read_fds, write_fds;
    int num_fds = 0;

    PutCallback callback(&remaining);

    for (int64_t i = 0; i < inputfile_stat.st_size; i += 1024*1024) {
        int value_size = 1024*1024;
        if (i + value_size > inputfile_stat.st_size) {
            value_size = inputfile_stat.st_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10" PRId64, kinetic_key, i);

        std::string key(key_buffer);
        std::string value(inputfile_data + i, value_size);

        KineticRecord record(value, "", "", Message_Algorithm_SHA1);
        remaining++;
        connection->nonblocking().Put(key, "", true, record, &callback);
        connection->nonblocking().Run(&read_fds, &write_fds, &num_fds);

    }

    KineticRecord record(std::to_string(inputfile_stat.st_size), "", "", Message_Algorithm_SHA1);
    remaining++;

    connection->nonblocking().Put(kinetic_key, "", true, record, &callback);

    connection->nonblocking().Run(&read_fds, &write_fds, &num_fds);
    while (remaining > 0) {
        while (select(num_fds + 1, &read_fds, &write_fds, NULL, NULL) <= 0);
        connection->nonblocking().Run(&read_fds, &write_fds, &num_fds);
    }

    if (close(file)) {
        printf("Unable to close file\n");
        return 1;
    }

    delete connection;
    google::protobuf::ShutdownProtobufLibrary();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();

    printf("Done!\n");

    return 0;
}
