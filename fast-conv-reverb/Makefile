CC=g++
CFLAGS=-O2 -Wall -I./include
LDFLAGS=-L./lib -lsndfile -lfftw3f-3

SRCS=main.cpp src/libHybridConv_eapo.cpp
OBJS=$(SRCS:.cpp=.o)
TARGET=fast_conv.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q *.o src\*.o $(TARGET)

run: $(TARGET)
	.\$(TARGET)

