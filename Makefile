evolution-notmuch-server: evolution-notmuch-server.c
	gcc -lnotmuch $< -o $@
