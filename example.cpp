#include <mpitest/mpitest.hpp>
#include <iostream>

TEST(suite0,test0){
    GLOBAL_ASSERT(true) << "message";
}

TEST(suite1,test0){
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    if (rank == 2) {
        LOCAL_ASSERT(false) << "debug message";
        // GLOBAL_ASSERT(true); <-- would deadlock
    }
}