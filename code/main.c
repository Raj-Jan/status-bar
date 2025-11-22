#include "main.h"
#include "service-dbus.h"
#include "service-hyprland.h"
#include "service-period.h"
#include "service-wayland.h"
#include "wayland/wayland-core.h"

#include <poll.h>
#include <unistd.h>
#include <sys/stat.h>

#define CACHE_PATH "/home/remi/.cache/status-bar"

static char buffer[4096];

static void set_directory(const char* path)
{
	struct stat sb;
	if (stat(path, &sb) != 0)
	{
		mkdir(path, 0700);
	}
	chdir(path);
}

int main(int argc, char* argv[])
{
	set_directory(CACHE_PATH);

	service_create_dbus();
	service_create_wayland();
	service_create_hyprland();
	service_create_period(buffer, sizeof(buffer));

	struct pollfd fds[] = {
		{ .fd = service_pollfd_dbus(), .events = POLLIN },
		{ .fd = service_pollfd_hyprland(), .events = POLLIN },
		{ .fd = service_pollfd_period(), .events = POLLIN },
		{ .fd = service_pollfd_wayland(), .events = POLLIN },
	};

	while (poll(fds, sizeof(fds) / sizeof(*fds), -1) > 0)
	{
		if (fds[3].revents & POLLIN)
		{
			service_update_wayland(fds[3].fd, buffer, sizeof(buffer));
		}

		if (fds[0].revents & POLLIN)
		{
			service_update_dbus(fds[0].fd, buffer, sizeof(buffer));
		}
		if (fds[1].revents & POLLIN)
		{
			service_update_hyprland(fds[1].fd, buffer, sizeof(buffer));
		}
		if (fds[2].revents & POLLIN)
		{
			service_update_period(fds[2].fd, buffer, sizeof(buffer));
		}
		
		if (!data_states.frame_requested_core && data_states.dirty_core != 0)
		{
			wayland_update_core();
			service_notify_wayland_core();
		}
	}
}
