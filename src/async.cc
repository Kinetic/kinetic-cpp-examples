// This is shows a minimal example of issuing an async request and processing the result

#include <fcntl.h>
#include <sys/select.h>

#include "glog/logging.h"
#include "protobufutil/message_stream.h"

#include "hmac_provider.h"
#include "kinetic.pb.h"
#include "kinetic_connection_factory.h"
#include "nonblocking_kinetic_connection.h"
#include "nonblocking_message_service.h"
#include "socket_wrapper.h"
#include "value_factory.h"

using com::seagate::kinetic::HmacProvider;
using com::seagate::kinetic::ValueFactory;
using kinetic::CallbackInterface;
using kinetic::ConnectionOptions;
using kinetic::GetCallbackInterface;
using kinetic::Message;
using kinetic::NonblockingKineticConnection;
using kinetic::NonblockingMessageService;
using palominolabs::protobufutil::MessageStreamFactory;

class TestCallback : public GetCallbackInterface {
    public:
    void Call(const std::string &value, const std::string &version, const std::string &tag) {
        LOG(INFO) << "The callback got called!";
    }
};

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;

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
    CHECK(kinetic_connection_factory.NewNonblockingConnection(options, &connection).ok());

    TestCallback *callback = new TestCallback;
    connection->Get("key", callback);

    fd_set read_fds, write_fds;
    connection->Run(&read_fds, &write_fds);
    connection->Run(&read_fds, &write_fds);

    delete callback;
    delete connection;
    return 0;
}
