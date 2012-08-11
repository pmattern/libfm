/*
 *      fm-cell-renderer-text.c
 *
 *      Copyright 2009 PCMan <pcman.tw@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

/**
 * SECTION:fm-cell-renderer-text
 * @short_description: An implementation of cell text renderer.
 * @title: FmCellRendererText
 *
 * @include: libfm/fm-cell-renderer-text.h
 *
 * The #FmCellRendererText can be used to render text in cell.
 */

#include "fm-cell-renderer-text.h"

G_DEFINE_TYPE(FmCellRendererText, fm_cell_renderer_text, GTK_TYPE_CELL_RENDERER_TEXT);

static void fm_cell_renderer_text_get_size(GtkCellRenderer            *cell,
                                           GtkWidget                  *widget,
#if GTK_CHECK_VERSION(3, 0, 0)
                                           const
#endif
                                           GdkRectangle               *rectangle,
                                           gint                       *x_offset,
                                           gint                       *y_offset,
                                           gint                       *width,
                                           gint                       *height);

#if GTK_CHECK_VERSION(3, 0, 0)
static void fm_cell_renderer_text_render(GtkCellRenderer *cell,
                                         cairo_t *cr,
                                         GtkWidget *widget,
                                         const GdkRectangle *background_area,
                                         const GdkRectangle *cell_area,
                                         GtkCellRendererState flags);
#else
static void fm_cell_renderer_text_render(GtkCellRenderer *cell, 
                                         GdkDrawable *window,
                                         GtkWidget *widget,
                                         GdkRectangle *background_area,
                                         GdkRectangle *cell_area,
                                         GdkRectangle *expose_area,
                                         GtkCellRendererState  flags);
#endif

static void fm_cell_renderer_text_class_init(FmCellRendererTextClass *klass)
{
    GtkCellRendererClass* render_class = GTK_CELL_RENDERER_CLASS(klass);
    render_class->render = fm_cell_renderer_text_render;
    render_class->get_size = fm_cell_renderer_text_get_size;
}


static void fm_cell_renderer_text_init(FmCellRendererText *self)
{
    
}

/**
 * fm_cell_renderer_text_new
 *
 * Creates new #GtkCellRenderer object for #FmCellRendererText.
 *
 * Returns: a new #GtkCellRenderer object.
 *
 * Since: 0.1.0
 */
GtkCellRenderer *fm_cell_renderer_text_new(void)
{
    return (GtkCellRenderer*)g_object_new(FM_CELL_RENDERER_TEXT_TYPE, NULL);
}

#if GTK_CHECK_VERSION(3, 0, 0)
static void fm_cell_renderer_text_render(GtkCellRenderer *cell,
                                         cairo_t *cr,
                                         GtkWidget *widget,
                                         const GdkRectangle *background_area,
                                         const GdkRectangle *cell_area,
                                         GtkCellRendererState flags)
#else
static void fm_cell_renderer_text_render(GtkCellRenderer *cell,
                                         GdkDrawable *window,
                                         GtkWidget *widget,
                                         GdkRectangle *background_area,
                                         GdkRectangle *cell_area,
                                         GdkRectangle *expose_area,
                                         GtkCellRendererState flags)
