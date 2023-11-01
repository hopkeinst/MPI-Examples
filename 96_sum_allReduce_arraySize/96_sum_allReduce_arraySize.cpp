#include <stdio.h>
#include <mpi.h>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, totalProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcs);

    int vectorSize = 16;
    int localVectorSize = vectorSize / totalProcs;
    
    std::vector<double> globalVector(vectorSize, 0.0);
    std::vector<double> localVector(localVectorSize, 0.0);

    // Llenar el vector local en cada proceso
    for (int i = 0; i < localVectorSize; i++) {
        localVector[i] = rank;
    }

    // Realizar la suma de los vectores locales en el vector global
    MPI_Allreduce(localVector.data(), globalVector.data(), localVectorSize, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    // Imprimir el vector global en cada proceso

    if(rank == 0) {
        for (int i = 0; i < vectorSize; i++) {
            printf("Proceso %d: globalVector[%d] = %lf\n", i, i, globalVector[i]);
        }
    }

    MPI_Finalize();
    return 0;
}