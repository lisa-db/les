PROGRAM=./week2/1-ledjes
LIBPYNQDIR=../libpynq_v2.2
PLATFORM=PYNQ
ifeq ($(PLATFORM),PYNQ)
	CFLAGS=-Wall -Wno-format-security -g -I$(LIBPYNQDIR) -D$(PLATFORM)
	LDFLAGS=-lm -lX11 -pthread
endif

LIBPYNQ=libpynq

all: $(PROGRAM)

run: $(PROGRAM)
	-./$(PROGRAM) || true

$(LIBPYNQ).o: $(LIBPYNQDIR)/$(LIBPYNQ).[ch]
	gcc -c $(CFLAGS) $(LIBPYNQDIR)/$(LIBPYNQ).c

$(PROGRAM): $(LIBPYNQ).o $(PROGRAM).c
	gcc $(CFLAGS) -o $(PROGRAM) $(PROGRAM).c $(LIBPYNQ).o $(LDFLAGS) 

clean:
	-rm -f $(PROGRAM) $(PROGRAM).o $(LIBPYNQ).o
	-rm -rf $(PROGRAM).dSYM
