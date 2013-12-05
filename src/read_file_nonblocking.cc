// This reads a file stored using write_file_blocking.cc using the nonblocking API

#include <stdio.h>
#include <glog/logging.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <inttypes.h>

#include "protobufutil/message_stream.h"

#include "connection_options.h"
#include "hmac_provider.h"
#include "kinetic_connection_factory.h"
#include "value_factory.h"
#include "socket_wrapper.h"

using com::seagate::kinetic::HmacProvider;
using com::seagate::kinetic::proto::Message;
using com::seagate::kinetic::proto::Message_MessageType_GET;
using com::seagate::kinetic::proto::Message_Algorithm_SHA1;
using com::seagate::kinetic::ValueFactory;
using kinetic::KineticConnection;
using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;
using palominolabs::protobufutil::MessageStreamFactory;
using com::seagate::kinetic::HmacProvider;
using kinetic::NonblockingError ;
using kinetic::ConnectionOptions;
using kinetic::GetCallbackInterface;
using kinetic::Message;
using kinetic::NonblockingKineticConnection;
using kinetic::SocketWrapper;

class TestCallback : public GetCallbackInterface {
public:
    TestCallback(char* buffer, unsigned int expected_length, int* remaining) : buffer_(buffer), expected_length_(expected_length), remaining_(remaining) {};
    void Success(const std::string &value, const std::string &version,
            const std::string &tag) {
        if(expected_length_ != value.size()) {
            printf("Received value chunk of wrong size\n");
            exit(1);
        }
        value.copy(buffer_, expected_length_);
        printf(".");
        fflush(stdout);
        (*remaining_)--;
    }
    void Failure(NonblockingError error) {
        printf("Error!\n");
        exit(1);
    }
private:
    char* buffer_;
    unsigned int expected_length_;
    int* remaining_;
};

int main(int argc, char* argv[]) {

    if (argc != 4) {
        printf("%s: <host> <kinetic key> <output file name>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    const char* kinetic_key = argv[2];
    const char* output_file_name = argv[3];

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

    kinetic::KineticConnection* kinetic_connection;
    if(!kinetic_connection_factory.NewConnection(options, &kinetic_connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }


    std::string value;
    if(!kinetic_connection->Get(kinetic_key, &value, NULL, NULL).ok()) {
        printf("Unable to get metadata\n");
        return 1;
    }

    long long file_size = std::stoll(value);
    printf("Reading file of size %llu\n", file_size);


    delete kinetic_connection;

    kinetic::NonblockingKineticConnection* connection;
    kinetic_connection_factory.NewNonblockingConnection(options, &connection);

    int file = open(output_file_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(!file) {
        printf("Unable to open output file\n");
        return 1;
    }
    if((off_t)(file_size - 1) != lseek(file, file_size - 1, SEEK_SET)) {
        printf("Unable to seek in file\n");
        return 1;
    }
    if(write(file, " ", 1) != 1) {
        printf("Unable to resize file\n");
        return 1;
    }
    char* output_buffer = (char*)mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    char key_buffer[100];
    int remaining = 0;
    fd_set read_fds, write_fds;
    int num_fds = 0;
    for (off_t i = 0; i < file_size; i += 1024*1024) {
        unsigned int block_length = 1024*1024;
        if (i + block_length > file_size) {
            block_length = file_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10" PRId64, kinetic_key, (int64_t)i);
        remaining++;
        TestCallback* callback = new TestCallback(output_buffer + i, block_length, &remaining);
        std::string key(key_buffer);
        connection->Get(key, callback);
        connection->Run(&read_fds, &write_fds, &num_fds);
    }

    connection->Run(&read_fds, &write_fds, &num_fds);
    while (remaining > 0) {
        while(select(num_fds + 1, &read_fds, &write_fds, NULL, NULL) <= 0);
        connection->Run(&read_fds, &write_fds, &num_fds);
    }

    CHECK(!close(file));

    printf("\nDone!\n");

    delete connection;

    return 0;
}
