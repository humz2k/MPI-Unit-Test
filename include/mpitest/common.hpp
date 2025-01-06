#ifndef _MPITEST_COMMON_HPP_
#define _MPITEST_COMMON_HPP_

#include <mpi.h>
#include <string>

namespace mpitest::mpihelpers {

inline int comm_rank(MPI_Comm comm = MPI_COMM_WORLD) {
    int rank;
    MPI_Comm_rank(comm, &rank);
    return rank;
}

inline int comm_size(MPI_Comm comm = MPI_COMM_WORLD) {
    int size;
    MPI_Comm_size(comm, &size);
    return size;
}

inline bool global_reduce(bool condition) {
    int out;
    int in = condition;
    MPI_Allreduce(&in, &out, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    int comm_size;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    return out == comm_size;
}

inline void send_string(const std::string str, const int rank) {
    std::size_t sz = str.size();
    MPI_Send(&sz, sizeof(std::size_t), MPI_BYTE, rank, 0, MPI_COMM_WORLD);
    if (sz > 0)
        MPI_Send(str.c_str(), sz, MPI_CHAR, rank, 0, MPI_COMM_WORLD);
}

inline std::string recv_string(const int rank) {
    std::size_t sz;
    MPI_Recv(&sz, sizeof(std::size_t), MPI_BYTE, rank, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    if (sz > 0) {
        std::string out;
        out.resize(sz);
        MPI_Recv(out.data(), sz, MPI_CHAR, rank, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        return out;
    }
    return "";
}

} // namespace mpitest

#endif // _MPITEST_COMMON_HPP_