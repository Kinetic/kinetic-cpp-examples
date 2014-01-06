// This deletes a file written using write_file_(non)blocking

#include <stdio.h>
#include <glog/logging.h>

#include "protobufutil/message_stream.h"

#include "kinetic/connection_options.h"
#include "kinetic/kinetic_connection_factory.h"
#include "value_factory.h"
#include "socket_wrapper.h"

using com::seagate::kinetic::HmacProvider;
using com::seagate::kinetic::proto::Message;
using com::seagate::kinetic::proto::Message_MessageType_GET;
using com::seagate::kinetic::proto::Message_Algorithm_SHA1;
using com::seagate::kinetic::ValueFactory;
using kinetic::KineticConnectionFactory;
using kinetic::KineticRecord;
using kinetic::NonblockingError;
using kinetic::SimpleCallbackInterface;
using kinetic::Status;
using palominolabs::protobufutil::MessageStreamFactory;

class DeleteCallback : public SimpleCallbackInterface {
public:
    DeleteCallback(int* remaining) : remaining_(remaining) {};
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

    kinetic::BlockingKineticConnection* kinetic_connection;
    if(!kinetic_connection_factory.NewBlockingConnection(options, &kinetic_connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }


    KineticRecord* record;
    if(!kinetic_connection->Get(kinetic_key, &record).ok()) {
        printf("Unable to get metadata\n");
        return 1;
    }

    unsigned int file_size = std::stoi(record->value());
    delete record;
    printf("Deleting file of size %d\n", file_size);


    delete kinetic_connection;

    kinetic::NonblockingKineticConnection* connection;
    kinetic_connection_factory.NewNonblockingConnection(options, &connection);

    char key_buffer[100];
    int remaining = 0;
    for (unsigned int i = 0; i < file_size; i += 1024*1024) {
        unsigned int block_length = 1024*1024;
        if (i + block_length > file_size) {
            block_length = file_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10d", kinetic_key, i);
        remaining++;
        DeleteCallback* callback = new DeleteCallback(&remaining);
        std::string key(key_buffer);
        connection->Delete(key, "", true, callback);
    }

    remaining++;
    connection->Delete(kinetic_key, "", true, new DeleteCallback(&remaining));


    fd_set read_fds, write_fds;
    int num_fds = 0;
    connection->Run(&read_fds, &write_fds, &num_fds);
    while (remaining > 0) {
        connection->Run(&read_fds, &write_fds, &num_fds);
    }

    printf("\nDone!\n");

    delete connection;

    return 0;
}
