#include "protocol/wlr-layer-shell.h"
#include "wayland.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

static struct wl_pointer* pointer;
static struct wl_seat* seat;
static struct wl_shm* shm;
static struct wl_compositor* compositor;
static struct zwlr_layer_shell_v1* layer_shell;

static struct wl_registry_listener register_listener;
static struct wl_seat_listener seat_listener;
static struct wl_pointer_listener pointer_listener;

// =======================================================================
//                               wl_pointer                              
// =======================================================================

static void pointer_motion(void *data, struct wl_pointer* wl_pointer,
	uint32_t time, wl_fixed_t x, wl_fixed_t y)
{

}
static void pointer_button(void* data, struct wl_pointer* wl_pointer,
	uint32_t serial, uint32_t time, uint32_t button, uint32_t states)
{

}
static void pointer_axis(void* data, struct wl_pointer* wl_pointer,
  uint32_t time,  uint32_t axis, wl_fixed_t value)
{

}
static void pointer_frame(void* data, struct wl_pointer* wl_pointer
	)
{

}
static void pointer_axis_source(void* data, struct wl_pointer* wl_pointer,
   uint32_t axis_source)
{

}
static void pointer_axis_stop(void* data,struct wl_pointer* wl_pointer,
   uint32_t time, uint32_t axis)
{
   
}
static void pointer_axis_discrete(void* data, struct wl_pointer* wl_pointer,
   uint32_t axis, int32_t discrete)
{
   
}
static void pointer_axis_value120(void* data, struct wl_pointer* wl_pointer,
   uint32_t axis, int32_t value120)
{
   
}
static void pointer_axis_relative_direction(void* data, struct wl_pointer* wl_pointer,
   uint32_t axis, uint32_t direction)
{

}

static void pointer_enter(void* data, struct wl_pointer* wl_pointer,
   uint32_t serial, struct wl_surface* surface, wl_fixed_t x, wl_fixed_t y)
{
	struct wayland_t* instance = wl_surface_get_user_data(surface);
	if (instance->pointer.enter)
	{
		instance->pointer.enter(data, &pointer_listener, serial, x, y);
	}
}
static void pointer_leave(void* data, struct wl_pointer* wl_pointer,
   uint32_t serial, struct wl_surface* surface)
{
	struct wayland_t* instance = wl_surface_get_user_data(surface);
	if (instance->pointer.leave)
	{
		instance->pointer.leave(data, &pointer_listener, serial);
	}
	pointer_listener = (struct wl_pointer_listener){
		.enter = pointer_enter,
		.leave = pointer_leave,
		.motion = pointer_motion,
		.button = pointer_button,
		.axis = pointer_axis,
		.frame = pointer_frame,
		.axis_source = pointer_axis_source,
		.axis_stop = pointer_axis_stop,
		.axis_discrete = pointer_axis_discrete,
		.axis_value120 = pointer_axis_value120,
		.axis_relative_direction = pointer_axis_relative_direction
	};
}

// =======================================================================
//                                 wl_seat                                
// =======================================================================

static void seat_capabilities(
   void* data, struct wl_seat* seat, uint32_t capabilities)
{
	if (capabilities & WL_SEAT_CAPABILITY_POINTER)
	{
		if (!pointer)
		{
			pointer = wl_seat_get_pointer(seat);
			wl_pointer_add_listener(pointer, &pointer_listener, 0);
		}
	}
	else if (pointer)
	{
		wl_pointer_destroy(pointer);
		pointer = 0;
	}
}
static void seat_name(
   void* data, struct wl_seat* wl_seat, const char* name)
{
   
}

// =======================================================================
//                               wl_registry                              
// =======================================================================

