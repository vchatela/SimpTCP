#
# Makefile
#

### VARIABLES #################################################################
EXEC	= client server
CC	    = gcc
INCSDIR = ../inc
MACROS  = -D__DEBUG__=1
CCFLAGS = -Wall  -I$(INCSDIR) $(MACROS)
LDFLAGS = -lm -ldl -lpthread 

### RULES #####################################################################
.PHONY : all clean $(EXEC)

all: $(EXEC)

# Rules to build all object files
%.o: %.c
	$(CC) $(CCFLAGS) -c $^ -o $@

# Rules to clean up build dir
clean:
#	-rm *.o *.i *.s *~ $(EXEC)
	-rm *.o 

# Dependencies
simptcp_packet.c: $(INCSDIR)/simptcp_packet.h \
                  $(INCSDIR)/term_colors.h    \
                  $(INCSDIR)/term_io.h
simptcp_lib.c:   $(INCSDIR)/simptcp_lib.h   \
                  $(INCSDIR)/simptcp_packet.h \
                  $(INCSDIR)/simptcp_entity.h \
                  $(INCSDIR)/libc_socket.h    \
                  $(INCSDIR)/term_colors.h    \
                  $(INCSDIR)/term_io.h
simptcp_entity.c: $(INCSDIR)/simptcp_entity.h \
		  $(INCSDIR)/simptcp_lib.h   \
		  $(INCSDIR)/simptcp_packet.h   \
                  $(INCSDIR)/libc_socket.h    \
                  $(INCSDIR)/term_colors.h    \
                  $(INCSDIR)/term_io.h
simptcp_api.c:    $(INCSDIR)/simptcp_api.h    \
                  $(INCSDIR)/simptcp_lib.h   \
                  $(INCSDIR)/simptcp_entity.h   \
                  $(INCSDIR)/libc_socket.h    \
                  $(INCSDIR)/term_colors.h    \
                  $(INCSDIR)/term_io.h
libc_socket.c:    $(INCSDIR)/libc_socket.h    \
                  $(INCSDIR)/term_colors.h    \
                  $(INCSDIR)/term_io.h        

# Rules to build executables
client: client.o simptcp_api.o simptcp_packet.o simptcp_lib.o simptcp_entity.o libc_socket.o
	$(CC) $^ $(LDFLAGS) -o $@

server: server.o simptcp_api.o simptcp_packet.o simptcp_lib.o simptcp_entity.o libc_socket.o
	$(CC) $^ $(LDFLAGS) -o $@

# vim: set expandtab ts=4 sw=4 tw=80: 
