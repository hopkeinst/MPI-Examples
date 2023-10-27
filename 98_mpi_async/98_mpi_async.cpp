#include <cmath>
#include <mpi.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

struct StateToSend {
    unsigned int state;
	double real;
	double imag;
};

void repeatChar(char c, int nC) {
	for(int i=0; i<nC; i++) {
		printf("%c", c);
	}
    printf("\n");
}

/// @brief Convert decimal number to binary number
/// @param decNum is the decimal number
/// @param maxBits is the number maximum of bits in the string
/// @return The number converted to binary, in string type
//------------------------------------------------------------------------------
std::string decimalToBinary(unsigned int decNum, unsigned int maxBits) {
	unsigned int nBits = ceil(log2(decNum+1));
	std::string s(maxBits, '0');
	if (nBits <= maxBits) {
		for (std::size_t i=0; i<maxBits; i++) {
			s[maxBits-1-i] = (decNum & 0x1) == 0 ? '0' : '1';
			decNum = decNum >> 1;
		}
	} else {
		for (std::size_t i=0; i<maxBits; i++) {
			s[i] = '-';
		}
	}
	return s;
}
 
int main(int argc, char *argv[]){

    int rank, size;

    unsigned int nQubits, nStates, nSends;
    nQubits = atoi(argv[1]);
    nStates = pow(2, nQubits);
    unsigned int maxBits = ceil(log2(nStates));
    unsigned int statesPerProc;
    int i, j;

    double start_clock;

    std::vector<double> amplitudes(nStates*2, 0.0);
 
    // Inicializa MPI
    MPI_Init(&argc, &argv);
    // Se obtiene la cantidad de procesos y asigna a size
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // Se obtiene el numero del proceso y asigna a rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // El tipo de dato de la estructura, en MPI
    MPI_Datatype structToSend;
    MPI_Type_contiguous(3, MPI_DOUBLE, &structToSend);
    MPI_Type_commit(&structToSend);

    start_clock = MPI_Wtime();

    // Semilla para el random
    srand(rank + time(NULL));

    // // ENVÍO // //
    // Cantidad de mensajes a enviar
    nSends = rand()%(nStates+1);
    if(nSends > 0) {
        for(i=0; i<nSends; i++) {
            struct StateToSend data;
            data.state = rand()%nStates;
            while(data.state == rank) {
                data.state = rand()%nStates;
            }
            data.real = ((rand()/(double)RAND_MAX)*2.0)-1.0;
            data.imag = ((rand()/(double)RAND_MAX)*2.0)-1.0;
            MPI_Request request;
            MPI_Status status;
            MPI_Isend(
                &data,
                1,
                structToSend,
                data.state,
                0,
                MPI_COMM_WORLD,
                &request
            );
            printf("TX: Envía: %d -> A: %d | %18.15lf, %18.15lf\n", rank, data.state, data.real, data.imag);
            MPI_Wait(&request, &status);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // // RECEPCIÓN // //
    for(i=0; i<size; i++) {
        int flag;
        MPI_Status status;
        MPI_Iprobe(
            MPI_ANY_SOURCE, 
            0, 
            MPI_COMM_WORLD, 
            &flag, 
            &status
        );
        if(flag) {
            struct StateToSend data_recv;
            MPI_Request request_recv;
            MPI_Recv(
                &data_recv,
                1,
                structToSend,
                MPI_ANY_SOURCE,
                0,
                MPI_COMM_WORLD,
                &status
            );
            printf(" RX: Recibe: %d <- De: %d | %18.15lf, %18.15lf\n", data_recv.state, status.MPI_SOURCE, data_recv.real, data_recv.imag);
            amplitudes[data_recv.state*2] += data_recv.real;
            amplitudes[(data_recv.state*2)+1] += data_recv.imag;
        }
    }

    // Se finaliza el MPI
    MPI_Finalize();

    repeatChar('=', 60);
    for (int i = 0; i < size; i++) {
        if (rank == i) {
            printf("%d | %s > %18.15lf\t%18.15lf\n", i, decimalToBinary(i, maxBits).c_str(), amplitudes[i*2], amplitudes[(i*2)+1]);
        }
    }
    printf("\tIteration took %.6f seconds.\n", (MPI_Wtime() - start_clock));

    return 0;

}