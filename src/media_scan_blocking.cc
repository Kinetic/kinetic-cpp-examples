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

// This allows changing a drive's cluster version
#include <stdio.h>

#include "kinetic/kinetic.h"
#include "gflags/gflags.h"

using com::seagate::kinetic::client::proto::Command_Algorithm_SHA1;
using kinetic::Status;
using kinetic::KineticRecord;
using kinetic::MediaScanRequest;
using kinetic::MediaOptimizeRequest;
using kinetic::RequestPriority;

using std::unique_ptr;

DEFINE_string(start_key, "", "MediaScan start key");
DEFINE_string(start_key_inclusive, "true", "MediaScan start key inclusive");
DEFINE_string(end_key, "", "MediaScan end key");
DEFINE_string(end_key_inclusive, "true", "MediaScan end key inclusive");

int example_main(
		std::shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection,
		std::shared_ptr<kinetic::BlockingKineticConnection> blocking_connection,
		int argc, char** argv) {
    //media scan
	MediaScanRequest media_scan_request;
	media_scan_request.start_key = FLAGS_start_key;
	media_scan_request.end_key = FLAGS_end_key;
	media_scan_request.start_key_inclusive = FLAGS_start_key_inclusive.compare("true") == 0;
	media_scan_request.end_key_inclusive = FLAGS_end_key_inclusive.compare("true") == 0;

	RequestPriority priority = RequestPriority::Priority_NORMAL;
	kinetic::KineticStatus status = blocking_connection->MediaScan(
			media_scan_request, priority);
	std::string result = status.ok() ? "success" : "failure";
	printf("Media scan %s ~ %s return %s\n", media_scan_request.start_key.c_str(),
			media_scan_request.end_key.c_str(), result.c_str());

	return 0;
}
