#include "input-core.h"
#include "common/utils.h"
#include "data/hyprland.h"
#include "data/states.h"
#include "data/player.h"
#include "data/pointer.h"
#include "data/tray.h"
#include "layout/layout-core.h"
#include "service-dbus.h"
#include "service-hyprland.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MENU_STATE_SHUTDOWN 1
#define MENU_STATE_REBOOT 2
#define MENU_STATE_HIBERNATE 3
#define MENU_STATE_FIRMWARE 4

static void launch_application(const char *app_path)
{
    pid_t pid1 = fork();
    
	if (pid1 < 0) return;
	if (pid1 > 0) return (void)waitpid(pid1, 0, 0);

    pid_t pid2 = fork();
    
    if (pid2 < 0) exit(EXIT_FAILURE);
    if (pid2 > 0) exit(EXIT_SUCCESS);
    
    setsid();
    execl(app_path, app_path, (char*)0);
    exit(EXIT_FAILURE);
}

static void handle_button_menu_motion(double x, double y)
{
	const int32_t dx = x - data_states.button_x;
	const int32_t dy = y - data_states.button_y;

	const int32_t zone = 14;

	if (-zone < dx && dx < zone && -zone < dy && dy < zone)
	{
		if (data_states.button_option)
			data_states.dirty_core |= (1ul << ELEMENT_BUTTON_MENU);

		data_states.button_option = 0;
		return;
	}

	uint32_t option = 0;
	if (dx <= 0 && dy <= 0)
	{
		option = (dx < dy) 
			? MENU_STATE_REBOOT 	// left
			: MENU_STATE_FIRMWARE;	// up
	}
	if (dx <= 0 && dy >= 0)
	{
		option = (dx < -dy) 
			? MENU_STATE_REBOOT 	// left
			: MENU_STATE_HIBERNATE;	// down
	}
	if (dx >= 0 && dy <= 0)
	{
		option = (-dx < dy) 
			? MENU_STATE_SHUTDOWN 	// right
			: MENU_STATE_FIRMWARE;	// up
	}
	if (dx >= 0 && dy >= 0)
	{
		option = (-dx < -dy) 
			? MENU_STATE_SHUTDOWN 	// right
			: MENU_STATE_HIBERNATE;	// down
	}

	if (data_states.button_option != option)
	{
		data_states.button_option = option;
		data_states.dirty_core |= (1ul << ELEMENT_BUTTON_MENU);
	}
}
static void handle_button_menu_press(double x, double y)
{
	data_states.button_x = x;
	data_states.button_y = y;
	data_states.button_option = 0;
	data_states.button_pressed = 1;
}
static void handle_button_menu_release(double x, double y)
{
	data_states.button_pressed = 0;
	if (!is_inside(x, y, BUTTON_INNER_X + MENU_POS, 
		BUTTON_OUTER_Y, BUTTON_INNER_W, BUTTON_OUTER_H)) return;

	const uint32_t i = data_states.button_option;
	data_states.button_option = 0;

	switch (i)
	{
		case MENU_STATE_SHUTDOWN: return service_notify_dbus_shutdown();
		case MENU_STATE_REBOOT: return service_notify_dbus_reboot();
		case MENU_STATE_HIBERNATE: return service_notify_dbus_hibernate();
		case MENU_STATE_FIRMWARE: return service_notify_dbus_firmware();
	}
}
static void handle_button_apps(double x, double y)
{
	const int i = (x - APPS_POS - APP_INNER_X) / APP_OUTER_W;
	const int j = (x - APPS_POS + APP_INNER_X) / APP_OUTER_W;

	if (i != j) return;

	int workspace = -1;
	for (int k = 0; k < data_hyprland.window_total; k++)
	{
		if (data_hyprland.window_params[k].app == i)
		{
			workspace = data_hyprland.window_params[k].workspace;
			break;
		}
	}

	if (workspace < 0)
	{
		const char* apps[] =
		{
			"/usr/bin/code",
			"/usr/bin/firefox",
			"/usr/bin/vesktop",
			"/usr/bin/steam",
			"/usr/bin/spotify-launcher",
			"/usr/bin/blender",
			"/usr/bin/krita",
			"/usr/bin/osu"
		};

		launch_application(apps[i]);
	}
	else
	{
		service_notify_hyprland_moveto(workspace + 1);
	}
}
static void handle_button_player(double x, double y)
{
	const double offset = PLAYER_BUTTON_X + PLAYER_BUTTON_POS(0);
	const double stride = (PLAYER_BUTTON_W + PLAYER_MARGIN_INNER);
	const int i = (x - offset) / stride;
	const int j = (x - offset + PLAYER_MARGIN_INNER) / stride;

	if (i != j) return;

	switch (i)
	{
		case 0: return (void)service_notify_dbus_prev();
		case 1: return (void)service_notify_dbus_swap();
		case 2: return (void)service_notify_dbus_next();
	}
}
static void handle_button_workspace(double x, double y)
{
	const int i = (x - WORKSPACES_POS - WORKSPACE_INNER_X) / WORKSPACE_OUTER_W;
	const int j = (x - WORKSPACES_POS + WORKSPACE_INNER_X) / WORKSPACE_OUTER_W;

	if (i != j) return;

	service_notify_hyprland_moveto(i + 1);
}

static void handle_press()
{
	const double x = data_pointer.x;
	const double y = data_pointer.y;

	if (is_inside(x, y, BUTTON_INNER_X + MENU_POS, 
		BUTTON_OUTER_Y, BUTTON_INNER_W, BUTTON_OUTER_H))
		handle_button_menu_press(x, y);

	if (is_inside(x, y, APPS_X + APPS_POS, 
		APPS_Y, APPS_W, APPS_H))
		handle_button_apps(x, y);

	if (is_inside(x, y, PLAYER_BUTTONS_X + PLAYER_POS,
		PLAYER_BUTTONS_Y, PLAYER_BUTTONS_W, PLAYER_BUTTONS_H))
		handle_button_player(x, y);

	if (is_inside(x, y, WORKSPACE_ALL_X + WORKSPACES_POS,
		WORKSPACE_ALL_Y, WORKSPACE_ALL_W, WORKSPACE_ALL_H))
		handle_button_workspace(x, y);
}
static void handle_release()
{
	const double x = data_pointer.x;
	const double y = data_pointer.y;

	if (data_states.button_pressed)
	{
		handle_button_menu_release(x, y);
	}
}

void input_core_motion(double x, double y)
{
	data_pointer.x = x;
	data_pointer.y = y;

	if (data_states.button_pressed)
		handle_button_menu_motion(x, y);
}
void input_core_button(int button, int state)
{
	if (button != 272) return;

	return state ? handle_press() : handle_release();
}
void input_core_axis(int axis, int value)
{
	const double x = data_pointer.x;
	const double y = data_pointer.y;
}
void input_core_leave()
{
	data_states.button_option = 0;
	data_states.button_pressed = 0;

	data_states.dirty_core |= (1ul << ELEMENT_BUTTON_MENU);
}
