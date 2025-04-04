#include <stdint.h>

extern struct player_t
{
	char text[163];

	uint8_t is_playing;

	uint32_t current;
	uint32_t duration;

} data_player;
