# supermagic

## What is supermagic?
supermagic is a very simple MPI sanity code. Nothing more, nothing less.

## Latest Distributions
Distribution tarballs are found here: http://hpc.github.com/supermagic

## Getting and Configuring supermagic
Clone this repository:
```shell
git clone https://github.com/hpc/supermagic.git
```

Run autogen to generate the configure script:
```shell
cd supermagic
# Note that this step is skipped when building from a distribution tarball.
./autogen
```

Run configure with the required options. Some examples include:
```shell
# Example 1: Using mpicc as the C wrapper compiler.
./configure CC=mpicc
# Example 2: Adding an installation prefix.
./configure CC=mpicc --prefix=$HOME/.local
```

## Building supermagic: MPI-Only
```shell
make
```

### An example using modules and `mpicc`:
```shell
module load openmpi-gcc
make
```

### An example with verbose build output
```shell
make V=1
```

## Building supermagic: MPI + Cell
```shell
make cell
```

### An example using modules, mpicc, and cellsdk
```shell
module load openmpi-gcc cellsdk/3.1
make cell
```

## supermagic Usage
```
Usage:
    mpirun -np N ./supermagic [OPTIONS]

options:
    [-a|--all]                   run all tests in suite
    [-h|--help]                  display this message
    [-m|--msg-size x[B,k,M,G]]   change message size
    [-M|--file-size B[B,k,M,G]]  change file size (per rank)
    [-n|--n-iters X]             run X iterations of a test suite
    [-q|--quiet]                 run in quiet mode
    [-s|--stat /a/path]          add /a/path to stat list
    [-t|--with-tests t1[,t2,tn]] run tests in requested order
    [-w|--write /a/path]         add /a/path to IO tests
    [-V|--verbose]               display verbose output

Available tests:
    hostname_exchange
    stat_paths
    mpi_io
    n_to_n_io
    small_all_to_all_ptp
    small_allreduce_max
    alt_sendrecv_ring
    root_bcast
    rand_root_bcast
    large_sendrecv_ring
    rand_root_bcast
    large_all_to_root_ptp
    large_all_to_all_ptp
    hello_world
# cell_sanity only available when cell support is requested via "make cell"
    cell_sanity
```

For example
```shell
mpirun -np 4 ./supermagic -s /glob/usr/file -s /usr/proj -n 2
  ```

### Example 1: Basic Usage
```shell
mpirun ./supermagic
```

### Example 2: Script that tests the system before application execution
```shell
  mpirun ./supermagic
  if [[ $? != 0 ]]
  then
      exit 1;
  fi
  # supermagic didn't detect any errors. Run my real application now:
  mpirun ./my_real_app
```

### Example 3: Running a custom set of tests
```shell
# First runs mpi_io test then runs cell_sanity test
mpirun ./supermagic -t mpi_io,cell_sanity -w /scratch1/jess/my_data_dir
```

## supermagic Best Practices
In general, it is best to run supermagic in a way that closely mimics the way in
which you run your real target application.  For example, if you provide a list
of MPI parameters that change the way in which your MPI implementation behaves,
please also include those parameters when running supermagic.

### Example 4: Open MPI MCA parameters
```shell
mpirun -mca a_parameter -mca another ./supermagic

if [[ $? != 0 ]]
then
  exit 1;
fi
# supermagic didn't detect any errors, run my real application now:
mpirun -mca a_parameter -mca another ./my_real_app
```

## Frequently Asked Questions

Q: "UNKNOWN" host names are not very useful.  How can I get useful host names?

A: Run hostname_exchange first.  This will populate a host name lookup table and
get rid of "UNKNOWN" host names.

For example:
```shell
mpirun ./supermagic -t hostname_exchange,rand_root_bcast
```
