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

// This reads a file stored using write_file_blocking.cc using the blocking API

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <inttypes.h>

#include "kinetic/kinetic.h"
#include "glog/logging.h"

using kinetic::Status;
using kinetic::KineticRecord;
using kinetic::KineticStatus;

using std::unique_ptr;

DEFINE_string(output_file_name, "", "Output file name");
DEFINE_string(kinetic_key, "", "Kinetic key");

int example_main(
        std::shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection,
        std::shared_ptr<kinetic::BlockingKineticConnection> blocking_connection,
        int argc,
        char** argv) {

    if (FLAGS_output_file_name.size() == 0) {
        printf("Must specify an output file\n");
        return 1;
    }

    std::unique_ptr<KineticRecord> record;
    KineticStatus get_status = blocking_connection->Get(FLAGS_kinetic_key, record);
    if(!get_status.ok()) {
        printf("Unable to get metadata: %s\n", get_status.message().c_str());
        return 1;
    }

    ssize_t file_size = std::stoll(*(record->value()));

    printf("Reading file of size %zd\n", file_size);

    int file = open(FLAGS_output_file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(!file) {
        printf("Unable to open output file\n");
        return 1;
    }
    if(file_size - 1 != lseek(file, file_size - 1, SEEK_SET)) {
        printf("Unable to seek\n");
        return 1;
    }
    if(write(file, " ", 1) != 1) {
        printf("Unable to resize file\n");
        return 1;
    }
    char* output_buffer = (char*)mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    char key_buffer[100];
    for (int64_t i = 0; i < file_size; i += 1024*1024) {
        int block_length = 1024*1024;
        if (i + block_length > file_size) {
            block_length = file_size - i;
        }

        sprintf(key_buffer, "%s-%10" PRId64, FLAGS_kinetic_key.c_str(), i);
        std::string key(key_buffer);

        if(!blocking_connection->Get(key, record).ok()) {
            printf("Unable to get chunk\n");
            return 1;
        }
        
        record->value()->copy(output_buffer + i, block_length);

        printf(".");
        fflush(stdout);
    }
    printf("\n");

    if(close(file)) {
        printf("Unable to close file\n");
        return 1;
    }

    printf("Done!\n");

    return 0;
}
