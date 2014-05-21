/*
 * kinetic-cpp-examples
 * Copyright (C) 2014 Seagate Technology.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

// This writes the given file to a drive as a series of 1MB chunks and a metadata key

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "glog/logging.h"

#include "kinetic/kinetic.h"

using com::seagate::kinetic::client::proto::Message_Algorithm_SHA1;
using kinetic::Status;
using kinetic::KineticRecord;

using std::make_shared;
using std::unique_ptr;
using std::to_string;

DEFINE_string(kinetic_key, "my_file", "Key prefix for storing file chunks");
DEFINE_string(local_file, "local_file", "Path of file to store in kinetic");

int example_main(
        std::shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection,
        std::shared_ptr<kinetic::BlockingKineticConnection> blocking_connection,
        int argc,
        char** argv) {
    int file = open(FLAGS_local_file.c_str(), O_RDONLY);
    if (file < 0) {
        printf("Unable to open file %s\n", FLAGS_local_file.c_str());
        return 1;
    }
    struct stat inputfile_stat;
    fstat(file, &inputfile_stat);
    char* inputfile_data = (char*)mmap(0, inputfile_stat.st_size, PROT_READ, MAP_SHARED, file, 0);
    char key_buffer[100];
    for (int64_t i = 0; i < inputfile_stat.st_size; i += 1024*1024) {
        int value_size = 1024*1024;
        if (i + value_size > inputfile_stat.st_size) {
            value_size = inputfile_stat.st_size - i + 1;
        }

        sprintf(key_buffer, "%s-%10" PRId64, FLAGS_kinetic_key.c_str(), i);

        std::string key(key_buffer);
        std::string value(inputfile_data + i, value_size);
        // Use the Write Back persist mode for the data puts, and then use Flush for the metadata
        // below to ensure everything is persisted by the time we exit.
        if(!blocking_connection->Put(
                key,
                "",
                kinetic::WriteMode::IGNORE_VERSION,
                KineticRecord(value, "", "", Message_Algorithm_SHA1),
                kinetic::PersistMode::WRITE_BACK).ok()) {
            printf("Unable to write a chunk\n");
            return 1;
        }
        printf(".");
        fflush(stdout);
    }
    printf("\n");


    // Use the Flush persist mode to make sure all previous writes are persisted
    if (!blocking_connection->Put(
            FLAGS_kinetic_key,
            "",
            kinetic::WriteMode::IGNORE_VERSION,
            KineticRecord(to_string(inputfile_stat.st_size), "", "", Message_Algorithm_SHA1),
            kinetic::PersistMode::FLUSH).ok()) {
        printf("Unable to write metadata\n");
        return 1;
    }

    if (close(file)) {
        printf("Unable to close file\n");
        return 1;
    }

    printf("Done!\n");

    return 0;
}
