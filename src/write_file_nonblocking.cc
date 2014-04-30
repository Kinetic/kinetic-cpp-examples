// This writes the given file to a drive as a series of 1MB chunks and a metadata
// key using the nonblocking API

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "kinetic/kinetic.h"
#include "glog/logging.h"

using com::seagate::kinetic::client::proto::Message_Algorithm_SHA1;
using kinetic::KineticConnectionFactory;
using kinetic::Status;
using kinetic::KineticRecord;
using kinetic::PutCallbackInterface;
using kinetic::KineticStatus;

using std::make_shared;
using std::unique_ptr;

class PutCallback : public PutCallbackInterface {
public:
    PutCallback(int* remaining) : remaining_(remaining) {};
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

DEFINE_string(kinetic_key, "my_file", "Key prefix for storing file chunks");
DEFINE_string(local_file, "local_file", "Path of file to store in kinetic");


int example_main(std::unique_ptr<kinetic::ConnectionHandle> connection, int argc, char** argv) {
    int file = open(FLAGS_local_file.c_str(), O_RDONLY);

    if (file < 0) {
        printf("Unable to open %s\n", FLAGS_local_file.c_str());
        return 1;
    }

    struct stat inputfile_stat;
    fstat(file, &inputfile_stat);
    char* inputfile_data = (char*)mmap(0, inputfile_stat.st_size, PROT_READ, MAP_SHARED, file, 0);
    char key_buffer[100];
    int remaining = 0;
    fd_set read_fds, write_fds;
    int num_fds = 0;

    auto callback = make_shared<PutCallback>(&remaining);

    for (int64_t i = 0; i < inputfile_stat.st_size; i += 1024*1024) {
        int value_size = 1024*1024;
        if (i + value_size > inputfile_stat.st_size) {
            value_size = inputfile_stat.st_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10" PRId64, FLAGS_kinetic_key.c_str(), i);

        std::string key(key_buffer);
        std::string value(inputfile_data + i, value_size);

        auto record = make_shared<KineticRecord>(value, "", "", Message_Algorithm_SHA1);
        remaining++;
        connection->nonblocking().Put(key, "", kinetic::IGNORE_VERSION, record, callback);
        connection->nonblocking().Run(&read_fds, &write_fds, &num_fds);
    }

    auto record = make_shared<KineticRecord>(std::to_string(inputfile_stat.st_size), "", "", Message_Algorithm_SHA1);
    remaining++;

    connection->nonblocking().Put(FLAGS_kinetic_key.c_str(), "", kinetic::IGNORE_VERSION, record, callback);

    connection->nonblocking().Run(&read_fds, &write_fds, &num_fds);
    while (remaining > 0) {
        while (select(num_fds + 1, &read_fds, &write_fds, NULL, NULL) <= 0);
        connection->nonblocking().Run(&read_fds, &write_fds, &num_fds);
    }

    if (close(file)) {
        printf("Unable to close file\n");
        return 1;
    }

    printf("Done!\n");

    return 0;
}
