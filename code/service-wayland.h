int service_create_wayland();
int service_pollfd_wayland();
int service_update_wayland(int fd, char* buffer, unsigned long length);

int service_notify_wayland_core();
