.PHONY: clean

GCC = gcc
GCC_OPT = -m64 -Wall -g

TARGET = index

OBJS = main_old.o index.o hashes.o
#OBJS = index.o hashes.o
INC = -I./include

LIBS = -lpthread -lpcap 
#LIBS = -lpthread -lssl -lcrypto -lnuma -lps -lpcap -laio
#LIB_DIR = -L/usr/local/io_engine/lib

all: $(TARGET) clean

$(TARGET): $(OBJS)
	$(GCC) $(GCC_OPT) -o $@ $^ $(INC) $(LIB_DIR) $(LIBS)

.c.o:
	$(GCC) $(GCC_OPT) -o $@ -c $^ $(INC)

clean:
	rm -f $(OBJS) *~ ./include/*~

cleanall:
	rm -f $(TARGET)
