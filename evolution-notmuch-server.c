#include <glib.h>
#include <gio/gio.h>

#include <notmuch.h>

typedef void (*crawler_callback_t) (notmuch_database_t*, GFile*);
static  void index_subfolders_cb   (notmuch_database_t*, GFile*);

static void
folder_crawler (notmuch_database_t *db,
                GFile              *basedir,
                crawler_callback_t  cb)
{
  GError          *error = NULL;
  GFileEnumerator *erator;
  GFileInfo       *info;

  erator = g_file_enumerate_children (basedir,
                                      G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                      G_FILE_QUERY_INFO_NONE,
                                      NULL,
                                      &error);
  if (error)
  {
    g_error (error->message);
    g_error_free (error);
    return;
  }

  info = g_file_enumerator_next_file (erator, NULL, &error);
  while (info)
  {
    const gchar *name = g_file_info_get_name (info);
    GFileType type = g_file_info_get_file_type (info);
    GFile     *item = g_file_get_child (basedir, name);

    g_message (name);
    cb (db, item);

    g_object_unref (item);
    g_object_unref (info);
    info = g_file_enumerator_next_file (erator, NULL, &error);
  }
  g_object_unref (erator);
}

static void
index_email_cb (notmuch_database_t *db, GFile *mailfile)
{
  notmuch_status_t status;
  gchar *path = g_file_get_path (mailfile);

  g_message ("Indexing %s", g_file_get_path (mailfile));

  status = notmuch_database_add_message (db, path, NULL);

  if (status != NOTMUCH_STATUS_SUCCESS &&
      status != NOTMUCH_STATUS_DUPLICATE_MESSAGE_ID)
    g_warning ("Couldm not index %s: %d", path, status);

  g_free (path);
}

static void
index_container_folder_cb (notmuch_database_t *db, GFile *folder)
{
  folder_crawler (db, folder, index_email_cb);
}

static void
index_subfolders_cb (notmuch_database_t *db, GFile *dir)
{
  int i = 0;
  char *stdfolders[] = {"cur", "tmp", "new", "subfolders", NULL};

  while (stdfolders[i])
  {
    GFile *folder = g_file_get_child (dir, stdfolders[i]);
    if (g_file_query_exists (folder, NULL))
    {
      if (g_str_equal (stdfolders[i], "subfolders"))
      {
        folder_crawler (db, folder, index_subfolders_cb);
      }
      else
      {
        folder_crawler (db, folder, index_container_folder_cb);
      }
    }
    g_object_unref (folder);
    i++;
  }
}

static void
scan_directory (notmuch_database_t *db, GFile *db_folder)
{
  GFile           *folders;
  folders = g_file_get_child (db_folder, "folders");
  if (!g_file_query_exists (folders, NULL))
  {
    gchar *path = g_file_get_path (folders);
    g_error ("%s does not exists", path);
    g_free (path);
  }
  else
  {
    folder_crawler (db, folders, index_subfolders_cb);
  }

  g_object_unref (folders);
}


int main (int argc, char** argv) {
  GFile              *db_dir, *db_file;
  notmuch_status_t    status;
  notmuch_database_t *db   = NULL;
  GMainLoop          *loop = NULL;

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
    status = notmuch_database_create (argv[1], &db);
  else
    status = notmuch_database_open (argv[1], NOTMUCH_DATABASE_MODE_READ_WRITE, &db);

  if (status)
  {
    g_error ("Could not open database: %d", status);
    g_object_unref (db_dir);
    g_object_unref (db_file);
    notmuch_database_destroy (db);
    return 3;
  }


  scan_directory (db, db_dir);
  //loop = g_main_loop_new (NULL, FALSE);
  //g_main_loop_run (loop);

  notmuch_database_destroy (db);
  g_object_unref (db_file);
  g_object_unref (db_dir);
  return 0;
}
