#ifndef _MPITEST_MPITEST_HPP_
#define _MPITEST_MPITEST_HPP_

#include "assertion.hpp"
#include "test.hpp"

#ifndef MPITEST_NO_MAIN
int main() {
    MPI_Init(NULL, NULL);
    mpitest::Test::run_all();
    MPI_Finalize();
    return 0;
}
#endif

#endif // _MPITEST_MPITEST_HPP_