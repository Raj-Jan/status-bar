#include <stdint.h>

extern struct system_t
{
	uint64_t cpu_used;
	uint64_t cpu_total;

	double usage_gpu;
	double usage_cpu;
	double used_ram;
	double free_home;
	double free_root;

	uint32_t value_gpu;
	uint32_t value_cpu;
	uint32_t value_ram;
	uint32_t value_home;
	uint32_t value_root;

} data_system;
