#include <mpi.h>
#include <cmath>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

    unsigned int nQubits, nStates;
    unsigned int maxBits = ceil(log2(nStates));
    unsigned int statesPerProc;
    int i, j;

    nQubits = atoi(argv[1]);
    nStates = pow(2, nQubits);

    repeatChar('=', 60);
    printf("Number of qubits: %d\nNumber of states: %d\n", nQubits, nStates);
    repeatChar('=', 60);
 
    // Inicializa MPI
    MPI_Init(&argc, &argv);
    // Se obtiene la cantidad de procesos y asigna a size
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // Se obtiene el numero del proceso y asigna a rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // El tipo de dato de la estructura, en MPI
    MPI_Datatype StateToSend;
    MPI_Type_contiguous(3, MPI_DOUBLE, &StateToSend);
    MPI_Type_commit(&StateToSend);

    // Semilla para el random
    srand(rank + time(NULL));

    statesPerProc = nStates/size;
    unsigned int nSends[statesPerProc];

    printf("\n");
    repeatChar('=', 60);
    printf("PROCESO # %d\n", rank);
    repeatChar('=', 60);
    printf("Cantidad de estados por este proceso: %d\n", statesPerProc);
    // La cantidad de mensajes a enviar para cada proceso
    for(i=0; i<statesPerProc; i++) {
        // Cantidad de mensajes a enviar por cada estado
        nSends[i] = rand() % (nStates+1);
        printf("El estado %d | %s > va a enviar %d mensajes\n", ((rank*statesPerProc)+i), decimalToBinary((rank*statesPerProc)+i, maxBits).c_str(), nSends[i]);
    }
    
    // Se finaliza el MPI
    MPI_Finalize();

    return 0;

}