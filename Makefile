evolution-notmuch-server: evolution-notmuch-server.c
	gcc `pkg-config gio-2.0 glib-2.0 --libs --cflags` -lnotmuch $< -o $@
