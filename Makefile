.PHONY: clean

GCC = gcc
GCC_OPT = -m64 -Wall -O3

TARGET = index

OBJS = main.o sorted_array.o bloom_filter.o avl_tree.o
INC = -I./include

#LIBS = -lpthread -lssl -lcrypto -lnuma -lps -lpcap -laio
#LIB_DIR = -L/usr/local/io_engine/lib

all: $(TARGET) clean

$(TARGET): $(OBJS)
	$(GCC) $(GCC_OPT) -o $@ $^ $(INC)
	#$(GCC) $(GCC_OPT) -o $@ $^ $(INC) $(LIB_DIR) $(LIBS)

.c.o:
	$(GCC) $(GCC_OPT) -o $@ -c $^ $(INC)

clean:
	rm -f $(OBJS) *~ ./include/*~

cleanall:
	rm -f $(TARGET)
