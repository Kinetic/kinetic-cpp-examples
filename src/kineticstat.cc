// This simple utility can periodically print various stats about a drive

#include <inttypes.h>
#include <stdio.h>

#include "kinetic/kinetic.h"

using kinetic::DriveLog;
using kinetic::Capacity;
using kinetic::OperationStatistic;
using kinetic::Utilization;
using kinetic::Temperature;

using std::unique_ptr;

void dump_all_information(const DriveLog& drive_log);
void print_temp_report(const DriveLog& drive_log, bool print_headers);
void print_utilization_report(const DriveLog& drive_log, bool print_headers);
void print_operation_stats_report(const DriveLog& drive_log, bool print_headers);

int main(int argc, char* argv[]) {
    (void) argc;

    if (argc != 2 && argc != 4) {
        printf("Usage: %s <host>\n", argv[0]);
        printf("       %s <host> <temp|utilization|stat> <interval>\n", argv[0]);
        return 1;
    }

    kinetic::ConnectionOptions options;
    options.host = std::string(argv[1]);
    options.port = 8123;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

    unique_ptr<kinetic::ConnectionHandle> connection;
    if(!kinetic_connection_factory.NewConnection(options, connection).ok()) {
        printf("Unable to connect\n");
        return 1;
    }

    if (argc == 2) {
        // User just specified host so dump everything
        unique_ptr<DriveLog> drive_log;
        if(!connection->blocking().GetLog(drive_log).ok()) {
            printf("Unable to get log\n");
            return 1;
        }

        dump_all_information(*drive_log);
    } else {
        // User wants to poll host so figure out the information so start
        // the polling
        std::string report_type(argv[2]);
        int report_interval = atoi(argv[3]);
        int report_number = 0;
        while (true) {
            unique_ptr<DriveLog> drive_log;
            if(!connection->blocking().GetLog(drive_log).ok()) {
                printf("Unable to get log\n");
                return 1;
            }

            bool print_headers = report_number % 5 == 0;

            if (report_type == "temp") {
                print_temp_report(*drive_log, print_headers);
            } else if (report_type == "utilization") {
                print_utilization_report(*drive_log, print_headers);
            } else if (report_type == "stat") {
                print_operation_stats_report(*drive_log, print_headers);
            }

            printf("\n");

            sleep(report_interval);
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

    printf("Capacity: %.0f bytes remaining / %.0f bytes\n\n",
        drive_log.capacity.remaining_bytes, drive_log.capacity.total_bytes);

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
