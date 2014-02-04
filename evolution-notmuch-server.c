#include <glib.h>
#include <notmuch.h>

int main (int argc, char** argv) {
  notmuch_status_t    status;
  notmuch_database_t *db;

  if (argc < 2)
  {
    g_warning ("Usage: %s EVOLUTION_MAILDIR", argv[0]);
    return 1;
  }

  if (status = notmuch_database_create (argv[1], &db)) {
      g_warning ("Could not create database: %d", status);
  }

  return 0;
}
