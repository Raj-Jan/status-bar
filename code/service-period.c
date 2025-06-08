#include "service-dbus.h"
#include "service-period.h"
#include "data/clock.h"
#include "data/states.h"
#include "data/player.h"
#include "data/system.h"

#include <pwd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/timerfd.h>

#define TIMER_PERIOD 1000000000 // 1s

static void read_file(char* buffer, int length, char* path)
{
	int fd = open(path, O_RDONLY);
	read(fd, buffer, length);
	close(fd);
}

static void create_text(char* dest, double val)
{
	int unit = 0;
	while (val >= 1000)
	{
		unit++;
		val /= 1024;
	}

	if (val < 10)
	{
		dest[0] = '0' + (int)(val / 1) % 10;
		dest[1] = '.';
		dest[2] = '0' + (int)(val * 10) % 10;
		dest[3] = '0' + (int)(val * 100) % 10;
		dest[4] = '0' + (int)(val * 1000) % 10;
	}
	else if (val < 100)
	{
		dest[0] = '0' + (int)(val / 10) % 10;
		dest[1] = '0' + (int)(val / 1) % 10;
		dest[2] = '.';
		dest[3] = '0' + (int)(val * 10) % 10;
		dest[4] = '0' + (int)(val * 100) % 10;
	}
	else
	{
		dest[0] = '0' + (int)(val / 100) % 10;
		dest[1] = '0' + (int)(val / 10) % 10;
		dest[2] = '0' + (int)(val / 1) % 10;
		dest[3] = '.';
		dest[4] = '0' + (int)(val * 10) % 10;
	}

	char units[] = { 'K', 'M', 'G', 'T' };

	dest[6] = units[unit];
}

static long clock_create()
{
	struct timespec now;
	struct tm tm;
	
	clock_gettime(CLOCK_REALTIME, &now);
	localtime_r(&now.tv_sec, &tm);

	data_clock.time_second = tm.tm_sec;
	data_clock.time_minute = tm.tm_min;
	data_clock.time_hour = tm.tm_hour;

	long ret = 0
		|| data_clock.date_day != tm.tm_mday
		|| data_clock.date_month != tm.tm_mon
		|| data_clock.date_weekday != tm.tm_wday;

	data_clock.date_day = tm.tm_mday;
	data_clock.date_month = tm.tm_mon;
	data_clock.date_weekday = tm.tm_wday;

	return ret;
}
static void clock_update(uint64_t num)
{
	const uint8_t second = data_clock.time_second + num;
	const uint8_t minute = data_clock.time_minute + (second / 60);

	data_clock.time_second = second % 60;
	data_clock.time_minute = minute;

	data_states.dirty_core |= (1ul << ELEMENT_CLOCK_TIME);

	if (second < 60) return;

	data_states.dirty_core |= (1ul << ELEMENT_CLOCK_TIME);

	if (minute < 60 || !clock_create()) return;
	
	data_states.dirty_core |= (1ul << ELEMENT_CLOCK_DATE);
}

static void gpu_create_fill(char* buffer, int length)
{
	read_file(buffer, length, "/sys/class/hwmon/hwmon1/device/gpu_busy_percent");

	char* str = buffer;
	const uint64_t usage = strtoul(str, &str, 0);

	data_system.usage_gpu = usage / 100.0;

	data_states.dirty_core |= (1ul << ELEMENT_METER0_FILL);
}
static void gpu_create_text(char* buffer, int length)
{
	read_file(buffer, length, "/sys/class/hwmon/hwmon1/temp1_input");;

	char* str = buffer;
	data_system.value_gpu = strtoul(str, &str, 0) / 1000;

	data_states.dirty_core |= (1ul << ELEMENT_METER0_TEXT);
}
static void gpu_update(char* buffer, int length)
{
	gpu_create_fill(buffer, length);
	gpu_create_text(buffer, length);
}

