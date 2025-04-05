#include "service-curl.h"
#include "draw/draw-core.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>

struct MemoryBuffer
{
    unsigned char *data;
    size_t size;
};

static size_t write_callback(
	void *contents, size_t size, size_t nmemb, void *userp)
{
	struct MemoryBuffer* mem = (struct MemoryBuffer*)userp;
	
	size_t realsize = size * nmemb;

	void* ptr = realloc(mem->data, mem->size + realsize);
	if (!ptr) return 0;

	mem->data = ptr;
	memcpy(mem->data + mem->size, contents, realsize);
	mem->size += realsize;
	return realsize;
}

int service_create_curl()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

void service_notify_curl_load(const char* url)
{
	const char* name = url;
	for (int i = 0; url[i]; i++)
	{
		if (url[i] == '/') name = url + i + 1;
	}

	struct stat sb;
	if (stat(name, &sb) < 0)
	{
		struct MemoryBuffer mem = { 0 };

		CURL *curl = curl_easy_init();
		if (!curl) return;
	
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	
		CURLcode res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		int width, height, channels;
		void* img = stbi_load_from_memory(
			mem.data, mem.size, &width, &height, &channels, STBI_rgb_alpha);
	
		stbi_write_png(name, width, height, 4, img, width * 4);
		stbi_image_free(img);

		free(mem.data);
	}

	draw_core_image(name);
}
