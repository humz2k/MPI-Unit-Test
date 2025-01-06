# MPI-Unit-Test
Simple C++ header-only unit testing framework for MPI codes. Contributions welcome.

## Building
Either use CMake, or just point your compiler towards the headers. For example:
```bash
mpicxx -o tests tests.cpp -I/path/to/mpitest/headers
```
Then run the tests with `mpirun` or `mpiexec`. For example:
```bash
mpirun -n 4 ./tests
```

## Usage
Use the `TEST(suite_name, test_name)` macro to create a test, and `GLOBAL_ASSERT(condition)`/`LOCAL_ASSERT(condition)` to check assumptions. The intended usage is including `mpitest/mpitest.hpp` in a *single* source file. Shenanigans and weirdness may occur if you try and link together multiple files that include `mpitest/mpitest.hpp`.
```c++
#include <mpitest/mpitest.hpp>

// suite0::test0
TEST(suite0, test0) {
    // Your test code here
    // For example, global assertion:
    GLOBAL_ASSERT(true) << "Optional message if it fails.";
}

// suite0::test1
TEST(suite0, test1) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // For demonstration, fail on rank 2
    if (rank == 2) {
        LOCAL_ASSERT(false) << "This test fails only on rank 2.";
    }
}
```

### Custom main
By default, `mpitest` will define a `main` function. If you want to define your own main function, define `MPITEST_NO_MAIN` before include `mpitest/mpitest.hpp`. The default main function looks like:
```c++
int main() {
    MPI_Init(NULL, NULL);
    mpitest::Test::run_all();
    MPI_Finalize();
    return 0;
}
```

## Local vs Global Assertions
* **Local Assertion** (`LOCAL_ASSERT(condition)`): Checks a condition on each individual rank. If it fails on rank i, it prints an error from that rank, but no collective gathering of messages occurs.
* **Global Assertion** (`GLOBAL_ASSERT(condition)`): Checks a condition collectively. If any rank fails, the framework marks the entire test as failed and gathers error messages from all ranks to print on rank 0. This calls `MPI_Barrier(MPI_COMM_WORLD)`, so watch out for deadlocks.

Each assertion supports streaming an optional message for extra context on failure:
```c++
GLOBAL_ASSERT(some_condition) << "Explaining what went wrong";
LOCAL_ASSERT(x == 10) << "Expected x==10, got " << x;
```
