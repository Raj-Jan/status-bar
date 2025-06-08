#include "draw-core.h"
#include "common/draw.h"
#include "layout/layout-core.h"
#include "data/clock.h"
#include "data/states.h"
#include "data/hyprland.h"
#include "data/player.h"
#include "data/system.h"
#include "data/tray.h"
#include "icon/blender.h"
#include "icon/discord.h"
#include "icon/firefox.h"
#include "icon/krita.h"
#include "icon/osu.h"
#include "icon/spotify.h"
#include "icon/steam.h"
#include "icon/vscode.h"

static cairo_surface_t* icon[8];
static cairo_surface_t* image;

static void draw_button(double x)
{
	const char str[][4] = { "", "", "", "", "" };
	const int color[] = {
		COLOR_FLAMINGO,
		COLOR_RED,
		COLOR_GREEN,
		COLOR_MAUVE,
		COLOR_SAPPHIRE };
	const int i = data_states.button_option;

	draw_square(x, BUTTON_INNER_Y, BUTTON_INNER_W, BUTTON_INNER_H, 0);
	draw_fill(COLOR_SURFACE0, 1.0);

	draw_round(x, BUTTON_INNER_Y, BUTTON_INNER_W, BUTTON_INNER_H, 
		0, BUTTON_RADIUS);
	draw_fill(color[i], 1.0);

	text_config(FONT_AWESOME, BUTTON_FONT, PANGO_WEIGHT_NORMAL);
	text_center(x, BUTTON_INNER_Y, BUTTON_INNER_W, BUTTON_INNER_H, str[i]);
	text_draw(COLOR_SURFACE0, 1.0);
}
static void draw_app(double x, int j)
{
	const double y = APP_INNER_Y;

	draw_square(x, y, APP_INNER_W, APP_INNER_H, 0);
	draw_fill(COLOR_SURFACE0, 1.0);

	const int color = (data_tray.window_count[j] == 0)
		? COLOR_OVERLAY0 : COLOR_FLAMINGO;

	cairo_set_source_rgba(
		cr, colors[color + 0], colors[color + 1], colors[color + 2], 1.0);
	cairo_mask_surface(cr, icon[j], x, y);
}
static void draw_workspace(double x, int j)
{
	const int b1 = (data_hyprland.workspace_active == j);
	const int b2 = (data_hyprland.workspace_count[j] == 0);

	const double y = (WINDOW_HEIGHT - WORKSPACE_HEIGHT) / 2.0;

	draw_square(x, y, WORKSPACE_WIDTH, WORKSPACE_HEIGHT, 0);
	draw_fill(COLOR_SURFACE0, 1.0);

	draw_skew(
		x, y, WORKSPACE_WIDTH, WORKSPACE_HEIGHT, WORKSPACE_STROKE, WORKSPACE_RADIUS);

	if (b1) draw_fill_preserve(COLOR_PEACH, 1.0);

	const int color = (b1 || !b2) ? COLOR_FLAMINGO : COLOR_SURFACE2;
	draw_stroke(color, 1.0, WORKSPACE_STROKE);
}
static void draw_time(double x)
{
	const char text_time[6] = {
		'0' + data_clock.time_hour / 10,
		'0' + data_clock.time_hour % 10,
		':',
		'0' + data_clock.time_minute / 10,
		'0' + data_clock.time_minute % 10,
		'\0' };

	const double y = CLOCK_TIME_OFFSET;

	draw_square(x, y, CLOCK_TIME_LENGTH + 1, CLOCK_TIME_SIZE, 0);
	draw_fill(COLOR_SURFACE0, 1.0);

	text_config(FONT_AWESOME, CLOCK_TIME_SIZE, PANGO_WEIGHT_SEMIBOLD);
	text_right(x, y, CLOCK_TIME_LENGTH, CLOCK_TIME_SIZE, text_time);
	text_draw(COLOR_PEACH, 1.0);
}
static void draw_date(double x)
{
	const char days[][8] = {
		"     Sun", 
		"     Mon", 
		"    Tues", 
		"  Wednes", 
		"   Thurs", 
		"     Fri", 
		"   Satur"
	};
	
	char text_date[] = "________day, XX/XX";
	for (int i = 0; i < sizeof(*days); i++)
	{
		text_date[i] = days[data_clock.date_weekday][i];
	}

	text_date[sizeof(text_date) - 6] = '0' + data_clock.date_day / 10;
	text_date[sizeof(text_date) - 5] = '0' + data_clock.date_day % 10;
	text_date[sizeof(text_date) - 3] = '0' + (data_clock.date_month + 1) / 10;
	text_date[sizeof(text_date) - 2] = '0' + (data_clock.date_month + 1) % 10;

	const double y = WINDOW_HEIGHT - CLOCK_TIME_OFFSET - CLOCK_DATE_SIZE;

	draw_square(x, y, CLOCK_DATE_LENGTH + 1, CLOCK_DATE_SIZE + 2, 0);
	draw_fill(COLOR_SURFACE0, 1.0);

	text_config(FONT_SANS, CLOCK_DATE_SIZE, PANGO_WEIGHT_MEDIUM);
	text_right(x, y, CLOCK_DATE_LENGTH, CLOCK_DATE_SIZE, text_date);
	text_draw(COLOR_PEACH, 1.0);
}
static void draw_meter_icon(double x, int j, const char* str)
{
	text_config(FONT_AWESOME, METER_ICON_SIZE, PANGO_WEIGHT_NORMAL);
	text_center(x, 0, METER_ICON_SIZE, WINDOW_HEIGHT, str);
	text_draw(j, 1.0);
}
static void draw_meter_fill(double x, double ratio)
{
	double y = (WINDOW_HEIGHT - METER_FILL_SIZE) / 2;

	draw_square(x, y, METER_FILL_SIZE, METER_FILL_SIZE, 0);
	draw_fill(COLOR_SURFACE0, 1.0);

	const double r = (METER_FILL_SIZE - METER_FILL_STROKE) / 2;

	cairo_arc(cr, x + METER_FILL_SIZE / 2, y + METER_FILL_SIZE / 2 , r, 0, TWO_PI);
	draw_stroke(COLOR_SURFACE2, 1.0, METER_FILL_STROKE);

	if (ratio > 0)
	{
		const double start = TWO_PI * (-0.25);
		const double end = TWO_PI * (-0.25 + ratio);

		cairo_arc(cr, x + METER_FILL_SIZE / 2, y + METER_FILL_SIZE / 2, r, start, end);
		draw_stroke(COLOR_PEACH, 1.0, METER_FILL_STROKE);
	}

	char str[4];

	const int val = (int)(ratio * 100);
	if (val < 10)
	{
		str[0] = '0' + (val % 10);
		str[1] = '\0';
	}
	else if (val < 100)
	{
		str[0] = '0' + (val / 10);
		str[1] = '0' + (val % 10);
		str[2] = '\0';
	}

	text_config(FONT_AWESOME, METER_FILL_FONT, PANGO_WEIGHT_ULTRAHEAVY);
	text_center(x, y, METER_FILL_SIZE, METER_FILL_SIZE, str);
	text_draw(COLOR_PEACH, 1.0);
}
static void draw_meter_text(double x, int j, double w, const char* str)
{
	const double y = (WINDOW_HEIGHT - METER_TEXT_SIZE) / 2;

	draw_square(x, y, w + 1, METER_TEXT_SIZE, 0);
	draw_fill(COLOR_SURFACE0, 1.0);

	text_config(FONT_AWESOME, METER_TEXT_SIZE, PANGO_WEIGHT_ULTRAHEAVY);
	text_left(x, 0, w, WINDOW_HEIGHT, str);
	text_draw(j, 1.0);
}
static void draw_meter_text1(double x, int j, uint32_t value)
{
	char str[] = "XX °C";
	str[0] = '0' + (value / 10);
	str[1] = '0' + (value % 10);
	draw_meter_text(x, j, METER_TEXT_THIN_LENGTH, str);
}
static void draw_meter_text2(double x, int j, double value)
{
	char str[] = "XXXXX KB";
	{
		char unit = 'K';
		if (value >= 1000)
		{
			value /= 1024;
			unit = 'M';
		}
		if (value >= 1000)
		{
			value /= 1024;
			unit = 'G';
		}
		if (value >= 1000)
		{
			value /= 1024;
			unit = 'T';
		}
		str[6] = unit;
	}

	if (value < 10)
	{
		str[0] = '0' + (int)(value / 1) % 10;
		str[1] = '.';
		str[2] = '0' + (int)(value * 10) % 10;
		str[3] = '0' + (int)(value * 100) % 10;
		str[4] = '0' + (int)(value * 1000) % 10;
	}
	else if (value < 100)
	{
		str[0] = '0' + (int)(value / 10) % 10;
		str[1] = '0' + (int)(value / 1) % 10;
		str[2] = '.';
		str[3] = '0' + (int)(value * 10) % 10;
		str[4] = '0' + (int)(value * 100) % 10;
	}
	else
	{
		str[0] = '0' + (int)(value / 100) % 10;
		str[1] = '0' + (int)(value / 10) % 10;
		str[2] = '0' + (int)(value / 1) % 10;
		str[3] = '.';
		str[4] = '0' + (int)(value * 10) % 10;
	}

	draw_meter_text(x, j, METER_TEXT_WIDE_LENGTH, str);
}

