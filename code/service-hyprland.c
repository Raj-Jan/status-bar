#include "service-hyprland.h"
#include "wayland/wayland-core.h"
#include "data/states.h"
#include "data/hyprland.h"
#include "data/tray.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define WORKSPACE_HIDE_CORE 0b1000000000u

static long string_cmp(const char* str, const char* exp)
{
	while (*str++ == *exp++)
	{
		if (*exp == 0) return 1;
		if (*str == 0) return 0;
	}
	return 0;
}
static long string_hex(const char* ptr, const char* end)
{
	long addr = 0;
	while (1)
	{
		if (ptr >= end) break;
		else if (*ptr >= '0' && *ptr <= '9') addr = 16 * addr + (*ptr++ - '0') + 0;
		else if (*ptr >= 'a' && *ptr <= 'f') addr = 16 * addr + (*ptr++ - 'a') + 10;
		else if (*ptr >= 'A' && *ptr <= 'F') addr = 16 * addr + (*ptr++ - 'A') + 10;
		else break;
	}
	return addr;
}
static long string_dec(const char* ptr, const char* end)
{
	long index = 0;
	while (ptr < end && *ptr >= '0' && *ptr <= '9')
	{
		index = 10 * index + (*ptr++ - '0');
	}
	return index - 1;
}

static long string_application(const char* name)
{
	if (string_cmp(name, "code-oss")) return 0;
	if (string_cmp(name, "firefox")) return 1;
	if (string_cmp(name, "vesktop")) return 2;
	if (string_cmp(name, "steam")) return 3;
	if (string_cmp(name, "spotify")) return 4;
	if (string_cmp(name, "blender")) return 5;
	if (string_cmp(name, "krita")) return 6;
	if (string_cmp(name, "osu!")) return 7;

	return 8;
}

static void app_increment(int i)
{
	if (i < 8 && data_tray.window_count[i]++ == 0)
	{
		data_states.dirty_core |= (1ul << (ELEMENT_APP0 + i));
	}
}
static void app_decrement(int i)
{
	if (i < 8 && --data_tray.window_count[i] == 0)
	{
		data_states.dirty_core |= (1ul << (ELEMENT_APP0 + i));
	}
}

static void window_open(uint64_t addr, uint8_t app, uint8_t index)
{
	const uint8_t i = data_hyprland.window_total++;

	data_hyprland.window_address[i] = addr;
	data_hyprland.window_params[i].app = app;
	data_hyprland.window_params[i].workspace = index;

	app_increment(app);
}
static void window_close(uint8_t i)
{
	app_decrement(data_hyprland.window_params[i].app);

	const uint8_t j = --data_hyprland.window_total;

	data_hyprland.window_params[i] = data_hyprland.window_params[j];
	data_hyprland.window_address[i] = data_hyprland.window_address[j];
}
static void window_move(uint8_t i, uint8_t index)
{
	data_hyprland.window_params[i].workspace = index;
}

static int hyprland_connect(const char* path)
{
	struct sockaddr_un addr = { .sun_family = AF_UNIX };

	const char* hypr_sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");
	const char* xdg_dir = getenv("XDG_RUNTIME_DIR");
	
	char* dst = addr.sun_path;
	for (const char* str = xdg_dir; *str; str++) *dst++ = *str;
	for (const char* str = "/hypr/"; *str; str++) *dst++ = *str;
	for (const char* str = hypr_sig; *str; str++) *dst++ = *str;
	for (const char* str = path; *str; str++) *dst++ = *str;

	const int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	connect(sock, (struct sockaddr*)&addr, sizeof(addr));
	return sock;
}

static void hyprland_switch(unsigned long index)
{
	data_states.dirty_core |= (1 << (ELEMENT_WORKSPACE0 + data_hyprland.workspace_active));
	data_states.dirty_core |= (1 << (ELEMENT_WORKSPACE0 + index));

	const uint32_t i = (1u << index);
	const uint32_t j = (1u << data_hyprland.workspace_active);
	
	if ((WORKSPACE_HIDE_CORE & i) && !(WORKSPACE_HIDE_CORE & j))
	{
		wayland_delete_core();
	}	
	if (!(WORKSPACE_HIDE_CORE & i) && (WORKSPACE_HIDE_CORE & j))
	{
		wayland_create_core();
	}

	data_hyprland.workspace_active = index;
}
static void hyprland_close(unsigned long index)
{
	if (--data_hyprland.workspace_count[index]) return;
	if (index == data_hyprland.workspace_active) return;

	data_states.dirty_core |= (1 << (ELEMENT_WORKSPACE0 + index));
}
static void hyprland_open(unsigned long index)
{
	if (data_hyprland.workspace_count[index]++) return;
	if (index == data_hyprland.workspace_active) return;

	data_states.dirty_core |= (1 << (ELEMENT_WORKSPACE0 + index));
}

