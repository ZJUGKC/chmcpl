# Makefile for LZX ccompressor

OBJS2 = obj/lzx_layer_nonslide.o obj/lz_nonslide.o
CWARNFLAGS = -Wall -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Winline
CWARNFLAGS += -Wno-uninitialized 
COPTFLAGS = -O2
CFLAGS = $(COPTFLAGS) $(CWARNFLAGS) $(CPROGDEFS)
CPROGDEFS = -DLZ_ONEBUFFER -DLAZY
LIBS= -lm

all: obj/liblzxcomp.a

obj/liblzxcomp.a: $(OBJS2)
	rm -f $@
	ar -q $@ $^
	ranlib $@

obj/lz_nonslide.o: lz_nonslide.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/lzx_layer_nonslide.o: lzx_layer.c
	$(CC) $(CFLAGS) -DNONSLIDE -c -o $@ $^

clean:
	rm -f obj/*.o *~ \#*\# obj/liblzxcomp.a