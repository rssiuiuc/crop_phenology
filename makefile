CC = mpic++
CFLAGS = -g -Wall -std=c++0x

all: pheno

Network.o: Network.h Network.cpp
	$(CC) $(CFLAGS) -c Network.cpp
NetworkUtils.o: NetworkUtils.cpp Network.h
	$(CC) $(CFLAGS) -c NetworkUtils.cpp
Utils.o: Utils.h Utils.cpp
	$(CC) $(CFLAGS) -c Utils.cpp
PhenoNet.o: PhenoNet.h PhenoNet.cpp Network.h TimeSeries.h
	$(CC) $(CFLAGS) -c PhenoNet.cpp
pheno: TimeSeries.h TimeSeriesDecomposition.h Network.o NetworkUtils.o Utils.o PhenoNet.o
	$(CC) $(CFLAGS) Pheno.cpp -o pheno Network.o NetworkUtils.o Utils.o PhenoNet.o

clean:
	$(RM) pheno *.o *~
