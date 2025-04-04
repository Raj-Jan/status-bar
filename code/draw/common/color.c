#include "color.h"

// ======================================================================
//                                 COLORS                                
// ======================================================================

#define MAKE_COLOR(hex) \
	((hex >> 0x10) & 0xff) / 255.,\
	((hex >> 0x08) & 0xff) / 255.,\
	((hex >> 0x00) & 0xff) / 255.

const double colors[] = {
	MAKE_COLOR(0xf5e0dc), // rosewater
	MAKE_COLOR(0xf2cdcd), // flamingo
	MAKE_COLOR(0xf5c2e7), // pink
	MAKE_COLOR(0xcba6f7), // mauve
	MAKE_COLOR(0xf38ba8), // red
	MAKE_COLOR(0xeba0ac), // maroon
	MAKE_COLOR(0xfab387), // peach
	MAKE_COLOR(0xa6e3a1), // green
	MAKE_COLOR(0x74c7ec), // sapphire
	MAKE_COLOR(0x6c7086), // overlay0
	MAKE_COLOR(0x585b70), // surface2
	MAKE_COLOR(0x313244), // surface0
};