static void registry_global(void* data, struct wl_registry* registry,
   uint32_t name, const char* interface, uint32_t version)
{
	if (strcmp(interface, wl_shm_interface.name) == 0)
	{
		shm = wl_registry_bind(
			registry, name, &wl_shm_interface, 1);
	}
	else if (strcmp(interface, wl_compositor_interface.name) == 0)
	{
		compositor = wl_registry_bind(
			registry, name, &wl_compositor_interface, 4);
	}
	else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0)
	{
		layer_shell = wl_registry_bind(
			registry, name, &zwlr_layer_shell_v1_interface, 5);
	}
	else if (strcmp(interface, wl_seat_interface.name) == 0)
	{
		seat = wl_registry_bind(
			registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(seat, &seat_listener, 0);
	}
}
static void registry_remove(void* data, struct wl_registry* registry,
   uint32_t name)
{
   
}

// =======================================================================
//                                listeners                               
// =======================================================================

static struct wl_pointer_listener pointer_listener =
{
   .enter = pointer_enter,
   .leave = pointer_leave,
   .motion = pointer_motion,
   .button = pointer_button,
   .axis = pointer_axis,
   .frame = pointer_frame,
   .axis_source = pointer_axis_source,
   .axis_stop = pointer_axis_stop,
   .axis_discrete = pointer_axis_discrete,
   .axis_value120 = pointer_axis_value120,
   .axis_relative_direction = pointer_axis_relative_direction
};
static struct wl_seat_listener seat_listener =
{ .capabilities = seat_capabilities, .name = seat_name };
static struct wl_registry_listener register_listener =
{ .global = registry_global, .global_remove = registry_remove };

// =======================================================================
//                                 wayland                                
// =======================================================================

void wayland_emplace(struct wl_display* display)
{
    struct wl_registry* registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &register_listener, 0);
    wl_display_roundtrip(display);
}
void wayland_destroy(struct wl_display* display)
{
}

void* wayland_setup(struct wayland_t* this,
	int layer, const char* name)
{
	this->surface = wl_compositor_create_surface(compositor);
	wl_surface_set_user_data(this->surface, this);
	this->surface_v1 = zwlr_layer_shell_v1_get_layer_surface(layer_shell,
		this->surface, 0, layer, name);

	return this->surface_v1;
}
void* wayland_close(struct wayland_t* this)
{
	if (this->callback)
	{
		wl_callback_destroy(this->callback);
        this->callback = NULL;
	}
	if (this->surface_v1)
	{
		zwlr_layer_surface_v1_destroy(this->surface_v1);
		this->surface_v1 = 0;
	}
	if (this->surface)
	{
		wl_surface_destroy(this->surface);
		this->surface = 0;
	}
	if (this->buffer)
	{
		wl_buffer_destroy(this->buffer);
		this->buffer = 0;
	}
	if (this->memory_ptr)
	{
		munmap(this->memory_ptr, this->memory_len);
		this->memory_ptr = 0;
		this->memory_len = 0;
	}
}
void* wayland_alloc(struct wayland_t* this, 
	int width, int height, const char* name)
{
	const int size = width * height * 4;
	const int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
	shm_unlink(name);
	ftruncate(fd, size);

	void* memory = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	void* pool = wl_shm_create_pool(shm, fd, size);
	close(fd);

	this->memory_ptr = memory;
	this->memory_len = size;

	this->buffer = wl_shm_pool_create_buffer(
		pool, 0, width, height, width * 4, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);
	return memory;
}
void wayland_frame(struct wayland_t* this,
	const struct wl_callback_listener* listener, int states)
{
	if (states)
	{
		this->callback = wl_surface_frame(this->surface);
		wl_callback_add_listener(this->callback, listener, 0);
	}
	else
	{
		this->callback = 0;
	}

    wl_surface_attach(this->surface, this->buffer, 0, 0);
    wl_surface_commit(this->surface);
}
void wayland_touch(struct wayland_t* this, 
	const struct wl_callback_listener* listener)
{
	if (this->surface == 0) return;

	struct wl_callback* callback = wl_surface_frame(this->surface);
	wl_callback_add_listener(callback, listener, 0);
}
