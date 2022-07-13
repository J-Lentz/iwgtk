CC?=gcc
PREFIX?=/usr/local

CCINCS=`pkg-config --cflags gtk4 libqrencode`
LDLIBS=`pkg-config --libs gtk4 libqrencode`

bindir=$(PREFIX)/bin
confdir=/etc
autostartdir=$(confdir)/xdg/autostart
datadir=$(PREFIX)/share
unitdir=$(PREFIX)/lib/systemd/user
mandir=$(datadir)/man
desktopdir=$(datadir)/applications
icondir=$(datadir)/icons/hicolor/scalable/apps

srcdir=src
files=sni main window indicator dialog adapter device station dpp wps diagnostic ap adhoc utilities icon switch known_network network hidden agent

headers=$(patsubst %,$(srcdir)/%.h,$(files) iwgtk)
objects=$(patsubst %,%.o,$(files))

.PHONY : clean install uninstall

iwgtk : $(objects)
	$(CC) $(CCINCS) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o : $(srcdir)/%.c $(headers)
	$(CC) -c $(CCINCS) $(CFLAGS) -o $@ $<

iwgtk.%.gz : misc/iwgtk.%.scd
	scdoc < $< | gzip > $@

install : iwgtk iwgtk.1.gz iwgtk.5.gz
	install -d $(DESTDIR)$(bindir)
	install iwgtk $(DESTDIR)$(bindir)
	install -d $(DESTDIR)$(confdir)
	install -m 644 misc/iwgtk.conf $(DESTDIR)$(confdir)
	install -d $(DESTDIR)$(desktopdir)
	install -m 644 misc/iwgtk.desktop $(DESTDIR)$(desktopdir)
	install -d $(DESTDIR)$(autostartdir)
	install -m 644 misc/iwgtk-indicator.desktop $(DESTDIR)$(autostartdir)
	install -d $(DESTDIR)$(unitdir)
	install -m 644 misc/iwgtk.service $(DESTDIR)$(unitdir)
	install -d $(DESTDIR)$(mandir)/man1
	install -m 644 iwgtk.1.gz $(DESTDIR)$(mandir)/man1
	install -d $(DESTDIR)$(mandir)/man5
	install -m 644 iwgtk.5.gz $(DESTDIR)$(mandir)/man5
	install -d $(DESTDIR)$(icondir)
	install -m 644 misc/iwgtk.svg $(DESTDIR)$(icondir)

uninstall :
	rm $(DESTDIR)$(bindir)/iwgtk
	rm $(DESTDIR)$(confdir)/iwgtk.conf
	rm $(DESTDIR)$(desktopdir)/iwgtk.desktop
	rm $(DESTDIR)$(autostartdir)/iwgtk-indicator.desktop
	rm $(DESTDIR)$(unitdir)/iwgtk.service
	rm $(DESTDIR)$(mandir)/man1/iwgtk.1.gz
	rm $(DESTDIR)$(mandir)/man5/iwgtk.5.gz
	rm $(DESTDIR)$(icondir)/iwgtk.svg

clean :
	rm -f iwgtk *.o iwgtk.*.gz
