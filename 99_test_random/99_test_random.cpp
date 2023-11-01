#include <cmath>
#include <mpi.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

struct StateToSend {
    unsigned int stateSource;
    unsigned int stateTarget;
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

    // thisProc -> número de este proceso
    // totalProcs -> el total de procesos, de MPI
    int thisProc, totalProcs;

    unsigned int totalQubits, totalStates, totalMsgSend;
    // Se lee la cantidad de qubits
    totalQubits = atoi(argv[1]);
    // Se establece la cantidad de estados
    totalStates = pow(2, totalQubits);
    unsigned int maxBits = ceil(log2(totalStates));
    int i, j;

    unsigned int chunkSize = 0;

    double start_clock;
 
    // Inicializa MPI
    MPI_Init(&argc, &argv);
    // Se obtiene la cantidad de procesos y asigna a totalProcs
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcs);
    // Se obtiene el numero del proceso y asigna a thisProc
    MPI_Comm_rank(MPI_COMM_WORLD, &thisProc);

    // Arreglo que va a guardar la cantidad de mensajes que va a llegar a cada proceso
    //unsigned int 

    // El tipo de dato de la estructura, en MPI
    MPI_Datatype structToSend;
    MPI_Type_contiguous(3, MPI_DOUBLE, &structToSend);
    MPI_Type_commit(&structToSend);

    start_clock = MPI_Wtime();

    // Semilla para el random
    srand(thisProc + time(NULL));

    // Calcular el tamaño de lo que se va a manejar local
    chunkSize = totalStates/totalProcs;

    // Donde voy a guardar los procesos locales
    std::vector<double> amplitudesLocal(chunkSize*2, 0.0);

    // Para almacenar cuantos mensajes se pasan a otros procesos
    std::vector<int> sizeMsgRecvGlobal(totalProcs, 0);
    std::vector<int> sizeMsgInteGlobal(totalProcs, 0);

    if(thisProc == 0) {
        printf("totalQubits = %d\n", totalQubits);
        printf("totalStates = %d\n", totalStates);
        printf("chunkSize = %d\n", chunkSize);
        printf(" ");
        repeatChar('=', 85);
        printf(" | jobType | staGlo | proc | staLoc | dir | staGlo | proc | staLoc |  real  |  imag  |\n");
        printf(" ");
        repeatChar('=', 85);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // // ENVÍO // //
    // Cantidad de mensajes a enviar
    std::vector<int> sizeMsgRecvLocal(totalProcs, 0);
    std::vector<int> sizeMsgInteLocal(totalProcs, 0);
    for(j=0; j<chunkSize; j++) { // j = state local
        totalMsgSend = rand()%(totalStates+1);
        if(totalMsgSend > 0) {
            for(i=0; i<totalMsgSend; i++) {
                int procTarget;
                int stateLocalSource, stateLocalTarget;
                struct StateToSend dataSend;
                dataSend.stateSource = j+(thisProc*chunkSize);
                dataSend.stateTarget = rand()%totalStates;
                while(dataSend.stateTarget == (j+(thisProc*chunkSize))) { // si es este mismo estado
                    dataSend.stateTarget = rand()%totalStates;
                }
                dataSend.real = rand()%3-1.0;
                dataSend.imag = rand()%3-1.0;
                //dataSend.real = ((rand()/(double)RAND_MAX)*2.0)-1.0;
                //dataSend.imag = ((rand()/(double)RAND_MAX)*2.0)-1.0;
                procTarget = (int)floor(dataSend.stateTarget/chunkSize);
                stateLocalSource = dataSend.stateSource - (thisProc*chunkSize);
                stateLocalTarget = dataSend.stateTarget - (procTarget*chunkSize);
                printf(" | Tx      | %6d | %4d | %6d | |-> | %6d | %4d | %6d | %6.1f | %6.1f |", dataSend.stateSource, thisProc, stateLocalSource, dataSend.stateTarget, procTarget, stateLocalTarget, dataSend.real, dataSend.imag);
                if(procTarget == thisProc) {
                    sizeMsgInteLocal[procTarget] += 1;
                    amplitudesLocal[(stateLocalTarget*2)] += dataSend.real;
                    amplitudesLocal[(stateLocalTarget*2)+1] += dataSend.imag;
                    printf(" *");
                } else {
                    MPI_Request requestSend;
                    MPI_Status statusSend;
                    MPI_Isend(
                        &dataSend,
                        1,
                        structToSend,
                        procTarget,
                        0,
                        MPI_COMM_WORLD,
                        &requestSend
                    );
                    sizeMsgRecvLocal[procTarget] += 1;
                }
                printf("\n");
            }
        }
    }
    //MPI_Barrier(MPI_COMM_WORLD);

    // Sumar los locales al global
    MPI_Allreduce(sizeMsgRecvLocal.data(), sizeMsgRecvGlobal.data(), totalProcs, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(sizeMsgInteLocal.data(), sizeMsgInteGlobal.data(), totalProcs, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    // // RECEPCIÓN // //
    int hasMsg = 0;
    int cntRecv = 0;
    
    do {
        MPI_Status statusRecv;
        MPI_Iprobe(
            MPI_ANY_SOURCE, 
            0, 
            MPI_COMM_WORLD, 
            &hasMsg, 
            &statusRecv
        );
        if(hasMsg) {
            struct StateToSend dataRecv;
            MPI_Request requestRecv;
            int stateLocalSource, stateLocalTarget;
            MPI_Recv(
                &dataRecv,
                1,
                structToSend,
                MPI_ANY_SOURCE,
                0,
                MPI_COMM_WORLD,
                &statusRecv
            );
            stateLocalSource = dataRecv.stateSource-(statusRecv.MPI_SOURCE*chunkSize);
            stateLocalTarget = dataRecv.stateTarget-(thisProc*chunkSize);
            printf(" |      Rx | %6d | %4d | %6d | <-| | %6d | %4d | %6d | %6.1f | %6.1f |\n", dataRecv.stateTarget, thisProc, stateLocalTarget, dataRecv.stateSource, statusRecv.MPI_SOURCE, stateLocalSource, dataRecv.real, dataRecv.imag);
            amplitudesLocal[(stateLocalTarget*2)] += dataRecv.real;
            amplitudesLocal[(stateLocalTarget*2)+1] += dataRecv.imag;
            cntRecv += 1;
        }
        
    } while(cntRecv < sizeMsgRecvGlobal[thisProc]);

    // Se finaliza el MPI
    MPI_Finalize();

    if(thisProc == 0) {
        printf(" ");
        repeatChar('=', 85);
        for(i = 0; i<totalProcs; i++) {
            printf("Proceso %d recibe %4d mensajes de otros procesos y %4d del mismo proceso\n", i, sizeMsgRecvGlobal[i], sizeMsgInteGlobal[i]);
        }
        printf(" ");
        repeatChar('=', 53);
        printf(" | staGlo | binGlo | proc | staLoc |  real  |  imag  |\n");
        printf(" ");
        repeatChar('=', 53);
    }

    for(i=0; i<totalProcs; i++) {
        if(i == thisProc) {
            for(j=0; j<chunkSize; j++) {
                    printf(" | %6d | %6s | %4d | %6d | %6.1f | %6.1f |\n", (i*chunkSize)+j, decimalToBinary((i*chunkSize)+j, maxBits).c_str(), i, j, amplitudesLocal[(j*2)], amplitudesLocal[(j*2)+1]);
            }
        }
    }
        
    printf(" ");
    repeatChar('=', 53);
    
    return 0;

}