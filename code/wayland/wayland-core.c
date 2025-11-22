#include "wayland.h"
#include "wayland-core.h"
#include "protocol/wlr-layer-shell.h"
#include "draw/draw-core.h"
#include "input/input-core.h"
#include "layout/layout-core.h"

static struct wayland_t instance;

static struct wl_callback_listener listener_callback;
static struct zwlr_layer_surface_v1_listener listener_surface;

static void callback_done(void* data, struct wl_callback* callback, uint32_t time)
{
	wl_callback_destroy(callback);

	const int states = draw_core_frame();
	wayland_frame(&instance, &listener_callback, states);
}

static void layer_surface_config(
	void* data, struct zwlr_layer_surface_v1* layer_surface, 
	uint32_t serial, uint32_t width, uint32_t height)
{
	zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

	const char* name = "/wl_shm-6935fc26a79017a731fc88a032036dff-core";
	void* const memory = wayland_alloc(&instance, width, height, name);

	const int states = draw_core_setup(memory);
	wayland_frame(&instance, &listener_callback, states);
}
static void layer_surface_closed(
	void* data, struct zwlr_layer_surface_v1* layer_surface)
{

}

static void pointer_motion(void *data, struct wl_pointer* pointer,
	uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
	input_core_motion(x / 256.0, y / 256.0);
}
static void pointer_button(void *data, struct wl_pointer* pointer,
	uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	input_core_button(button, state);
}
static void pointer_axis(void* data, struct wl_pointer* wl_pointer,
	uint32_t time, uint32_t axis, wl_fixed_t value)
{
	input_core_axis(axis, value / 3840);
}

static void pointer_enter(void *data, struct wl_pointer_listener* listener,
	uint32_t serial, wl_fixed_t x, wl_fixed_t y)
{
	input_core_motion(x / 256.0, y / 256.0);

	listener->motion = pointer_motion;
	listener->button = pointer_button;
	listener->axis = pointer_axis;
}

static void pointer_leave(void *data, struct wl_pointer_listener* listener,
	uint32_t serial)
{
	input_core_leave();
	
	listener->motion = 0;
	listener->button = 0;
	listener->axis = 0;
}

static struct wayland_t instance =
{ .pointer = { .enter = pointer_enter, .leave = pointer_leave } };

static struct wl_callback_listener listener_callback =
{ .done = callback_done };
static struct zwlr_layer_surface_v1_listener listener_surface =
{ .configure = layer_surface_config, .closed = layer_surface_closed };

void wayland_create_core()
{
	struct zwlr_layer_surface_v1* surface = wayland_setup(&instance,
		ZWLR_LAYER_SHELL_V1_LAYER_TOP, "status-core");

	const uint32_t m = WINDOW_MARGIN;
	const uint32_t h = WINDOW_HEIGHT;

	zwlr_layer_surface_v1_set_size(surface, 0, h);
	zwlr_layer_surface_v1_set_anchor(surface,
		ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
		ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
	zwlr_layer_surface_v1_set_exclusive_zone(surface, h);
	zwlr_layer_surface_v1_set_margin(surface, m, m, 0, m);
	zwlr_layer_surface_v1_add_listener(surface, &listener_surface, 0);

	wl_surface_commit(instance.surface);
}
void wayland_delete_core()
{
	wayland_close(&instance);
	draw_core_close();
}
void wayland_update_core()
{
	wayland_touch(&instance, &listener_callback);
}
