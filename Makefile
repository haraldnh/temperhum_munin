.PHONY: all clean
.SUFFIXES:
.SUFFIXES: .c .o

CFLAGS = -Wall -Wextra -Wshadow -Werror -D_POSIX_C_SOURCE=200112L \
	 -D_ISOC99_SOURCE -D_SVID_SOURCE -O2
TEMPERHUM_OBJS=comms.o readings.o temperhum_munin.o
BINARY = temperhum_munin

all: $(BINARY)
clean:
	rm -f $(BINARY) *.o

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

temperhum_munin.o: readings.h comms.h
readings.o: readings.h comms.h
comms.o: comms.h

$(BINARY): $(TEMPERHUM_OBJS) Makefile
	$(CC) $(CFLAGS) -o $@ $(TEMPERHUM_OBJS) -lm
