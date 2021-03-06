/**
 * Copyright (c) 2011 ~ 2013 Deepin, Inc.
 *               2013 ~ 2013 Liqiang Lee
 *
 * Author:      Liqiang Lee <liliqiang@linuxdeepin.com>
 * Maintainer:  Liqiang Lee <liliqiang@linuxdeepin.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 **/

#include <string.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "background.h"
#include "jsextension.h"
#include "utils.h"


#define SCHEME_LEN 7 // "file://"
#define DEFAULT_BACKGROUND_IMAGE "/usr/share/backgrounds/default_background.jpg"


char* get_blur_path()
{
    GError* err = NULL;
    GDBusProxy* proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                      G_DBUS_PROXY_FLAGS_NONE,
                                                      NULL,
                                                      "com.deepin.dde.daemon.Launcher",
                                                      "/com/deepin/dde/daemon/Launcher",
                                                      "com.deepin.dde.daemon.Launcher",
                                                      NULL,
                                                      &err
                                                      );

    if (err != NULL) {
        g_warning("[%s] get dbus proxy failed: %s", __func__, err->message);
        g_error_free(err);
        return NULL;
    }

    GSettings* settings = g_settings_new("com.deepin.dde.personalization");
    char* pic = g_settings_get_string(settings, "current-picture");
    GVariant* res = g_dbus_proxy_call_sync(proxy,
                                           "GetBackgroundPict",
                                           NULL,
                                            G_DBUS_CALL_FLAGS_NONE,
                                            -1,  // timeout
                                            NULL,  // cancellable
                                            &err
                                           );
    g_free(pic);
    g_object_unref(proxy);

    if (err != NULL) {
        g_warning("[%s:%s] %s", __FILE__, __func__, err->message);
        g_error_free(err);
    }

    if (res == NULL) {
        return NULL;
    }

    char* blur_path = NULL;
    g_variant_get(res, "(s)", &blur_path);
    g_variant_unref(res);
    return blur_path;
}


PRIVATE
gboolean _set_background_aux(GdkWindow* win, const char* bg_path, double width,
                             double height)
{
    GError* error = NULL;
    GdkPixbuf* _background_image = gdk_pixbuf_new_from_file_at_scale(bg_path,
                                                                     width,
                                                                     height,
                                                                     FALSE,
                                                                     &error);

    if (_background_image == NULL) {
        g_debug("[%s] %s\n", __func__, error->message);
        g_error_free(error);
        return FALSE;
    }

    cairo_surface_t* img_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                              width,
                                                              height);


    if (cairo_surface_status(img_surface) != CAIRO_STATUS_SUCCESS) {
        g_warning("[%s] create cairo surface fail!\n", __func__);
        g_object_unref(_background_image);
        return FALSE;
    }

    cairo_t* cr = cairo_create(img_surface);

    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
        g_warning("[%s] create cairo fail!\n", __func__);
        g_object_unref(_background_image);
        cairo_surface_destroy(img_surface);
        return FALSE;
    }

    gdk_cairo_set_source_pixbuf(cr, _background_image, 0, 0);
    cairo_paint(cr);
    g_object_unref(_background_image);

    cairo_pattern_t* pt = cairo_pattern_create_for_surface(img_surface);

    if (cairo_pattern_status(pt) == CAIRO_STATUS_NO_MEMORY) {
        g_warning("[%s] create cairo pattern fail!\n", __func__);
        cairo_surface_destroy(img_surface);
        cairo_destroy(cr);
        return FALSE;
    }

    gdk_window_hide(win);
    gdk_window_set_background_pattern(win, pt);
    gdk_window_show(win);

    cairo_pattern_destroy(pt);
    cairo_surface_destroy(img_surface);
    cairo_destroy(cr);

    return TRUE;
}


void set_background(GdkWindow* win,  double width, double height)
{
    char* blur_path = get_blur_path();
    if (blur_path == NULL) {
        g_debug("[%s] get blur pic failed, use default background: %s", __func__, DEFAULT_BACKGROUND_IMAGE);
        _set_background_aux(win, DEFAULT_BACKGROUND_IMAGE, width, height);
        return;
    }

    g_debug("[%s] blur pic path: %s", __func__, blur_path);

    _set_background_aux(win, &blur_path[SCHEME_LEN], width, height);

    g_free(blur_path);
}

