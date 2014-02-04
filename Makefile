evolution-notmuch-server: evolution-notmuch-server.c
	gcc `pkg-config glib-2.0 --libs --cflags` -lnotmuch $< -o $@
