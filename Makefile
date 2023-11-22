all: armee sarmale

.PHONY: all server sarmale clean

# MAKEOPTS=#-j16 -l16
MANPREFIX = /usr/share/man
	
libs:
	$(MAKE) $(MAKEOPTS) -C ./Libs

armee: libs
	$(MAKE) $(MAKEOPTS) -C ./Armee

sarmale: libs
	$(MAKE) $(MAKEOPTS) -C ./Sarmale

clean:
	$(MAKE) $(MAKEOPTS) -C ./Armee clean
	$(MAKE) $(MAKEOPTS) -C ./Sarmale clean

install:
	$(MAKE) $(MAKEOPTS) -C ./Armee install
	$(MAKE) $(MAKEOPTS) -C ./Sarmale install
	cp armee.1 $(MANPREFIX)/man1/armee.1
	chmod 644 $(MANPREFIX)/man1/armee.1