static void cpu_create_fill(char* buffer, int length)
{
	read_file(buffer, length, "/proc/stat");

	char* str = buffer;
	while (*str != ' ') str++;
	while (*str == ' ') str++;

	uint64_t x[7];

	x[0] = strtoul(str, &str, 0);
	while (*str == ' ') str++;
	x[1] = strtoul(str, &str, 0);
	while (*str == ' ') str++;
	x[2] = strtoul(str, &str, 0);
	while (*str == ' ') str++;
	x[3] = strtoul(str, &str, 0);
	while (*str == ' ') str++;
	x[4] = strtoul(str, &str, 0);
	while (*str == ' ') str++;
	x[5] = strtoul(str, &str, 0);
	while (*str == ' ') str++;
	x[6] = strtoul(str, &str, 0);

	const uint64_t new_total = x[0] + x[1] + x[2] + x[3] + x[4] + x[5] + x[6];
	const uint64_t new_used = x[0] + x[1] + x[2] + x[4] + x[5] + x[6];

	data_system.usage_cpu = (double)(new_used - data_system.cpu_used)
		/ (new_total - data_system.cpu_total);

	data_system.cpu_total = new_total;
	data_system.cpu_used = new_used;

	data_states.dirty_core |= (1ul << ELEMENT_METER1_FILL);
}
static void cpu_create_text(char* buffer, int length)
{
	read_file(buffer, length, "/sys/class/hwmon/hwmon2/temp1_input");

	char* str = buffer;
	data_system.value_cpu = strtoul(str, &str, 0) / 1000;

	data_states.dirty_core |= (1ul << ELEMENT_METER1_TEXT);
}
static void cpu_update(char* buffer, int length)
{
	cpu_create_fill(buffer, length);
	cpu_create_text(buffer, length);
}

static void ram_create_fill(char* buffer, int length)
{
	read_file(buffer, length, "/proc/meminfo");

	char* str = buffer;

	while (*str != ' ') str++;
	while (*str == ' ') str++;
	const uint64_t total = strtoul(str, &str, 0);

	while (*str++ != '\n');
	while (*str++ != '\n');

	while (*str != ' ') str++;
	while (*str == ' ') str++;
	const uint64_t available = strtoul(str, &str, 0);
	const uint64_t used = total - available;

	data_system.used_ram = (double)used / total;
	data_system.value_ram = used;

	data_states.dirty_core |= (1ul << ELEMENT_METER2_FILL);
	data_states.dirty_core |= (1ul << ELEMENT_METER2_TEXT);
}
static void ram_update(char* buffer, int length)
{
	ram_create_fill(buffer, length);
}

static void root_update()
{
	struct statvfs stat = {};
	statvfs("/", &stat);

	const double free_space = stat.f_bsize * stat.f_bavail / 1024.0;
	const double total_space = stat.f_blocks * stat.f_frsize / 1024.0;

	data_system.free_root = 1.0 - free_space / total_space;
	data_system.value_root = free_space;

	data_states.dirty_core |= (1ul << ELEMENT_METER3_FILL);
	data_states.dirty_core |= (1ul << ELEMENT_METER3_TEXT);
}
static void home_update()
{
	struct statvfs stat = {};
	statvfs(getpwuid(getuid())->pw_dir, &stat);

	const double free_space = stat.f_bsize * stat.f_bavail / 1024.0;
	const double total_space = stat.f_blocks * stat.f_frsize / 1024.0;

	data_system.free_home = 1.0 - free_space / total_space;
	data_system.value_home = free_space;

	data_states.dirty_core |= (1ul << ELEMENT_METER4_FILL);
	data_states.dirty_core |= (1ul << ELEMENT_METER4_TEXT);
}

static void player_update()
{
	if (data_player.is_playing)
	{
		if (data_player.current > data_player.duration)
		{
			service_notify_dbus_sync();
		}
		else
		{
			data_player.current += 1000;
		}
		data_states.dirty_core |= (1ul << ELEMENT_PLAYER_BAR);
	}
}

int service_create_period(char* buffer, int length)
{
	clock_create();
	gpu_update(buffer, length);
	cpu_update(buffer, length);
	ram_update(buffer, length);
	root_update();
	home_update();
}
int service_pollfd_period()
{
	const int fd = timerfd_create(0, 0);

	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);

	uint64_t overhead = (now.tv_sec % 1000) * 1000000000 + now.tv_nsec;
	overhead = overhead % TIMER_PERIOD;
	overhead = TIMER_PERIOD - overhead;

	struct itimerspec time;
	time.it_interval.tv_sec = (TIMER_PERIOD / 1000000000);
	time.it_interval.tv_nsec = (TIMER_PERIOD % 1000000000);
	time.it_value.tv_sec = overhead / 1000000000;
	time.it_value.tv_nsec = overhead % 1000000000;

	timerfd_settime(fd, 0, &time, 0);
	return fd;
}

int service_update_period(int fd, char* buffer, int length)
{
	uint64_t num;
	read(fd, &num, sizeof(num));

	clock_update(num);
	cpu_update(buffer, length);
	gpu_update(buffer, length);
	ram_update(buffer, length);
	root_update();
	home_update();

	player_update();
}