static void draw_player_image(double x)
{
	double y = (WINDOW_HEIGHT - PLAYER_IMAGE_SIZE) / 2.0;

	draw_square(x, y, PLAYER_IMAGE_SIZE, PLAYER_IMAGE_SIZE, 0);
	draw_fill(COLOR_SURFACE0, 1.0);

	draw_round(x, y, PLAYER_IMAGE_SIZE, PLAYER_IMAGE_SIZE, 0, PLAYER_IMAGE_RADIUS);

	cairo_clip(cr);

	if (image)
	{
		int w = cairo_image_surface_get_width(image);
		int h = cairo_image_surface_get_height(image);
	
		double sx = PLAYER_IMAGE_SIZE / (double)w;
		double sy = PLAYER_IMAGE_SIZE / (double)h;

		cairo_scale(cr, sx, sy);
		cairo_set_source_surface(cr, image, x / sx, y / sy);
	}
	else
	{
		draw_color(COLOR_SURFACE2, 1.0);
	}

	cairo_paint(cr);
	cairo_identity_matrix(cr);

	cairo_reset_clip(cr);
}
static void draw_player_text(double x)
{
	draw_square(x, PLAYER_TEXT_Y, PLAYER_TEXT_W + 1, PLAYER_TEXT_SIZE + 3, 0);
	draw_fill(COLOR_SURFACE0, 1.0);

	const char* str = data_player.text;

	text_config(FONT_SANS, PLAYER_TEXT_SIZE, PANGO_WEIGHT_MEDIUM);
	text_shorten(PLAYER_TEXT_W);
	text_left(x, PLAYER_TEXT_Y, PLAYER_TEXT_W, PLAYER_TEXT_SIZE, str);
	text_draw(COLOR_PEACH, 1.0);
}
static void draw_player_bar(double x)
{
	draw_square(x, PLAYER_BAR_Y, PLAYER_BAR_W, PLAYER_BAR_H, 0);
	draw_fill(COLOR_SURFACE0, 1.0);

	double p = (data_player.duration > 0)
		? (double)data_player.current / data_player.duration
		: 0;
	
	draw_round(x, PLAYER_BAR_Y, PLAYER_BAR_W, PLAYER_BAR_H, 0, PLAYER_BAR_HEIGHT / 2.0);
	draw_fill(COLOR_SURFACE2, 1.0);

	double min = (double)PLAYER_BAR_H / PLAYER_BAR_W;

	if (p > 1) p = 1;
	if (p < 0) p = 0;

	p = (p + min) / (1 + min);

	draw_round(x, PLAYER_BAR_Y, p * PLAYER_BAR_W, PLAYER_BAR_H, 0, PLAYER_BAR_HEIGHT / 2.0);
	draw_fill(COLOR_PEACH, 1.0);
}
static void draw_player_button(double x, const char* str)
{
	double y = WINDOW_HEIGHT - PLAYER_BUTTON_OFFSET - PLAYER_BUTTON_H;
	
	draw_square(x, y, PLAYER_BUTTON_W, PLAYER_BUTTON_H, 0);
	draw_fill(COLOR_SURFACE0, 1.0);

	draw_round(x, y, PLAYER_BUTTON_W, PLAYER_BUTTON_H, 0, PLAYER_BUTTON_RADIUS);
	draw_fill(COLOR_FLAMINGO, 1.0);

	text_config(FONT_AWESOME, PLAYER_TAG_SIZE, PANGO_WEIGHT_NORMAL);
	text_center(x, y, PLAYER_BUTTON_W, PLAYER_BUTTON_H, str);
	text_draw(COLOR_SURFACE0, 1.0);
}
static void draw_player_playbutton(double x)
{
	draw_player_button(x, data_player.is_playing ? "" : "");
}

