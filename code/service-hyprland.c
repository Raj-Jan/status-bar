#include "service-hyprland.h"
#include "data/states.h"
#include "data/hyprland.h"
#include "data/tray.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

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
	if (string_cmp(name, "Spotify")) return 4;
	if (string_cmp(name, "blender")) return 5;
	if (string_cmp(name, "krita")) return 6;
	if (string_cmp(name, "osu!")) return 07;

	return -1;
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
	const uint64_t mask1 = (1 << data_hyprland.workspace_active);
	const uint64_t mask2 = (1 << index);

	data_states.dirty_core |= (1 << (ELEMENT_WORKSPACE0 + data_hyprland.workspace_active));
	data_states.dirty_core |= (1 << (ELEMENT_WORKSPACE0 + index));

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

	if (app >= 0 && data_tray.address[app] == 0)
	{
		data_tray.address[app] = addr;

		data_states.dirty_core |= (1ul << (ELEMENT_APP0 + app));
	}

	data_hyprland.window_address[data_hyprland.window_total] = addr;
	data_hyprland.window_workspace[data_hyprland.window_total] = index;

	data_hyprland.window_total++;

	hyprland_open(index);
}
static void hyprland_closewindow(const char* ptr, const char* end)
{
	while (*ptr++ != '>');
	const long addr = string_hex(ptr + 1, end);

	for (int i = 0; i < 8; i++)
	{
		if (data_tray.address[i] == addr)
		{
			data_tray.address[i] = 0;
			
			data_states.dirty_core |= (1ul << (ELEMENT_APP0 + i));
		}
	}

	for (int i = 0; i < data_hyprland.window_total; i++)
	{
		if (data_hyprland.window_address[i] == addr)
		{
			hyprland_close(data_hyprland.window_workspace[i]);

			--data_hyprland.window_total;

			data_hyprland.window_workspace[i] =
				data_hyprland.window_workspace[data_hyprland.window_total];
			data_hyprland.window_address[i] =
				data_hyprland.window_address[data_hyprland.window_total];
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
			hyprland_close(data_hyprland.window_workspace[i]);
			hyprland_open(index);

			data_hyprland.window_workspace[i] = index;
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
}
static void hyprland_clients()
{
	char buffer[4096];

	const int socket = hyprland_connect("/.socket.sock");
	write(socket, "clients", sizeof("clients"));

	const char* str = buffer;
	const char* end = buffer + read(socket, buffer, sizeof(buffer));

	while (str < end)
	{
		const long addr = string_hex(str + sizeof("Window ") - 1, end);

		while (*str++ != '\n');
		while (*str++ != '\n');
		while (*str++ != '\n');
		while (*str++ != '\n');
		while (*str++ != '\n');

		const long workspace = string_dec(str + sizeof("\tworkspace: ") - 1, end);

		data_hyprland.window_address[data_hyprland.window_total] = addr;
		data_hyprland.window_workspace[data_hyprland.window_total] = workspace;
		data_hyprland.window_total++;

		if (workspace >= 0)
		{
			data_hyprland.workspace_count[workspace]++;
			data_states.dirty_core |= (1 << workspace);
		}

		while (*str++ != '\n');
		while (*str++ != '\n');
		while (*str++ != '\n');
		while (*str++ != '\n');

		const char* name = str + sizeof("\tclass: ") - 1;

		const long app = string_application(name);
		if (app >= 0) data_tray.address[app] = addr;

		for (int i = 0; i < 15; i++) while (*str++ != '\n');
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
