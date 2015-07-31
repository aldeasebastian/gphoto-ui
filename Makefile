all: ui
ui: ui.c ui.h gphotoapi.a
	gcc `pkg-config --cflags gtk+-3.0` -Wall -o gphoto-ui ui.c `pkg-config --libs gtk+-3.0` -rdynamic -Lgphoto_api -lgphotoapi -lgphoto2


gphotoapi.a: force_look
	cd ./gphoto_api; make lib

clean:
	rm -f gphoto-ui
	cd ./gphoto_api; make clean

force_look: 
	true
