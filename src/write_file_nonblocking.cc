// This writes the given file to a drive as a series of 1MB chunks and a metadata
// key using the nonblocking API

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <inttypes.h>

#include "protobufutil/message_stream.h"

#include "connection_options.h"
#include "hmac_provider.h"
#include "kinetic_connection_factory.h"
#include "value_factory.h"

using com::seagate::kinetic::HmacProvider;
using com::seagate::kinetic::proto::Message;
using com::seagate::kinetic::proto::Message_MessageType_GET;
using com::seagate::kinetic::proto::Message_Algorithm_SHA1;
using com::seagate::kinetic::ValueFactory;
using kinetic::KineticConnection;
using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;
using kinetic::PutCallbackInterface;
using kinetic::NonblockingError ;
using palominolabs::protobufutil::MessageStreamFactory;

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

    HmacProvider hmac_provider;
    ValueFactory value_factory;
    MessageStreamFactory message_stream_factory(NULL, value_factory);
    kinetic::KineticConnectionFactory kinetic_connection_factory(hmac_provider,
            message_stream_factory);

    kinetic::NonblockingKineticConnection* connection;
    if (!kinetic_connection_factory.NewNonblockingConnection(options, &connection).ok()) {
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
    for (off_t i = 0; i < inputfile_stat.st_size; i += 1024*1024) {
        int value_size = 1024*1024;
        if (i + value_size > inputfile_stat.st_size) {
            value_size = inputfile_stat.st_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10" PRId64, kinetic_key, (int64_t)i);

        std::string key(key_buffer);
        std::string value(inputfile_data + i, value_size);

        KineticRecord record(value, "", "", Message_Algorithm_SHA1);
        remaining++;
        connection->Put(key, "", true, record, new PutCallback(&remaining));
        connection->Run(&read_fds, &write_fds, &num_fds);

    }

    KineticRecord record(std::to_string(inputfile_stat.st_size), "", "", Message_Algorithm_SHA1);
    remaining++;
    connection->Put(kinetic_key, "", true, record, new PutCallback(&remaining));

    connection->Run(&read_fds, &write_fds, &num_fds);
    while (remaining > 0) {
        while (select(num_fds + 1, &read_fds, &write_fds, NULL, NULL) <= 0);
        connection->Run(&read_fds, &write_fds, &num_fds);
    }

    if (close(file)) {
        printf("Unable to close file\n");
        return 1;
    }

    printf("Done!\n");

    return 0;
}
