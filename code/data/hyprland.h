#include <stdint.h>

extern struct hyprland_t
{
	uint8_t workspace_count[10];
	uint8_t workspace_active;

	uint8_t window_total;
	uint64_t window_address[40];
	uint8_t window_workspace[40];
	
} data_hyprland;
