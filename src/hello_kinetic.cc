// This is a minimal blocking get/blocking put example

#include <stdio.h>
#include <glog/logging.h>

#include "protobufutil/message_stream.h"

#include "kinetic/connection_options.h"
#include "kinetic/kinetic_connection_factory.h"
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

    printf("Hello Kinetic Client!\n");

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
    if(!kinetic_connection_factory.NewConnection(options, &kinetic_connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    std::string key = "key";
    std::string value, version, tag;
    if(!kinetic_connection->Put(key, "",
                    KineticRecord("the value", "", "", Message_Algorithm_SHA1)).ok()) {
        printf("Unable to PUT\n");
        return 1;
    }
    if(!kinetic_connection->Get(key, &value, &version, &tag).ok()) {
        printf("Unable to GET\n");
        return 1;
    }
    printf("Value for %s is <%s>\n", key.c_str(), value.c_str());

    return 0;
}
