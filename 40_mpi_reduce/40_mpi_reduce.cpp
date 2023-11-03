#include <cmath>
#include <mpi.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
 
int main(int argc, char *argv[]){

    // thisProc -> número de este proceso
    // totalProcs -> el total de procesos, de MPI
    int thisProc, totalProcs;

    int i, j;
 
    // Inicializa MPI
    MPI_Init(&argc, &argv);
    // Se obtiene la cantidad de procesos y asigna a totalProcs
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcs);
    // Se obtiene el numero del proceso y asigna a thisProc
    MPI_Comm_rank(MPI_COMM_WORLD, &thisProc);

    srand(time(NULL)*thisProc);

    unsigned int totalMsgSend = rand()%10;
    unsigned int dataSend = 0;
    unsigned int dataRecv = 0;
    unsigned int targetProc = 0;

    printf("This proc %2d send %2d messages\n", thisProc, totalMsgSend);

    for(i=0; i<totalMsgSend; i++) {
        targetProc = rand()%totalProcs;
        dataSend = (rand()%20)-10;
        MPI_Reduce(
            &dataSend,
            &dataRecv,
            1,
            MPI_INT,
            MPI_SUM,
            targetProc,
            MPI_COMM_WORLD
        );
        printf("  %2d -> %2d : %2d\n", thisProc, targetProc, dataSend);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    for(i=0; i<totalMsgSend; i++) {
        if(i == thisProc) {
            printf("This proc %2d recv %3d\n", thisProc, dataRecv);
        }
    }

    return 0;

}