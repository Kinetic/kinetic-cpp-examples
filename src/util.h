/**
 * Copyright 2013-2015 Seagate Technology LLC.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at
 * https://mozilla.org/MP:/2.0/.
 * 
 * This program is distributed in the hope that it will be useful,
 * but is provided AS-IS, WITHOUT ANY WARRANTY; including without 
 * the implied warranty of MERCHANTABILITY, NON-INFRINGEMENT or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the Mozilla Public 
 * License for more details.
 *
 * See www.openkinetic.org for more project information
 */
#ifndef KINETIC_CPP_EXAMPLES_H_
#define KINETIC_CPP_EXAMPLES_H_

void PerformKineticOpOrExit(kinetic::KineticStatus status) {
  if (!status.ok()) {
    fprintf(stderr, "KineticError: %d %s\n", status.statusCode(), status.message().c_str());
    exit(1);
  }
}

#endif  // KINETIC_CPP_EXAMPLES_H_
