PREFIX = /usr/local

myip: myip.c
	gcc -o myip myip.c

.PHONY: install
install: myip
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $< $(DESTDIR)$(PREFIX)/bin/myip

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/myip
