int service_create_hyprland();
int service_pollfd_hyprland();
int service_update_hyprland(int fd, char* buffer, int length);

int service_notify_hyprland_moveto(int i);
