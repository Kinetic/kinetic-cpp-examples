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

// This simple utility can periodically print various stats about a drive

#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

#include "kinetic/kinetic.h"
#include "gflags/gflags.h"

using kinetic::DriveLog;
using kinetic::Capacity;
using kinetic::OperationStatistic;
using kinetic::Utilization;
using kinetic::Temperature;
using kinetic::Limits;

using std::unique_ptr;

void dump_all_information(const DriveLog& drive_log);
void print_temp_report(const DriveLog& drive_log, bool print_headers);
void print_utilization_report(const DriveLog& drive_log, bool print_headers);
void print_operation_stats_report(const DriveLog& drive_log, bool print_headers);

DEFINE_string(type, "all", "Stat type (all|log|temp|utilization|stat)");
DEFINE_uint64(interval, 5, "Refresh interval");

int example_main(
        std::shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection,
        std::shared_ptr<kinetic::BlockingKineticConnection> blocking_connection,
        int argc,
        char** argv) {
    if (FLAGS_type == "all") {
        unique_ptr<DriveLog> drive_log;
        if(!blocking_connection->GetLog(drive_log).ok()) {
            printf("Unable to get log\n");
            return 1;
        }

        dump_all_information(*drive_log);
    } else if (FLAGS_type == "log") {
        unique_ptr<DriveLog> drive_log;
        if(!blocking_connection->GetLog(drive_log).ok()) {
            printf("Unable to get log\n");
            return 1;
        }

        printf("%s\n", drive_log->messages.c_str());
    } else {
        int report_number = 0;
        while (true) {
            unique_ptr<DriveLog> drive_log;
            if(!blocking_connection->GetLog(drive_log).ok()) {
                printf("Unable to get log\n");
                return 1;
            }

            bool print_headers = report_number % 5 == 0;

            if (FLAGS_type == "temp") {
                print_temp_report(*drive_log, print_headers);
            } else if (FLAGS_type == "utilization") {
                print_utilization_report(*drive_log, print_headers);
            } else if (FLAGS_type == "stat") {
                print_operation_stats_report(*drive_log, print_headers);
            }

            printf("\n");

            sleep(FLAGS_interval);
            report_number++;
        }
    }

    return 0;
}

void dump_all_information(const DriveLog& drive_log) {
    printf("Configuration:\n");
    printf("  Vendor: %s\n", drive_log.configuration.vendor.c_str());
    printf("  Model: %s\n", drive_log.configuration.model.c_str());
    printf("  SN: %s\n", drive_log.configuration.serial_number.c_str());
    printf("  Version: %s\n", drive_log.configuration.version.c_str());
    printf("  Compilation Date: %s\n", drive_log.configuration.compilation_date.c_str());
    printf("  Source Hash: %s\n", drive_log.configuration.source_hash.c_str());
    printf("  Port: %d\n", drive_log.configuration.port);
    printf("  TLS Port: %d\n", drive_log.configuration.tls_port);


    printf("\n");


    printf("Capacity: %" PRIu64 " nominal capcity,  %f %% full.\n\n",
        drive_log.capacity.nominal_capacity_in_bytes, drive_log.capacity.portion_full);

    printf("\n");

    printf("Limits: \n");
    printf("  Max Key Size: %d\n", drive_log.limits.max_key_size);
    printf("  Max Value Size: %d\n", drive_log.limits.max_value_size);
    printf("  Max Version Size: %d\n", drive_log.limits.max_version_size);
    printf("  Max Tag Size: %d\n", drive_log.limits.max_tag_size);
    printf("  Max Connections: %d\n", drive_log.limits.max_connections);
    printf("  Max Outstanding Read Operations: %d\n", drive_log.limits.max_outstanding_read_requests);
    printf("  Max Outstanding Write Requests: %d\n", drive_log.limits.max_outstanding_write_requests);
    printf("  Max Message Size: %d\n", drive_log.limits.max_message_size);

    printf("\n");

    printf("Statistics:\n");
    for (auto it = drive_log.operation_statistics.begin();
        it != drive_log.operation_statistics.end();
        ++it) {
        printf("  %s: %" PRId64 " times %" PRId64 " bytes\n", it->name.c_str(), it->count,
            it->bytes);
    }

    printf("\n");

    printf("Utilizations:\n");
    for (auto it = drive_log.utilizations.begin();
        it != drive_log.utilizations.end();
        ++it) {
        printf("  %s: %.02f%%\n", it->name.c_str(), it->percent);
    }

    printf("\n");

    printf("Temperatures:\n");
    for (auto it = drive_log.temperatures.begin();
        it != drive_log.temperatures.end();
        ++it) {
        printf("  %s: %.0f\u00B0C\n", it->name.c_str(), it->current_degc);
    }

    printf("\nMessages:\n");
    printf("%s\n", drive_log.messages.c_str());
}

void print_temp_report(const DriveLog& drive_log, bool print_headers) {
    if (print_headers) {
        for (auto it = drive_log.temperatures.begin();
            it != drive_log.temperatures.end();
            ++it) {
            printf("%7s \u00B0C ", it->name.c_str());
        }
        printf("\n");
    }

    for (auto it = drive_log.temperatures.begin();
        it != drive_log.temperatures.end();
        ++it) {
        printf("%10.0f ", it->current_degc);
    }
}

void print_utilization_report(const DriveLog& drive_log, bool print_headers) {
    if (print_headers) {
        for (auto it = drive_log.utilizations.begin();
            it != drive_log.utilizations.end();
            ++it) {
            printf("%8s %% ", it->name.c_str());
        }
        printf("\n");
    }

    for (auto it = drive_log.utilizations.begin();
        it != drive_log.utilizations.end();
        ++it) {
        printf("%10.0f ", 100 * it->percent);
    }
}

void print_operation_stats_report(const DriveLog& drive_log, bool print_headers) {
    for (auto it = drive_log.operation_statistics.begin();
        it != drive_log.operation_statistics.end();
        ++it) {
        printf("%25s: %" PRId64 "\n", it->name.c_str(), it->count);
    }
}
