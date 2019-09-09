# the command "make" compiles your program
# the command "make clean" removes all compiled files (but not your source code)
# the command "make run" compiles & runs your program
# the command "make clean run" compiles & runs your program from scratch

# modify the following line to the name of your program (without ".c")
PROGRAM=nameofyourcfilebutwithoutthedotattheend
LIBPYNQDIR=../libpynq_v2.2


### do not modify anything below this line ###

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