#endif
{
#if GTK_CHECK_VERSION(3, 0, 0)
    GtkStyleContext* style;
    GtkStateFlags state;
#else
    GtkStyle* style;
    GtkStateType state;
#endif
    gchar* text;
    gint text_width;
    gint text_height;
    gint x_offset;
    gint y_offset;
    gint x_align_offset;
    GdkRectangle rect;
    PangoWrapMode wrap_mode;
    gint wrap_width;
    gint wrap_height = -1;
    PangoAlignment alignment;
    gfloat xalign, yalign;
    gint xpad, ypad;

    /* FIXME: this is time-consuming since it invokes pango_layout.
     *        if we want to fix this, we must implement the whole cell
     *        renderer ourselves instead of derived from GtkCellRendererText. */
    PangoContext* context = gtk_widget_get_pango_context(widget);

    PangoLayout* layout = pango_layout_new(context);

    g_object_get(G_OBJECT(cell),
                 "wrap-mode" , &wrap_mode,
                 "wrap-width", &wrap_width,
                 "alignment" , &alignment,
                 "text", &text,
                 NULL);

    pango_layout_set_alignment(layout, alignment);

    /* Setup the wrapping. */
    if (wrap_width < 0)
    {
        pango_layout_set_width(layout, -1);
        pango_layout_set_wrap(layout, PANGO_WRAP_CHAR);
    }
    else
    {
        pango_layout_set_width(layout, wrap_width * PANGO_SCALE);
        pango_layout_set_wrap(layout, wrap_mode);
        /* FIXME: add custom ellipsize from object? */
        pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
        /* Setup max height to be not more than half of width */
        wrap_height = wrap_width * PANGO_SCALE / 2;
        /* FIXME: add custom height from object? */
        pango_layout_set_height(layout, wrap_height);
    }

    pango_layout_set_text(layout, text, -1);

    pango_layout_set_auto_dir(layout, TRUE);

    pango_layout_get_pixel_size(layout, &text_width, &text_height);

    gtk_cell_renderer_get_alignment(cell, &xalign, &yalign);
    gtk_cell_renderer_get_padding(cell, &xpad, &ypad);
    /* Calculate the real x and y offsets. */
    x_offset = ((gtk_widget_get_direction(widget) == GTK_TEXT_DIR_RTL) ? (1.0 - xalign) : xalign)
             * (cell_area->width - text_width - (2 * xpad));
    x_offset = MAX(x_offset, 0);

    y_offset = yalign * (cell_area->height - text_height - (2 * ypad));
    y_offset = MAX (y_offset, 0);

    if(flags & (GTK_CELL_RENDERER_SELECTED|GTK_CELL_RENDERER_FOCUSED))
    {
        rect.x = cell_area->x + x_offset;
        rect.y = cell_area->y + y_offset;
        rect.width = text_width + (2 * xpad);
        rect.height = text_height + (2 * ypad);
    }

#if GTK_CHECK_VERSION(3, 0, 0)
    style = gtk_widget_get_style_context(widget);
#else
    style = gtk_widget_get_style(widget);
#endif
    if(flags & GTK_CELL_RENDERER_SELECTED) /* item is selected */
    {
#if GTK_CHECK_VERSION(3, 0, 0)
        GdkRGBA clr;

        if(flags & GTK_CELL_RENDERER_INSENSITIVE) /* insensitive */
            state = GTK_STATE_FLAG_INSENSITIVE;
        else
            state = GTK_STATE_FLAG_SELECTED;

        gtk_style_context_get_background_color(style, state, &clr);
#else
        cairo_t *cr = gdk_cairo_create (window);
        GdkColor clr;

        if(flags & GTK_CELL_RENDERER_INSENSITIVE) /* insensitive */
            state = GTK_STATE_INSENSITIVE;
        else
            state = GTK_STATE_SELECTED;

        clr = style->bg[state];

        /* paint the background */
        if(expose_area)
        {
            gdk_cairo_rectangle(cr, expose_area);
            cairo_clip(cr);
        }
#endif
        gdk_cairo_rectangle(cr, &rect);

        cairo_set_source_rgb(cr, clr.red / 65535., clr.green / 65535., clr.blue / 65535.);
        cairo_fill (cr);

#if !GTK_CHECK_VERSION(3, 0, 0)
        cairo_destroy (cr);
#endif
    }
#if !GTK_CHECK_VERSION(3, 0, 0)
    else
        state = GTK_STATE_NORMAL;
#endif

    x_align_offset = (alignment == PANGO_ALIGN_CENTER) ? (wrap_width - text_width) / 2 : 0;

#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_render_layout(style, cr,
                      cell_area->x + x_offset + xpad - x_align_offset,
                      cell_area->y + y_offset + ypad, layout);
#else
    gtk_paint_layout(style, window, state, TRUE,
                     expose_area, widget, "cellrenderertext",
                     cell_area->x + x_offset + xpad - x_align_offset,
                     cell_area->y + y_offset + ypad, layout);
#endif

    g_object_unref(layout);

    if(G_UNLIKELY( flags & GTK_CELL_RENDERER_FOCUSED) ) /* focused */
    {
#if GTK_CHECK_VERSION(3, 0, 0)
        gtk_render_focus(style, cr, rect.x, rect.y, rect.width, rect.height);
#else
        gtk_paint_focus(style, window, state, background_area,
                        widget, "cellrenderertext", rect.x, rect.y,
                        rect.width, rect.height);
#endif
    }
}

static void fm_cell_renderer_text_get_size(GtkCellRenderer            *cell,
                                           GtkWidget                  *widget,
#if GTK_CHECK_VERSION(3, 0, 0)
                                           const
#endif
                                           GdkRectangle               *rectangle,
                                           gint                       *x_offset,
                                           gint                       *y_offset,
                                           gint                       *width,
                                           gint                       *height)
{
    gint wrap_width;

    g_object_get(G_OBJECT(cell),
                 "wrap-width", &wrap_width,
                 NULL);

    if (wrap_width <= 0)
    {
        GTK_CELL_RENDERER_CLASS(fm_cell_renderer_text_parent_class)->get_size(cell, widget, rectangle, x_offset, y_offset, width, height);
    }
    else
    {
        *width = wrap_width;
        /* FIXME: add custom height from object? */
        *height = wrap_width / 2;
    }
}

