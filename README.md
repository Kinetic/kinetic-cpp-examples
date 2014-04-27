Overview
========
This repository contains a number of examples showing how to use the Kinetic C++ client in different way. Each example is fairly simple but includes error handling and they cover threaded and non-blocking IO.

Building
========
The examples live in the `src` directory . To build them examples you need `cmake`, `git` and a working build system. Once you've acquired those items you can build all the examples by running:

    cmake .
    make

Binaries will appear in the working directory. The first build takes several minutes because all the dependencies are downloaded and compiled.

Examples
========

`CMakeLists.txt` provides a starting point for including the Kinetic C++ client in CMake-based projects.

All the examples have shared boilerplate code for command line argument parsing, logging setup, etc. This common code is extracted out into `src/command_line_flags.h` and `src/example_main.cc`.

`copydrive` (see `src/copydrive.cc`)
-----------
This example uses the P2P push functionality to copy an entire keyspace from one drive to one or more other drives. For example, to run a pipeline that
looks like A -> B -> C you could call it like so:

    ./copydrive A 8123 B 8123 C 4444

`dump_keyspace` (see `src/dump_keyspace.cc`)
---------------
This example uses the C++ client iterator (which uses GetKeyRange internally) to print the entire keyspace of the given drive. You can expose it like so:

    ./dump_keyspace 127.0.0.1 8123

`firmware_update` (see `src/firmware_update.cc`)
-----------------
The Kinetic API allows supplying new firmware images. This example shows how to do this. To invoke it run:

    ./firmware_update <ip> <host> <firmware image file>

`ise` (see `src/ise.cc`)
-----
This example shows how to use the ISE functionality to immediately wipe a drive. To try it out run:

    ./ise -host 127.1 -port 8123

`kineticstat` (see `src/kineticstat.cc`)
-------------
The Kinetic API exposes rich management detail available via the GetLog command. The `kineticstat` example shows how to present this information in different ways. Some fun things to try:

* Dump all available info: `./kineticstat -host 127.1 -port 8123 -type all`
* Display application logs: `./kineticstat -host 127.1 -port 8123 -type log`
* Print temperatures every second: `./kineticstat -host 127.1 -port 8123 -type temp -interval 1`
* Print utilizations every 3s: `./kineticstat -host 127.1 -port 8123 -type utilization -interval 3`
* Print operation counts every 10s: `./kineticstat -host 127.1 -port 8123 -type stat -interval 10`

`set_acls` (see `src/set_acls.cc`)
----------
Toy example demonstrating how to change the ACLs. It always sets a hard-coded set of ACLs. Invoke it like so:

    ./set_acls -host 127.0.0.1 -port 8123

`setclusterversion` (see `src/setclusterversion.cc`)
-------------------
Changes the cluster version and verifies that requests with the old cluster version get rejected. Example usage:

    ./setclusterversion -host 127.1 -port 8123 -new_cluster_version 99

`setpin` (see `src/setpin.cc`)
--------
Allows changing a drive's PIN. For instance:

    ./setpin -host 127.1 -port 8123 -new_pin 1234 -old_pin 5678

Object store examples
---------------------
The remaining examples form a more complex app that allows storing, retrieveing and deleting large files in a Kinetic implementation. They break up the file into 1MB chunks and store each chunk in its own keys. During
retrieval the individual chunks are re-assembled. For instance, suppose you create a file called `my_precious_data` by doing something like this:

    dd if=/dev/random of=my_precious_data bs=1048576 count=1024

You can store it in Kinetic implementation in a few different ways:

    ./write_file_blocking -host localhost -port 8123 -kinetic_key object_store -local_file my_precious_data
    ./write_file_blocking_threads localhost object_store my_precious_data
    ./write_file_nonblocking -host localhost -port 8123 -kinetic_key object_store -local_file my_precious_data

Those example programs are implemented in `src/write_file_blocking.cc`, `src/write_file_blocking_threads.cc`, and `src/write_file_nonblocking.cc`. They all do the same thing but use non-blocking IO, blocking IO, or multi-threaded blocking IO to demonstrate different styles.

Getting the data back is easy. Just try one of:

    ./read_file_blocking -host localhost -port 8123 -kinetic_key object_store -output_file_name file_from_kinetic
    ./read_file_nonblocking localhost object_store other_file_from_kinetic

Those live in `src/read_file_blocking.cc` and `src/read_file_nonblocking.cc` and show blocking an non-blocking styles, respectively.

Finally, if you reconsider the preciousness of the data you can obliterate it by running something like this:

    ./delete_file_blocking localhost object_store
    ./delete_file_nonblocking localhost object_store

Similarly to their read counterparts, those examples are implemented by `src/delete_file_blocking.cc` and `src/delete_file_nonblocking.cc`.