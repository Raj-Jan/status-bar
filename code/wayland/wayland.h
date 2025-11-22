struct wl_buffer;
struct wl_surface;
struct wl_display;
struct wl_callback;

struct wl_callback_listener;
struct wl_pointer_listener;

struct zwlr_layer_surface_v1;

struct wayland_t
{
	struct
	{
		void (*enter)(void *data, struct wl_pointer_listener* listener,
			unsigned int serial, int x, int y);
		void (*leave)(void *data, struct wl_pointer_listener* listener,
			unsigned int serial);

	} pointer;

	struct wl_buffer* buffer;
	struct wl_surface* surface;
	struct wl_callback* callback;
	struct zwlr_layer_surface_v1* surface_v1;

	void* memory_ptr;
	unsigned long memory_len;
};

void wayland_emplace(struct wl_display* display);
void wayland_destroy(struct wl_display* display);

void* wayland_setup(struct wayland_t* this,
	int layer, const char* name);
void* wayland_close(struct wayland_t* this);
void* wayland_alloc(struct wayland_t* this, 
	int width, int height, const char* name);
void wayland_frame(struct wayland_t* this, 
	const struct wl_callback_listener* listener, int states);
void wayland_touch(struct wayland_t* this, 
	const struct wl_callback_listener* listener);
