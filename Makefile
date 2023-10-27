CXX = mpicxx

LIBS += -L /opt/intel/oneapi/mpi/latest/lib/release -lmpi

all: hello_world send_receive mpi_async test_random

# 01 HELLO WORLD
hello_world:
	$(CXX) 01_hello_world/01_hello_world.cpp -o bin/01HelloWorld
	@echo "Generado el ejecutable bin/01HelloWorld"
	@echo "-----------------------------------------------------------"

# 02 SEND RECEIVE
send_receive:
	$(CXX) 02_send_receive/02_send_receive.cpp -o bin/02_send_receive
	@echo "Generado el ejecutable bin/02_send_receive"
	@echo "-----------------------------------------------------------"

# 98 MPI ASYNC
# Test para crear varios envíos de manera aleatorio, al igual que recibirlos, asincrónicos
mpi_async:
	$(CXX) 98_mpi_async/98_mpi_async.cpp -o bin/98_mpi_async
	$(CXX) 98_mpi_async/97_mpi_test.cpp -o bin/97_mpi_test
	@echo "Generado el ejecutable bin/97_mpi_test"
	@echo "-----------------------------------------------------------"

# 99 TEST RANDOM
# Test para crear varios envíos de manera aleatorio, al igual que recibirlos, asincrónicos
test_random:
	$(CXX) 99_test_random/99_test_random.cpp -o bin/99_test_random
	@echo "Generado el ejecutable bin/99_test_random"
	@echo "-----------------------------------------------------------"

