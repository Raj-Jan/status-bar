#include "color.h"

#include <cairo/cairo.h>
#include <pango/pangocairo.h>

#define HALF_PI 1.57079632679
#define TWO_PI 6.28318530718

enum font_t { FONT_SANS, FONT_AWESOME };

static cairo_surface_t* surface;
static cairo_t* cr;

extern PangoFontDescription* desc[2];
static PangoLayout* font_layout;

static void draw_color(int color, double opacity)
{
	cairo_set_source_rgba(
		cr, colors[color + 0], colors[color + 1], colors[color + 2], opacity);
}

static void draw_fill(int color, double opacity)
{
	draw_color(color, opacity);
	cairo_fill(cr);
}
static void draw_fill_preserve(int color, double opacity)
{
	draw_color(color, opacity);
	cairo_fill_preserve(cr);
}
static void draw_stroke(int color, double opacity, double s)
{
	draw_color(color, opacity);
	cairo_set_line_width(cr, s);
	cairo_stroke(cr);
}

static void draw_square(double x, double y, double w, double h, double s)
{
	cairo_rectangle(cr, x + s / 2, y + s / 2, w - s, h - s);
}
static void draw_round(double x, double y, double w, double h, double s, double r)
{
	const double x_min = x + s / 2 + r;
	const double y_min = y + s / 2 + r;

	const double x_max = x - s / 2 + w - r;
	const double y_max = y - s / 2 + h - r;

	cairo_new_sub_path(cr);
	cairo_arc(cr, x_max, y_max, r, 0 * HALF_PI, 1 * HALF_PI);
	cairo_arc(cr, x_min, y_max, r, 1 * HALF_PI, 2 * HALF_PI);
	cairo_arc(cr, x_min, y_min, r, 2 * HALF_PI, 3 * HALF_PI);
	cairo_arc(cr, x_max, y_min, r, 3 * HALF_PI, 4 * HALF_PI);
	cairo_close_path(cr);
}
static void draw_skew(double x, double y, double w, double h, double s, double r)
{
	const double x_min = x + s / 2;
	const double y_min = y + s / 2;

	const double x_max = x - s / 2 + w;
	const double y_max = y - s / 2 + h;
	
	const double r1 = r;
	const double r2 = h - r - 1;

	cairo_new_sub_path(cr);
	cairo_arc(cr, x_max - r2, y_max - r2, r2, 0 * HALF_PI, 1 * HALF_PI);
	cairo_arc(cr, x_min + r1, y_max - r1, r1, 1 * HALF_PI, 2 * HALF_PI);
	cairo_arc(cr, x_min + r2, y_min + r2, r2, 2 * HALF_PI, 3 * HALF_PI);
	cairo_arc(cr, x_max - r1, y_min + r1, r1, 3 * HALF_PI, 4 * HALF_PI);
	cairo_close_path(cr);
}

static void draw_circle(double x, double y, double r)
{
	cairo_arc(cr, x, y, r, 0 * HALF_PI, 4 * HALF_PI);
}

static void text_draw(int color, double opacity)
{
	draw_color(color, opacity);
	pango_cairo_show_layout(cr, font_layout);
	cairo_new_path(cr);
}
static void text_config(int i, double size, PangoWeight weight)
{
	pango_font_description_set_absolute_size(desc[i], size * PANGO_SCALE);
	pango_font_description_set_weight(desc[i], weight);
	pango_layout_set_font_description(font_layout, desc[i]);
}
static void text_shorten(double w)
{
	pango_layout_set_ellipsize(font_layout, PANGO_ELLIPSIZE_END);
	pango_layout_set_width(font_layout, w * PANGO_SCALE);
}
static void text_center(double x, double y, double w, double h, const char* str)
{
	int _w, _h;
	pango_layout_set_text(font_layout, str, -1);
	pango_layout_get_size(font_layout, &_w, &_h);

	x += (w - (double)_w / PANGO_SCALE) / 2;
	y += (h - (double)_h / PANGO_SCALE) / 2;

	cairo_move_to(cr, x, y);
}
static void text_right(double x, double y, double w, double h, const char* str)
{
	int _w, _h;
	pango_layout_set_text(font_layout, str, -1);
	pango_layout_get_size(font_layout, &_w, &_h);

	x += (w - (double)_w / PANGO_SCALE) / 1.0;
	y += (h - (double)_h / PANGO_SCALE) / 2.0;

	cairo_move_to(cr, x, y);
}
static void text_left(double x, double y, double w, double h, const char* str)
{
	int _w, _h;
	pango_layout_set_text(font_layout, str, -1);
	pango_layout_get_size(font_layout, &_w, &_h);

	y += (h - (double)_h / PANGO_SCALE) / 2.0;

	cairo_move_to(cr, x, y);
}
