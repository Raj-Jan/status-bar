#include "service-wayland.h"
#include "wayland/wayland.h"
#include "wayland/wayland-core.h"

#include <wayland-client-core.h>

static struct wl_display* display;

static int create_prepare()
{
	if (wl_display_prepare_read(display) < 0)
	{
		if (wl_display_dispatch_pending(display) == 0) return -1;
	}
	return wl_display_flush(display);
}

int service_create_wayland()
{
	display = wl_display_connect(0);
	wayland_emplace(display);
	wayland_create_core();
}
int service_pollfd_wayland()
{
	create_prepare();

	return wl_display_get_fd(display);
}
int service_update_wayland(int fd, char* buffer, unsigned long length)
{
	int ret;

	ret = wl_display_read_events(display);
	if (ret >= 0)
	{
		ret = wl_display_dispatch_pending(display);
		if (ret == 0)
		{
			ret = wl_display_prepare_read(display);
			if (ret < 0)
			{
				ret = wl_display_dispatch_pending(display);
				if (ret < 0)
				{
					return ret;
				}
			}
		}
	}

	return create_prepare();
}
int service_notify_wayland_core()
{
	wayland_update_core();
	return wl_display_flush(display);
}
