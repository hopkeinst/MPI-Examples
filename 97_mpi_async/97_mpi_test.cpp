#include <iostream>
#include <mpi.h>
#include <cstdlib>
#include <ctime>
#include <vector>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Semilla aleatoria
    std::srand(static_cast<unsigned int>(std::time(nullptr) + rank));

    // Generar un número aleatorio de mensajes a enviar
    int numMessagesToSend = std::rand() % 11; // Número entre 0 y 10

    // Generar una lista de procesos de destino aleatorios
    std::vector<int> randomDestinations;
    for (int i = 0; i < numMessagesToSend; i++) {
        int dest = std::rand() % size; // Destino aleatorio
        while (dest == rank) {
            // Asegurarse de que no se envíen mensajes a uno mismo
            dest = std::rand() % size;
        }
        randomDestinations.push_back(dest);
    }

    // Enviar los mensajes a los procesos de destino
    for (int i = 0; i < numMessagesToSend; i++) {
        int message = std::rand() % 101; // Mensaje aleatorio

        // Enviar el mensaje al proceso de destino
        MPI_Send(&message, 1, MPI_INT, randomDestinations[i], 0, MPI_COMM_WORLD);
        std::cout << "TX: Proceso " << rank << " envía " << message << " a proceso " << randomDestinations[i] << std::endl;
    }
    // Esperar para asegurarse de que todos los mensajes se envíen y reciban
    MPI_Barrier(MPI_COMM_WORLD);

    // Recibir mensajes de manera asincrónica
    std::vector<int> receivedValues(size, 0);
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            int flag;
            MPI_Status status;
            MPI_Iprobe(i, 0, MPI_COMM_WORLD, &flag, &status);
            MPI_Request request_recv;
            if (flag) {
                int receivedMessage;
                MPI_Recv(&receivedMessage, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
                std::cout << "  RX: Proceso " << rank << " recibe " << receivedMessage << " de proceso " << i << std::endl;
            
                // Acumular el valor en el vector
                receivedValues[rank] += receivedMessage;
            }
        }
    }
    // Esperar para asegurarse de que todos los mensajes se envíen y reciban
    MPI_Barrier(MPI_COMM_WORLD);

    if(rank == 0) {
        // Imprimir los valores acumulados en el vector
        std::cout << "Valores acumulados: ";
        std::cout << rank << ": " << receivedValues[rank] << ", ";
    } else {
        std::cout << rank << ": " << receivedValues[rank] << ", ";
    }

    MPI_Finalize();

    std::cout << std::endl;
    return 0;
}
