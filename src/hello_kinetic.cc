#include <stdio.h>

#include "glog/logging.h"
#include "protobufutil/message_stream.h"

#include "connection_options.h"
#include "hmac_provider.h"
#include "kinetic_connection_factory.h"
#include "kinetic_connection.h"
#include "kinetic_record.h"
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
using palominolabs::protobufutil::MessageStreamFactory;

int main(int argc, char* argv[]) {
    (void) argc;

    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;

    LOG(INFO)<< "Hello Kinetic Client!";

    kinetic::ConnectionOptions options;
    options.host = std::string("localhost");
    options.port = 8123;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    HmacProvider hmac_provider;
    ValueFactory value_factory;
    MessageStreamFactory message_stream_factory(NULL, value_factory);
    kinetic::KineticConnectionFactory kinetic_connection_factory(hmac_provider,
            message_stream_factory);

    kinetic::KineticConnection* kinetic_connection;
    CHECK(
            kinetic_connection_factory.NewConnection(options, &kinetic_connection).ok());

    std::string key = "key";
    std::string value, version, tag;
    CHECK(
            kinetic_connection->Put(key, "",
                    KineticRecord("the value", "", "", Message_Algorithm_SHA1)).ok());
    CHECK(kinetic_connection->Get(key, &value, &version, &tag).ok());
    LOG(INFO)<< "Value for " << key << " is <" << value << ">";

    return 0;
}
