include Make.defines

PROGS =	 client server

OPTIONS = -DUNIX  -DANSI


#COBJECTS =	DieWithError.o
#CSOURCES =	DieWithError.c

CPLUSOBJECTS =

COMMONSOURCES =

CPLUSSOURCES =

all:	${PROGS}


client:		client.o $(CPLUSOBJECTS) $(COBJECTS) $(LIBS) $(COMMONSOURCES) $(SOURCES)
		${CC} ${LINKOPTIONS}  $@ client.o $(CPLUSOBJECTS) $(COBJECTS) $(LIBS) $(LINKFLAGS)

sleeptest:	sleeptest.o $(CPLUSOBJECTS) $(COBJECTS) $(LIBS) $(COMMONSOURCES) $(SOURCES)
		${CC} ${LINKOPTIONS}  $@ sleeptest.o $(CPLUSOBJECTS) $(COBJECTS) $(LIBS) $(LINKFLAGS)

server:		server.o $(CPLUSOBJECTS) $(COBJECTS)
		${CC} ${LINKOPTIONS} $@ server.o $(CPLUSOBJECTS) $(COBJECTS) $(LIBS) $(LINKFLAGS)


.cc.o:	$(HEADERS)
	$(CPLUS) $(CPLUSFLAGS) $(OPTIONS) $<

.c.o:	$(HEADERS)
	$(CC) $(CFLAGS) $(OPTIONS) $<



backup:
	rm -f UDPEcho2.tar
	rm -f UDPEcho2.tar.gz
	tar -cf UDPEcho2.tar *
	gzip -f UDPEcho2.tar

clean:
		rm -f ${PROGS} ${CLEANFILES}

depend:
		makedepend client.c server.c $(CFLAGS) $(HEADERS) $(SOURCES) $(COMMONSOURCES) $(CSOURCES)
#		mkdep $(CFLAGS) $(HEADERS) $(SOURCES) $(COMMONSOURCES) $(CSOURCES)

# DO NOT DELETE

UDPEchoClient2.o: UDPEcho.h /usr/include/stdio.h /usr/include/features.h
UDPEchoClient2.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
UDPEchoClient2.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
UDPEchoClient2.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
UDPEchoClient2.o: /usr/include/libio.h /usr/include/_G_config.h
UDPEchoClient2.o: /usr/include/wchar.h /usr/include/bits/wchar.h
UDPEchoClient2.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
UDPEchoClient2.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
UDPEchoClient2.o: /usr/include/sys/types.h /usr/include/time.h
UDPEchoClient2.o: /usr/include/endian.h /usr/include/bits/endian.h
UDPEchoClient2.o: /usr/include/sys/select.h /usr/include/bits/select.h
UDPEchoClient2.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
UDPEchoClient2.o: /usr/include/sys/sysmacros.h
UDPEchoClient2.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
UDPEchoClient2.o: /usr/include/errno.h /usr/include/bits/errno.h
UDPEchoClient2.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
UDPEchoClient2.o: /usr/include/asm-generic/errno.h
UDPEchoClient2.o: /usr/include/asm-generic/errno-base.h /usr/include/string.h
UDPEchoClient2.o: /usr/include/netinet/in.h /usr/include/stdint.h
UDPEchoClient2.o: /usr/include/sys/socket.h /usr/include/sys/uio.h
UDPEchoClient2.o: /usr/include/bits/uio.h /usr/include/bits/socket.h
UDPEchoClient2.o: /usr/include/limits.h /usr/include/bits/posix1_lim.h
UDPEchoClient2.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
UDPEchoClient2.o: /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h
UDPEchoClient2.o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
UDPEchoClient2.o: /usr/include/bits/in.h /usr/include/bits/byteswap.h
UDPEchoClient2.o: /usr/include/arpa/inet.h /usr/include/unistd.h
UDPEchoClient2.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
UDPEchoClient2.o: /usr/include/getopt.h /usr/include/netdb.h
UDPEchoClient2.o: /usr/include/rpc/netdb.h /usr/include/bits/netdb.h
UDPEchoClient2.o: /usr/include/signal.h /usr/include/bits/signum.h
UDPEchoClient2.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
UDPEchoClient2.o: /usr/include/bits/sigcontext.h
UDPEchoClient2.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
UDPEchoClient2.o: /usr/include/bits/sigthread.h /usr/include/sys/time.h
UDPEchoServer.o: UDPEcho.h /usr/include/stdio.h /usr/include/features.h
UDPEchoServer.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
UDPEchoServer.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
UDPEchoServer.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
UDPEchoServer.o: /usr/include/libio.h /usr/include/_G_config.h
UDPEchoServer.o: /usr/include/wchar.h /usr/include/bits/wchar.h
UDPEchoServer.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
UDPEchoServer.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
UDPEchoServer.o: /usr/include/sys/types.h /usr/include/time.h
UDPEchoServer.o: /usr/include/endian.h /usr/include/bits/endian.h
UDPEchoServer.o: /usr/include/sys/select.h /usr/include/bits/select.h
UDPEchoServer.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
UDPEchoServer.o: /usr/include/sys/sysmacros.h
UDPEchoServer.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
UDPEchoServer.o: /usr/include/errno.h /usr/include/bits/errno.h
UDPEchoServer.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
UDPEchoServer.o: /usr/include/asm-generic/errno.h
UDPEchoServer.o: /usr/include/asm-generic/errno-base.h /usr/include/string.h
UDPEchoServer.o: /usr/include/netinet/in.h /usr/include/stdint.h
UDPEchoServer.o: /usr/include/sys/socket.h /usr/include/sys/uio.h
UDPEchoServer.o: /usr/include/bits/uio.h /usr/include/bits/socket.h
UDPEchoServer.o: /usr/include/limits.h /usr/include/bits/posix1_lim.h
UDPEchoServer.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
UDPEchoServer.o: /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h
UDPEchoServer.o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
UDPEchoServer.o: /usr/include/bits/in.h /usr/include/bits/byteswap.h
UDPEchoServer.o: /usr/include/arpa/inet.h /usr/include/unistd.h
UDPEchoServer.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
UDPEchoServer.o: /usr/include/getopt.h
DieWithError.o: /usr/include/stdio.h /usr/include/features.h
DieWithError.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
DieWithError.o: /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h
DieWithError.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
DieWithError.o: /usr/include/libio.h /usr/include/_G_config.h
DieWithError.o: /usr/include/wchar.h /usr/include/bits/wchar.h
DieWithError.o: /usr/include/gconv.h /usr/include/bits/stdio_lim.h
DieWithError.o: /usr/include/bits/sys_errlist.h /usr/include/stdlib.h
DieWithError.o: /usr/include/sys/types.h /usr/include/time.h
DieWithError.o: /usr/include/endian.h /usr/include/bits/endian.h
DieWithError.o: /usr/include/sys/select.h /usr/include/bits/select.h
DieWithError.o: /usr/include/bits/sigset.h /usr/include/bits/time.h
DieWithError.o: /usr/include/sys/sysmacros.h /usr/include/bits/pthreadtypes.h
DieWithError.o: /usr/include/alloca.h
