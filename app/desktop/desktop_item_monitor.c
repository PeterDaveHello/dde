#include "dwebview.h"
#include "xdg_misc.h"
#include <gio/gio.h>
#include <glib/gprintf.h>

void cancel_monitor_dir(const char* path);
void monitor_dir(const char* path);
void begin_monitor_dir(const char* path, GCallback cb);
void end_monitor_dir(const char* path);

GHashTable *monitor_table = NULL;

void monitor_desktop_dir_cb(GFileMonitor *m, 
        GFile *file, GFile *other, GFileMonitorEvent t, 
        gpointer user_data)
{
    switch (t) {
        case G_FILE_MONITOR_EVENT_MOVED:
            {
                char* old_path = g_file_get_path(file);
                char* new_path = g_file_get_path(other);
                char* info = get_entry_info(new_path);

                char* tmp = g_strdup_printf("{\"old_id\":\"%s\", \"info\":%s}", old_path, info);
                js_post_message("item_rename", tmp);
                g_free(tmp);

                g_free(info);
                g_free(old_path);
                g_free(new_path);
                break;
            }
        case G_FILE_MONITOR_EVENT_DELETED:
            {
                char* path = g_file_get_path(file);

                g_hash_table_remove(monitor_table, path); //if the path is not an monitored dir, the remove operation will no effect.

                if (g_strcmp0(path, get_desktop_dir(FALSE)) == 0) {
                    cancel_monitor_dir(path);
                    char* desktop = get_desktop_dir(TRUE);
                    begin_monitor_dir(desktop, G_CALLBACK(monitor_desktop_dir_cb));
                    g_free(desktop);
                } else {
                    char* tmp = g_strdup_printf("\"%s\"", path);
                    js_post_message("item_delete", tmp);
                    g_free(tmp);
                }
                g_free(path);
                break;
            }
        case G_FILE_MONITOR_EVENT_CREATED:
        case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
            {
                char* path = g_file_get_path(file);
                if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
                    monitor_dir(path);
                }

                char* info = get_entry_info(path);
                if (info != NULL) {
                    js_post_message("item_update", info);
                    g_free(info);
                }

                g_free(path);
                break;
            }
    }
}

void monitor_dir_cb(GFileMonitor *m, 
        GFile *file, GFile *other, GFileMonitorEvent t, 
        gpointer path)
{
    switch (t) {
        case G_FILE_MONITOR_EVENT_MOVED:
            {
                char* new_path = g_file_get_path(other);
                if (g_strcmp0(new_path, get_desktop_dir(FALSE)) == 0) {
                    char* info = get_entry_info(new_path);
                    if (info != NULL) {
                        js_post_message("item_update", info);
                        g_free(info);
                    }
                    break;
                }
                g_free(new_path);
            }
        case G_FILE_MONITOR_EVENT_DELETED:
            {
                char* _path = g_file_get_path(file);
                if (g_strcmp0(_path, path) == 0) {
                    g_free(path);
                    cancel_monitor_dir(_path);
                } else {
                    char* info = get_entry_info(path);
                    if (info != NULL) {
                        js_post_message("item_update", info);
                        g_free(info);
                    }
                }
                g_free(_path);
                break;
            }
        case G_FILE_MONITOR_EVENT_CREATED:
        case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
            {
                char* info = get_entry_info(path);
                if (info != NULL) {
                    js_post_message("item_update", info);
                    g_free(info);
                }
                break;

                /*char* tmp = g_strdup_printf("{\"id\":\"%s\"}", (char*)path);*/
                /*js_post_message("dir_changed", tmp);*/
                /*break;*/
            }
    }
}



void begin_monitor_dir(const char* path, GCallback cb)
{
    if (!g_hash_table_contains(monitor_table, path)) {
        GFile* dir = g_file_new_for_path(path);
        GFileMonitor* monitor = g_file_monitor_directory(dir, G_FILE_MONITOR_NONE, NULL, NULL);
        char* key = g_strdup(path);
        g_hash_table_insert(monitor_table, key, monitor);
        g_signal_connect(monitor, "changed", cb, key);
    } else {
        g_warning("The %s has aleardy monitored! You many forget call the function of end_monitor_dir", path);
    }
}

void end_monitor_dir(const char* path)
{
    g_hash_table_remove(monitor_table, path);
}


void monitor_dir(const char* path)
{
    begin_monitor_dir(path, G_CALLBACK(monitor_dir_cb));
}
void cancel_monitor_dir(const char* path)
{
    end_monitor_dir(path);
}


//JS_EXPORT
void install_monitor()
{
    if (monitor_table == NULL) {
        monitor_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_object_unref);
        char* desktop = get_desktop_dir(TRUE);
        begin_monitor_dir(desktop, G_CALLBACK(monitor_desktop_dir_cb));


        GDir *dir =  g_dir_open(desktop, 0, NULL);
        if (dir != NULL) {
            const char* filename = NULL;
            char path[500];
            while ((filename = g_dir_read_name(dir)) != NULL) {
                g_sprintf(path, "%s/%s", desktop, filename);
                if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
                    monitor_dir(path);
                }
            }
            g_dir_close(dir);
        }
        g_free(desktop);
    }
}
