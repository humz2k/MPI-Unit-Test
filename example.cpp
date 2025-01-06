#include <mpitest/mpitest.hpp>
#include <iostream>

TEST(suite0,test0){
    GLOBAL_ASSERT(true) << "message";
}

TEST(suite0,test1){
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    if (rank == 2)
        LOCAL_ASSERT(false) << "debug message";
}