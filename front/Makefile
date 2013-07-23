.PHONY: clean

GCC = gcc
GCC_OPT = -m64 -Wall -O2

TARGET = server

OBJS = server.o
INC = -I./include

LIBS = -lpthread -lpcap 

all: $(TARGET) clean

$(TARGET): $(OBJS)
	$(GCC) $(GCC_OPT) -o $@ $^ $(INC) $(LIB_DIR) $(LIBS)

.c.o:
	$(GCC) $(GCC_OPT) -o $@ -c $^ $(INC)

clean:
	rm -f $(OBJS) *~ ./include/*~

cleanall:
	rm -f $(TARGET)
