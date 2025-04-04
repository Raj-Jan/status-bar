int service_create_dbus();
int service_pollfd_dbus();
int service_update_dbus(int fd, char* buffer, int length);

void service_notify_dbus_next();
void service_notify_dbus_prev();
void service_notify_dbus_swap();
void service_notify_dbus_sync();

void service_notify_dbus_shutdown();
void service_notify_dbus_reboot();
void service_notify_dbus_hibernate();
void service_notify_dbus_firmware();
