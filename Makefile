CC=gcc
CFLAGS=`pkg-config --cflags gtk+-3.0` -O3
LDLIBS=`pkg-config --libs gtk+-3.0`

FILES=main dialog objects adapter device station wps ap adhoc utilities switch known_network network agent icons
ICONS=icons/*.svg

HEADERS=$(patsubst %,src/%.h,$(FILES) iwgtk)
OBJ=$(patsubst %,obj/%.o,$(FILES))

.PHONY : clean install

iwgtk : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

obj/%.o : src/%.c $(HEADERS) obj/
	$(CC) -c $(CFLAGS) -o $@ $<

obj/ :
	mkdir obj

src/icons.c : icons.gresource.xml $(ICONS)
	glib-compile-resources --generate-source $<
	mv icons.c src/

src/icons.h : icons.gresource.xml $(ICONS)
	glib-compile-resources --generate-header $<
	mv icons.h src/

install : iwgtk
	install iwgtk /usr/local/bin

clean :
	rm -rf obj src/icons.c src/icons.h
