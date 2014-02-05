#include <glib.h>
#include <gio/gio.h>

#include <notmuch.h>

void scan_directory (notmuch_database_t *db, GFile *dir)
{
  GError          *error;
  GFile           *db_folders;
  GFileEnumerator *erator;
  GFileInfo       *info;

  db_folders = g_file_get_child (dir, "folders");
  if (!g_file_query_exists (db_folders, NULL))
  {
    gchar *path = g_file_get_path (db_folders);
    g_error ("%s does not exists", path);
    g_free (path);
    g_object_unref (db_folders);
    return;
  }

  erator = g_file_enumerate_children (db_folders,
                                      G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                      G_FILE_QUERY_INFO_NONE,
                                      NULL,
                                      &error);

  info = g_file_enumerator_next_file (erator, NULL, &error);
  while (info)
  {
    const gchar *name = g_file_info_get_name (info);
    GFileAttributeType type = g_file_info_get_file_type (info);

    /* TODO: check for cur/tmp/new and subfolders/ */
    g_message ("%s %d", name, type);

    g_object_unref (info);
    info = g_file_enumerator_next_file (erator, NULL, &error);
  }

  g_object_unref (erator);
  g_object_unref (db_folders);
}


int main (int argc, char** argv) {
  GFile *db_dir, *db_file;
  notmuch_status_t    status;
  notmuch_database_t *db;
  GMainLoop *loop;

  if (argc != 2)
  {
    g_warning ("Usage: %s EVOLUTION_MAILDIR", argv[0]);
    return 1;
  }

  db_dir = g_file_new_for_path (argv[1]);
  db_file = g_file_get_child (db_dir, ".notmuch");

  if (!g_file_query_exists (db_dir, NULL))
  {
    g_object_unref (db_dir);
    g_object_unref (db_file);
    g_error ("directory %s does not exists");
    return 2;
  }

  if (!g_file_query_exists (db_file, NULL))
  {
    status = notmuch_database_create (argv[1], &db);
    if (status)
    {
      g_error ("Could not create database: %d", status);
      g_object_unref (db_dir);
      g_object_unref (db_file);
      notmuch_database_destroy (db);
      return 3;
    }
  }

  scan_directory (db, db_dir);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);
  return 0;
}