static void hyprland_workspace(const char* ptr, const char* end)
{
	while (*ptr++ != ',');
	const long index = string_dec(ptr, end);
	hyprland_switch(index);
}
static void hyprland_openwindow(const char* ptr, const char* end)
{
	while (*ptr++ != '>');
	const long addr = string_hex(ptr + 1, end);
	while (*ptr++ != ',');
	const long index = string_dec(ptr, end);
	while (*ptr++ != ',');
	const long app = string_application(ptr);

	window_open(addr, app, index);
	hyprland_open(index);
}
static void hyprland_closewindow(const char* ptr, const char* end)
{
	while (*ptr++ != '>');
	const long addr = string_hex(ptr + 1, end);

	for (int i = 0; i < data_hyprland.window_total; i++)
	{
		if (data_hyprland.window_address[i] == addr)
		{
			hyprland_close(data_hyprland.window_params[i].workspace);
			window_close(i);
			return;
		}
	}
}
static void hyprland_movewindow(const char* ptr, const char* end)
{
	while (*ptr++ != '>');
	const long addr = string_hex(ptr + 1, end);
	while (*ptr++ != ',');
	while (*ptr++ != ',');
	const long index = string_dec(ptr, end);

	for (int i = 0; i < data_hyprland.window_total; i++)
	{
		if (data_hyprland.window_address[i] == addr)
		{
			hyprland_close(data_hyprland.window_params[i].workspace);
			hyprland_open(index);
			window_move(i, index);
			return;
		}
	}
}

static void hyprland_activeworkspace()
{
	char buffer[4096];

	const int socket = hyprland_connect("/.socket.sock");
	write(socket, "activeworkspace", sizeof("activeworkspace"));
	
	const char* str = buffer;
	const char* end = buffer + read(socket, buffer, sizeof(buffer));

	const long i = string_dec(str + sizeof("workspace ID ") - 1, end);

	data_hyprland.workspace_active = i;
	data_states.dirty_core |= (1 << i);

	if ((1 << i) & WORKSPACE_HIDE_CORE) return;
	
	wayland_create_core();
}
static void hyprland_clients()
{
	char buffer[4096];

	const int socket = hyprland_connect("/.socket.sock");
	write(socket, "clients", sizeof("clients"));

	const char* str = buffer;
	const char* end = buffer + read(socket, buffer, sizeof(buffer));

	const char str_window[] = "Window ";
	const char str_name[] = "\tclass: ";
	const char str_workspace[] = "\tworkspace: ";

	while (str < end)
	{
		const char str_window[] = "Window ";
		const char str_name[] = "\tclass: ";
		const char str_workspace[] = "\tworkspace: ";

		const long addr = string_hex(str + sizeof("Window ") - 1, end);

		long index;
		const char* name;

		while (str < end)
		{
			while (*str++ != '\n');
			if (string_cmp(str, str_workspace))
			{
				index = string_dec(str + sizeof(str_workspace) - 1, end);
			}
			if (string_cmp(str, str_name))
			{
				name = str + sizeof("\tclass: ") - 1;
			}
			if (string_cmp(str, str_window))
			{
				break;
			}
		}

		const long app = string_application(name);

		if (index >= 0 && index <= 9)
		{
			window_open(addr, app, index);
			hyprland_open(index);
		}
	}

	close(socket);
}

int service_create_hyprland()
{

}
int service_pollfd_hyprland()
{
	int fd = hyprland_connect("/.socket2.sock");

	hyprland_activeworkspace();
	hyprland_clients();

	return fd;
}
int service_update_hyprland(int fd, char* buffer, int length)
{
	char* ptr = buffer;
	char* end = buffer + read(fd, buffer, length);

	while (ptr < end)
	{
		if (string_cmp(ptr, "workspacev2>>"))
		{
			hyprland_workspace(ptr, end);
		}
		if (string_cmp(ptr, "openwindow>>"))
		{
			hyprland_openwindow(ptr, end);
		}
		if (string_cmp(ptr, "closewindow>>"))
		{
			hyprland_closewindow(ptr, end);
		}
		if (string_cmp(ptr, "movewindowv2>>"))
		{
			hyprland_movewindow(ptr, end);
		}

		while (ptr < end && *ptr++ != '\n');
	}
}

int service_notify_hyprland_moveto(int i)
{
	char msg[] = "dispatch workspace XX";

	msg[19] = '0' + (i / 10);
	msg[20] = '0' + (i % 10);

	const int socket = hyprland_connect("/.socket.sock");
	write(socket, msg, sizeof(msg));

	char resp[4];

	read(socket, resp, sizeof(resp));
	close(socket);
}