static long is_dirty(uint64_t index)
{
	if (data_states.dirty_core & (1ul << index))
	{
		data_states.dirty_core ^= (1ul << index);
		return 1;
	}
	return 0;
}

int draw_core_setup(void* data)
{
	surface = cairo_image_surface_create_for_data(
		data, CAIRO_FORMAT_ARGB32, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH * 4);
	cr = cairo_create(surface);

	desc[FONT_SANS] = pango_font_description_from_string("Sans");
	desc[FONT_AWESOME] = pango_font_description_from_string(
		"Font Awesome 6 Free Solid");
	font_layout = pango_cairo_create_layout(cr);

	icon[0] = cairo_image_surface_create_for_data((unsigned char*)icon_vscode_data, 
		CAIRO_FORMAT_ARGB32, APP_SIZE, APP_SIZE, APP_SIZE * 4);
	icon[1] = cairo_image_surface_create_for_data((unsigned char*)icon_firefox_data, 
		CAIRO_FORMAT_ARGB32, APP_SIZE, APP_SIZE, APP_SIZE * 4);
	icon[2] = cairo_image_surface_create_for_data((unsigned char*)icon_discord_data, 
		CAIRO_FORMAT_ARGB32, APP_SIZE, APP_SIZE, APP_SIZE * 4);
	icon[3] = cairo_image_surface_create_for_data((unsigned char*)icon_steam_data, 
		CAIRO_FORMAT_ARGB32, APP_SIZE, APP_SIZE, APP_SIZE * 4);
	icon[4] = cairo_image_surface_create_for_data((unsigned char*)icon_spotify_data, 
		CAIRO_FORMAT_ARGB32, APP_SIZE, APP_SIZE, APP_SIZE * 4);
	icon[5] = cairo_image_surface_create_for_data((unsigned char*)icon_blender_data, 
		CAIRO_FORMAT_ARGB32, APP_SIZE, APP_SIZE, APP_SIZE * 4);
	icon[6] = cairo_image_surface_create_for_data((unsigned char*)icon_krita_data, 
		CAIRO_FORMAT_ARGB32, APP_SIZE, APP_SIZE, APP_SIZE * 4);
	icon[7] = cairo_image_surface_create_for_data((unsigned char*)icon_osu_data, 
		CAIRO_FORMAT_ARGB32, APP_SIZE, APP_SIZE, APP_SIZE * 4);

	// icon[0] = cairo_image_surface_create_for_data((unsigned char*)icon_vscode_data, 
	// 	CAIRO_FORMAT_A8, APP_SIZE, APP_SIZE, APP_SIZE);
	// icon[1] = cairo_image_surface_create_for_data((unsigned char*)icon_firefox_data, 
	// 	CAIRO_FORMAT_A8, APP_SIZE, APP_SIZE, APP_SIZE);
	// icon[2] = cairo_image_surface_create_for_data((unsigned char*)icon_discord_data, 
	// 	CAIRO_FORMAT_A8, APP_SIZE, APP_SIZE, APP_SIZE);
	// icon[3] = cairo_image_surface_create_for_data((unsigned char*)icon_steam_data, 
	// 	CAIRO_FORMAT_A8, APP_SIZE, APP_SIZE, APP_SIZE);
	// icon[4] = cairo_image_surface_create_for_data((unsigned char*)icon_spotify_data, 
	// 	CAIRO_FORMAT_A8, APP_SIZE, APP_SIZE, APP_SIZE);
	// icon[5] = cairo_image_surface_create_for_data((unsigned char*)icon_blender_data, 
	// 	CAIRO_FORMAT_A8, APP_SIZE, APP_SIZE, APP_SIZE);
	// icon[6] = cairo_image_surface_create_for_data((unsigned char*)icon_krita_data, 
	// 	CAIRO_FORMAT_A8, APP_SIZE, APP_SIZE, APP_SIZE);
	// icon[7] = cairo_image_surface_create_for_data((unsigned char*)icon_osu_data, 
	// 	CAIRO_FORMAT_A8, APP_SIZE, APP_SIZE, APP_SIZE);

	draw_round(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, WINDOW_RADIUS);
	draw_fill(COLOR_SURFACE0, 1.0);

	draw_meter_icon(METER0_POS + METER_ICON_X, COLOR_FLAMINGO, "");
	draw_meter_icon(METER1_POS + METER_ICON_X, COLOR_FLAMINGO, "");
	draw_meter_icon(METER2_POS + METER_ICON_X, COLOR_MAROON, "");
	draw_meter_icon(METER3_POS + METER_ICON_X, COLOR_ROSEWAWTER, "");
	draw_meter_icon(METER4_POS + METER_ICON_X, COLOR_ROSEWAWTER, "");

	draw_player_button(PLAYER_BUTTON_POS(0) + PLAYER_BUTTON_X, "");
	draw_player_button(PLAYER_BUTTON_POS(2) + PLAYER_BUTTON_X, "");

	cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);

	return draw_core_frame();
}
int draw_core_frame()
{
	if (is_dirty(ELEMENT_BUTTON_MENU)) draw_button(MENU_POS + BUTTON_INNER_X);

	if (is_dirty(ELEMENT_APP0)) draw_app(APP_POS(0), 0);
	if (is_dirty(ELEMENT_APP1)) draw_app(APP_POS(1), 1);
	if (is_dirty(ELEMENT_APP2)) draw_app(APP_POS(2), 2);
	if (is_dirty(ELEMENT_APP3)) draw_app(APP_POS(3), 3);
	if (is_dirty(ELEMENT_APP4)) draw_app(APP_POS(4), 4);
	if (is_dirty(ELEMENT_APP5)) draw_app(APP_POS(5), 5);
	if (is_dirty(ELEMENT_APP6)) draw_app(APP_POS(6), 6);
	if (is_dirty(ELEMENT_APP7)) draw_app(APP_POS(7), 7);

	if (is_dirty(ELEMENT_PLAYER_IMAGE)) draw_player_image(
		PLAYER_POS + PLAYER_IMAGE_X);
	if (is_dirty(ELEMENT_PLAYER_TEXT)) draw_player_text(
		PLAYER_POS + PLAYER_TEXT_X);
	if (is_dirty(ELEMENT_PLAYER_BUTTON)) draw_player_playbutton(
		PLAYER_BUTTON_POS(1) + PLAYER_BUTTON_X);
	if (is_dirty(ELEMENT_PLAYER_BAR)) draw_player_bar(
		PLAYER_POS + PLAYER_BAR_X);

	if (is_dirty(ELEMENT_WORKSPACE0)) draw_workspace(WORKSPACE_POS(0), 0);
	if (is_dirty(ELEMENT_WORKSPACE1)) draw_workspace(WORKSPACE_POS(1), 1);
	if (is_dirty(ELEMENT_WORKSPACE2)) draw_workspace(WORKSPACE_POS(2), 2);
	if (is_dirty(ELEMENT_WORKSPACE3)) draw_workspace(WORKSPACE_POS(3), 3);
	if (is_dirty(ELEMENT_WORKSPACE4)) draw_workspace(WORKSPACE_POS(4), 4);
	if (is_dirty(ELEMENT_WORKSPACE5)) draw_workspace(WORKSPACE_POS(5), 5);
	if (is_dirty(ELEMENT_WORKSPACE6)) draw_workspace(WORKSPACE_POS(6), 6);
	if (is_dirty(ELEMENT_WORKSPACE7)) draw_workspace(WORKSPACE_POS(7), 7);
	if (is_dirty(ELEMENT_WORKSPACE8)) draw_workspace(WORKSPACE_POS(8), 8);
	if (is_dirty(ELEMENT_WORKSPACE9)) draw_workspace(WORKSPACE_POS(9), 9);

	if (is_dirty(ELEMENT_CLOCK_TIME)) draw_time(CLOCK_POS + CLOCK_TIME_X);
	if (is_dirty(ELEMENT_CLOCK_DATE)) draw_date(CLOCK_POS + CLOCK_DATE_X);

	if (is_dirty(ELEMENT_METER0_FILL)) draw_meter_fill(
		METER0_POS + METER_FILL_X, data_system.usage_gpu);
	if (is_dirty(ELEMENT_METER1_FILL)) draw_meter_fill(
		METER1_POS + METER_FILL_X, data_system.usage_cpu);
	if (is_dirty(ELEMENT_METER2_FILL)) draw_meter_fill(
		METER2_POS + METER_FILL_X, data_system.used_ram);
	if (is_dirty(ELEMENT_METER3_FILL)) draw_meter_fill(
		METER3_POS + METER_FILL_X, data_system.free_root);
	if (is_dirty(ELEMENT_METER4_FILL)) draw_meter_fill(
		METER4_POS + METER_FILL_X, data_system.free_home);

	if (is_dirty(ELEMENT_METER0_TEXT)) draw_meter_text1(
		METER0_POS + METER_TEXT_X, COLOR_FLAMINGO, data_system.value_gpu);
	if (is_dirty(ELEMENT_METER1_TEXT)) draw_meter_text1(
		METER1_POS + METER_TEXT_X, COLOR_FLAMINGO, data_system.value_cpu);
	if (is_dirty(ELEMENT_METER2_TEXT)) draw_meter_text2(
		METER2_POS + METER_TEXT_X, COLOR_MAROON, data_system.value_ram);
	if (is_dirty(ELEMENT_METER3_TEXT)) draw_meter_text2(
		METER3_POS + METER_TEXT_X, COLOR_ROSEWAWTER, data_system.value_root);
	if (is_dirty(ELEMENT_METER4_TEXT)) draw_meter_text2(
		METER4_POS + METER_TEXT_X, COLOR_ROSEWAWTER, data_system.value_home);

    cairo_surface_flush(surface);

	return (data_states.frame_requested_core = (data_states.dirty_core > 0));
}

void draw_core_image(const char* name)
{
	if (image)
	{
		cairo_surface_destroy(image);
		image = 0;
	}
	image = cairo_image_surface_create_from_png(name);

	data_states.dirty_core |= (1ul << ELEMENT_PLAYER_IMAGE);
}
