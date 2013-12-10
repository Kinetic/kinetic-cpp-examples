// This is shows a minimal example of issuing an async request and processing the result

#include <fcntl.h>
#include <sys/select.h>

#include "protobufutil/message_stream.h"

#include "hmac_provider.h"
#include "kinetic_connection_factory.h"
#include "socket_wrapper.h"
#include "value_factory.h"

using com::seagate::kinetic::HmacProvider;
using com::seagate::kinetic::ValueFactory;
using kinetic::ConnectionOptions;
using kinetic::GetCallbackInterface;
using kinetic::Message;
using kinetic::NonblockingKineticConnection;
using kinetic::NonblockingError;
using palominolabs::protobufutil::MessageStreamFactory;

class TestCallback : public GetCallbackInterface {
    public:
    void Success(const std::string &key, const std::string &value,
            const std::string &version, const std::string &tag) {
        printf("The callback got called!\n");
    }
    virtual void Failure(NonblockingError error) {
        printf("Error!\n");
        exit(1);
    }
};

int main(int argc, char* argv[]) {
    ConnectionOptions options = {
        .host = "localhost",
        .port = 8123,
        .use_ssl = false,
        .user_id = 1,
        .hmac_key = "asdfasdf"
    };

    HmacProvider hmac_provider;
    ValueFactory value_factory;
    MessageStreamFactory message_stream_factory(NULL, value_factory);
    kinetic::KineticConnectionFactory kinetic_connection_factory(hmac_provider,
            message_stream_factory);

    kinetic::NonblockingKineticConnection *connection;
    if(!kinetic_connection_factory.NewNonblockingConnection(options, &connection).ok()) {
        printf("Unable to connect");
        return 1;
    }

    TestCallback *callback = new TestCallback;
    connection->Get("key", callback);

    fd_set read_fds, write_fds;
    int num_fds = 0;
    connection->Run(&read_fds, &write_fds, &num_fds);
    connection->Run(&read_fds, &write_fds, &num_fds);

    delete callback;
    delete connection;
    return 0;
}
