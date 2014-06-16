#ifndef KINETIC_CPP_EXAMPLES_H_
#define KINETIC_CPP_EXAMPLES_H_

void PerformKineticOpOrExit(kinetic::KineticStatus status) {
  if (!status.ok()) {
    fprintf(stderr, "KineticError: %d %s\n", status.statusCode(), status.message().c_str());
    exit(1);
  }
}

#endif  // KINETIC_CPP_EXAMPLES_H_
