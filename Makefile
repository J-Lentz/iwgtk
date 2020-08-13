CC=gcc
CFLAGS=-O3 `pkg-config --cflags gtk+-3.0`
LDLIBS=`pkg-config --libs gtk+-3.0`

ICON_LIST=resources/*.svg
HEADER_LIST=iwgtk.h main.h dialog.h objects.h adapter.h device.h station.h wps.h ap.h adhoc.h utilities.h switch.h known_network.h network.h agent.h icons.h
OBJECT_LIST=main.o dialog.o objects.o adapter.o device.o station.o wps.o ap.o adhoc.o utilities.o switch.o known_network.o network.o agent.o icons.o

.PHONY : clean install

iwgtk : $(HEADER_LIST) $(OBJECT_LIST)
	$(CC) $(CFLAGS) -o $@ $(OBJECT_LIST) $(LDLIBS)

icons.c : icons.gresource.xml $(ICON_LIST)
	glib-compile-resources --generate-source icons.gresource.xml

icons.h : icons.gresource.xml $(ICON_LIST)
	glib-compile-resources --generate-header icons.gresource.xml

install : iwgtk
	install iwgtk /usr/local/bin

clean :
	rm -f *.o icons.c icons.h
