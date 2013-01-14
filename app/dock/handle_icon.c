#include <gtk/gtk.h>
#include <string.h>
#include "dominant_color.h"
#include "xid2aid.h"
#include "handle_icon.h"
#include "xdg_misc.h"
#include "utils.h"

#define BOARD_PATH RESOURCE_DIR"/dock/img/board.png"
#define BOARD_MASK_PATH RESOURCE_DIR"/dock/img/mask.png"

cairo_surface_t* _board = NULL;
cairo_surface_t* _board_mask = NULL;

char* handle_icon(GdkPixbuf* icon)
{
    if (_board== NULL) {
        _board = cairo_image_surface_create_from_png(BOARD_PATH);
        _board_mask = cairo_image_surface_create_from_png(BOARD_MASK_PATH);
    }
    g_assert(_board_mask != NULL);

    double r, g, b;
    calc_dominant_color_by_pixbuf(icon, &r, &g, &b);

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, BOARD_WIDTH, BOARD_HEIGHT);
    cairo_t* cr  = cairo_create(surface);

    cairo_set_source_rgb(cr, r, g, b);
    cairo_mask_surface(cr, _board_mask, 0, 0);
    /*cairo_paint(cr);*/

    gdk_cairo_set_source_pixbuf(cr, icon, MARGIN_LEFT, MARGIN_TOP);
    cairo_paint(cr);

    cairo_set_source_surface(cr, _board, 0, 0);
    cairo_paint(cr);

    guchar* data = get_data_uri_by_surface(surface);

    cairo_surface_destroy(surface);
    cairo_destroy(cr);
    return data;
}

guchar* __data_base64 = NULL;
size_t __data_size = 0;
cairo_status_t write_func(void* store, unsigned char* data, unsigned int length)
{
    __data_size = length + __data_size;
    __data_base64 = g_renew(guchar, __data_base64, __data_size);
    memmove((void*)(__data_base64 + __data_size - length), (void*)data, (size_t)length);
    return CAIRO_STATUS_SUCCESS;
}

char* get_data_uri_by_surface(cairo_surface_t* surface)
{
    __data_base64 = NULL;
    __data_size = 0;
    cairo_surface_write_to_png_stream(surface, (cairo_write_func_t)write_func, NULL);
    guchar* base64 = g_base64_encode(__data_base64, __data_size);
    g_free(__data_base64);

    char* ret = g_strconcat("data:image/png;base64,", base64, NULL);
    g_free(base64);

    return ret;
}


gboolean is_deepin_icon(const char* path)
{
    return g_str_has_prefix(path, "/usr/share/icons/GoodIcons/");
}

char* try_get_deepin_icon(const char* _app_id)
{
    char* app_id = g_strdup(_app_id);
    to_lower_inplace(app_id);
    if (is_deepin_app_id(app_id)) {
        switch (get_deepin_app_id_operator(app_id)) {
            case ICON_OPERATOR_USE_ICONNAME:
                {
                    char* icon_name =  get_deepin_app_id_value(app_id);
                    char* icon_path = icon_name_to_path(icon_name, 48);
                    g_free(icon_name);
                    g_free(app_id);
                    return icon_path;
                }
            case ICON_OPERATOR_USE_RUNTIME:
                g_free(app_id);
                return NULL;
            case ICON_OPERATOR_USE_PATH:
                g_free(app_id);
                g_warning("Hasn't support set path Icon Handler\n");
                break;
            case ICON_OPERATOR_SET_DOMINANTCOLOR:
                g_free(app_id);
                g_warning("Hasn't support set dominantcolor Icon Handler\n");
                break;
            default:
                g_warning("Hasn't support unknow Icon Handler\n");

        }
    } else {
        g_free(app_id);
        return NULL;
    }
}