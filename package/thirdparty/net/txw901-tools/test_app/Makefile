
CFLAGS += -D__linux__
LDFLAGS += -lm
	
# tcp_test: tcp_test.c
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LDLIBS);mv -f $@ bin/

# dsleep_test: dsleep_test.c
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LDLIBS);mv -f $@ bin/

# hgota: libota.o hgota.o fwinfo.o
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ libota.o hgota.o fwinfo.o $(LDLIBS);mv -f $@ bin/

# hguevent: hguevent.c
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LDLIBS);mv -f $@ bin/

# libnetat: libnetat.c
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LDLIBS);mv -f $@ bin/

# hgics: hgics.o blenc.o hgic_blenc.o hgics_relay.o
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ hgics.o hgic_blenc.o hgics_relay.o blenc.o $(LDLIBS)

hgics: hgics.o blenc.o hgic_blenc.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ hgics.o hgic_blenc.o $(LDLIBS)

# hgicf: hgicf.o blenc.o hgic_blenc.o 
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ hgicf.o hgic_blenc.o blenc.o $(LDLIBS);mv -f $@ bin/

# hgpriv: hgpriv.c
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.c $(LDLIBS);mv -f $@ bin/

all: hgics

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

