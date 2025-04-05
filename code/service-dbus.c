#include "service-curl.h"
#include "service-dbus.h"
#include "data/states.h"
#include "data/player.h"
#include "draw/draw-core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include <sys/stat.h>

static DBusConnection* conn;

static void send_dbus_method_call(const char* method)
{
    DBusMessage* msg;
    DBusMessage* reply;
    DBusError err;
    dbus_error_init(&err);

    msg = dbus_message_new_method_call(
        "org.mpris.MediaPlayer2.spotify",       // Destination service (Spotify)
        "/org/mpris/MediaPlayer2",              // Object path
        "org.mpris.MediaPlayer2.Player",        // Interface
        method                                  // Method name (Play, Pause, Next, Previous)
    );

    if (!msg) 
	{
        fprintf(stderr, "Failed to create message!\n");
        return;
    }

    reply = dbus_connection_send_with_reply_and_block(conn, msg, 1000, &err);
    dbus_message_unref(msg);

    if (!reply)
	{
        fprintf(stderr, "DBus Error: %s\n", err.message);
        dbus_error_free(&err);
        return;
    }

    dbus_message_unref(reply);
}
static void send_system_method_call(DBusMessage *msg)
{
    if (!msg)
	{
        fprintf(stderr, "Failed to create D-Bus message\n");
        return;
    }

	DBusConnection *conn;
    DBusMessage *reply;
    DBusError err;
    DBusPendingCall *pending;
    int32_t response_code; // If the reply has an integer response

    // Initialize the error
    dbus_error_init(&err);

    // Connect to the system bus
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (!conn) {
        fprintf(stderr, "Failed to connect to the D-Bus system bus: %s\n", err.message);
        dbus_error_free(&err);
        return;
    }

    // Send the message and wait for a reply
    if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) {
        fprintf(stderr, "Failed to send D-Bus message\n");
        dbus_message_unref(msg);
        return;
    }

    if (!pending) {
        fprintf(stderr, "Failed to create pending call\n");
        dbus_message_unref(msg);
        return;
    }

    dbus_connection_flush(conn);
    dbus_message_unref(msg);

    // Block until we receive a reply
    dbus_pending_call_block(pending);
    reply = dbus_pending_call_steal_reply(pending);
    dbus_pending_call_unref(pending);

    if (!reply) {
        fprintf(stderr, "No reply received\n");
        return;
    }

    // Check if there's an error in the reply
    if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
        const char *error_name = dbus_message_get_error_name(reply);
        fprintf(stderr, "D-Bus error: %s\n", error_name);
    } else {
        // Try reading an integer response (if applicable)
        if (dbus_message_get_args(reply, &err, DBUS_TYPE_INT32, &response_code, DBUS_TYPE_INVALID)) {
            printf("Received response code: %d\n", response_code);
        } else if (dbus_error_is_set(&err)) {
            fprintf(stderr, "Error parsing response: %s\n", err.message);
            dbus_error_free(&err);
        } else {
            printf("D-Bus call executed successfully, but no response data.\n");
        }
    }

    dbus_message_unref(reply);
}

static int save_artist(int i, const char* artist)
{
	data_states.dirty_core |= (1ul << ELEMENT_PLAYER_TEXT);

	while (i < sizeof(data_player.text) && *artist)
	{
		data_player.text[i++] =* artist++;
	}

	if (i < sizeof(data_player.text)) data_player.text[i++] = ',';
	if (i < sizeof(data_player.text)) data_player.text[i++] = ' ';

	return i;
}
static int save_title(int i, const char* title)
{
	data_states.dirty_core |= (1ul << ELEMENT_PLAYER_TEXT);

	while (i < sizeof(data_player.text) &&* title)
	{
		data_player.text[i++] = *title++;
	}
	return i;
}
static int save_duration(int i, uint64_t duration)
{
	data_states.dirty_core |= (1ul << ELEMENT_PLAYER_BAR);

	data_player.duration = duration / 1000;
	data_player.current = 0;
	return i;
}
static int save_image(int i, const char* url)
{
	service_notify_curl_load(url);
	return i;
}

static int extract_metadata_artist(DBusMessageIter* entry, int i)
{
	if (dbus_message_iter_get_arg_type(entry) == DBUS_TYPE_VARIANT)
	{
		DBusMessageIter variant, array;
		dbus_message_iter_recurse(entry, &variant);
		if (dbus_message_iter_get_arg_type(&variant) == DBUS_TYPE_ARRAY)
		{
			dbus_message_iter_recurse(&variant, &array);
			while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_STRING)
			{
				const char* artist;
				dbus_message_iter_get_basic(&array, &artist);
				dbus_message_iter_next(&array);

				i = save_artist(i, artist);
			}
		}
	}

	if (data_player.text[i - 1] == ' ') i--;
	if (data_player.text[i - 1] == ',') i--;

	if (i < sizeof(data_player.text)) data_player.text[i++] = ' ';
	if (i < sizeof(data_player.text)) data_player.text[i++] = '-';
	if (i < sizeof(data_player.text)) data_player.text[i++] = ' ';

	return i;
}
static int extract_metadata_title(DBusMessageIter* entry, int i)
{
	if (dbus_message_iter_get_arg_type(entry) == DBUS_TYPE_VARIANT)
	{
		DBusMessageIter variant;
		dbus_message_iter_recurse(entry, &variant);
		if (dbus_message_iter_get_arg_type(&variant) == DBUS_TYPE_STRING)
		{
			const char* title;
			dbus_message_iter_get_basic(&variant, &title);

			i = save_title(i, title);
		}
	}

	return i;
}
static int extract_metadata_duration(DBusMessageIter* entry, int i)
{
	if (dbus_message_iter_get_arg_type(entry) == DBUS_TYPE_VARIANT)
	{
		DBusMessageIter variant;
		dbus_message_iter_recurse(entry, &variant);
		if (dbus_message_iter_get_arg_type(&variant) == DBUS_TYPE_UINT64)
		{
			uint64_t duration = 0;
			dbus_message_iter_get_basic(&variant, &duration);

			i = save_duration(i, duration);
		}
	}

	return i;
}
static int extract_metadata_image(DBusMessageIter* entry, int i)
{
	if (dbus_message_iter_get_arg_type(entry) == DBUS_TYPE_VARIANT)
	{
		DBusMessageIter variant;
		dbus_message_iter_recurse(entry, &variant);
		if (dbus_message_iter_get_arg_type(&variant) == DBUS_TYPE_STRING)
		{
			const char* url;
			dbus_message_iter_get_basic(&variant, &url);

			i = save_image(i, url);
		}
	}

	return i;
}

static void extract_metadata(DBusMessageIter* iter)
{
	DBusMessageIter dict, entry, variant;
	const char* key;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_VARIANT) return;
	dbus_message_iter_recurse(iter, &variant);
	if (dbus_message_iter_get_arg_type(&variant) != DBUS_TYPE_ARRAY) return;

	int i = 0;
	dbus_message_iter_recurse(&variant, &dict);
	while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY)
	{
		dbus_message_iter_recurse(&dict, &entry);
		dbus_message_iter_get_basic(&entry, &key);
		dbus_message_iter_next(&entry);

		if (strcmp(key, "xesam:artist") == 0)
		{
			i = extract_metadata_artist(&entry, i);
		} 
		else if (strcmp(key, "xesam:title") == 0) 
		{
			i = extract_metadata_title(&entry, i);
		} 
		else if (strcmp(key, "mpris:length") == 0)
		{
			i = extract_metadata_duration(&entry, i);
		}
		else if (strcmp(key, "mpris:artUrl") == 0)
		{
			i = extract_metadata_image(&entry, i);
		}

		dbus_message_iter_next(&dict);
	}

	if (i == sizeof(data_player.text))
	{
		data_player.text[i - 1] = '\0';
	}
	else
	{
		data_player.text[i] = '\0';
	}
}
static void extract_position(DBusMessageIter* iter)
{
	uint64_t pos = 0;
	if (dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_VARIANT)
	{
		DBusMessageIter variant;
		dbus_message_iter_recurse(iter, &variant);
		dbus_message_iter_get_basic(&variant, &pos);
		data_player.current = pos / 1000; // Convert microseconds to ms

		data_states.dirty_core |= (1ul << ELEMENT_PLAYER_BAR);
	}
}
static void extract_status(DBusMessageIter* iter)
{
	data_states.dirty_core |= (1ul << ELEMENT_PLAYER_BUTTON);

	const char* status;
	if (dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_VARIANT)
	{
		DBusMessageIter variant;
		dbus_message_iter_recurse(iter, &variant);
		dbus_message_iter_get_basic(&variant, &status);
		
		data_player.is_playing = strcmp(status, "Playing") == 0;
	}
}

static void handle_dbus_message(DBusMessage* msg)
{
	DBusMessageIter args;
	const char* interface_name;

	if (!dbus_message_iter_init(msg, &args)) return;
	dbus_message_iter_get_basic(&args, &interface_name);
	
	dbus_message_iter_next(&args);
	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY) return;

	DBusMessageIter dict;
	dbus_message_iter_recurse(&args, &dict);

	while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY)
	{
		DBusMessageIter dict_entry;
		dbus_message_iter_recurse(&dict, &dict_entry);

		const char* property_name;
		dbus_message_iter_get_basic(&dict_entry, &property_name);
		dbus_message_iter_next(&dict_entry);

		if (strcmp(property_name, "Metadata") == 0)
		{
			extract_metadata(&dict_entry);
		} 
		else if (strcmp(property_name, "Position") == 0)
		{
			extract_position(&dict_entry);
		}
		else if (strcmp(property_name, "PlaybackStatus") == 0)
		{
			extract_status(&dict_entry);
		}

		dbus_message_iter_next(&dict);
	}
}

static void fetch_metadata()
{
	DBusMessage* msg;
	DBusMessage* reply;
	DBusMessageIter args;
	DBusError err;
	dbus_error_init(&err);

	msg = dbus_message_new_method_call(
		"org.mpris.MediaPlayer2.spotify",
		"/org/mpris/MediaPlayer2",
		"org.freedesktop.DBus.Properties",
		"Get"
	);

	const char* property = "Metadata";
	const char* argument = "org.mpris.MediaPlayer2.Player";
	dbus_message_append_args(msg, DBUS_TYPE_STRING, &argument,
		DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
	if (!reply)
	{
		fprintf(stderr, "⚠️ Failed to fetch metadata: %s\n", err.message);
		dbus_error_free(&err);
		return;
	}

	dbus_message_iter_init(reply, &args);
	extract_metadata(&args);
	dbus_message_unref(reply);
	dbus_message_unref(msg);
}
static void fetch_position()
{
	DBusMessage* msg;
	DBusMessage* reply;
	DBusMessageIter args;
	DBusError err;
	dbus_error_init(&err);

	msg = dbus_message_new_method_call(
		"org.mpris.MediaPlayer2.spotify",
		"/org/mpris/MediaPlayer2",
		"org.freedesktop.DBus.Properties",
		"Get"
	);

	const char* property = "Position";
	const char* argument = "org.mpris.MediaPlayer2.Player";
	dbus_message_append_args(msg,
		DBUS_TYPE_STRING, &argument,
		DBUS_TYPE_STRING, &property,
		DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
	if (!reply)
	{
		fprintf(stderr, "⚠️ Failed to fetch metadata: %s\n", err.message);
		dbus_error_free(&err);
		return;
	}

	dbus_message_iter_init(reply, &args);
	extract_position(&args);
	dbus_message_unref(reply);
	dbus_message_unref(msg);
}
static void fetch_status()
{
	DBusMessage* msg;
	DBusMessage* reply;
	DBusMessageIter args;
	DBusError err;
	dbus_error_init(&err);

	msg = dbus_message_new_method_call(
		"org.mpris.MediaPlayer2.spotify",
		"/org/mpris/MediaPlayer2",
		"org.freedesktop.DBus.Properties",
		"Get"
	);

	const char* property = "PlaybackStatus";
	const char* argument = "org.mpris.MediaPlayer2.Player";
	dbus_message_append_args(msg, DBUS_TYPE_STRING, &argument,
		DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
	if (!reply)
	{
		fprintf(stderr, "⚠️ Failed to fetch metadata: %s\n", err.message);
		dbus_error_free(&err);
		return;
	}

	dbus_message_iter_init(reply, &args);
	extract_status(&args);
	dbus_message_unref(reply);
	dbus_message_unref(msg);
}

int service_create_dbus()
{
	DBusError err;
	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (!conn)
	{
		dbus_error_free(&err);
		return -1;
	}

	dbus_bus_add_match(conn,
		"type='signal',"
		"interface='org.freedesktop.DBus.Properties',"
		"sender='org.mpris.MediaPlayer2.spotify'",
		&err);
	dbus_connection_flush(conn);
	
	fetch_metadata();
	fetch_position();
	fetch_status();
}
int service_pollfd_dbus()
{
	int fd;
	dbus_connection_get_unix_fd(conn, &fd);

	return fd;
}
int service_update_dbus(int fd, char* buffer, int length)
{
	dbus_connection_read_write_dispatch(conn, 0);

	DBusMessage* msg;
	while (msg = dbus_connection_pop_message(conn))
	{
		if (dbus_message_is_signal(msg, 
			"org.freedesktop.DBus.Properties", "PropertiesChanged"))
		{
			handle_dbus_message(msg);
			printf("Spotify event detected!\n");
		}
		dbus_message_unref(msg);
	}
}

void service_notify_dbus_next()
{
	send_dbus_method_call("Next");
}
void service_notify_dbus_prev()
{
	send_dbus_method_call("Previous");
}
void service_notify_dbus_swap()
{
	send_dbus_method_call("PlayPause");
	fetch_position();
}
void service_notify_dbus_sync()
{
	fetch_position();
}

void service_notify_dbus_shutdown()
{
	DBusMessage* msg = dbus_message_new_method_call(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        "PowerOff");

    dbus_bool_t dont_force = 0;
    dbus_message_append_args(msg,
		DBUS_TYPE_BOOLEAN, &dont_force,
		DBUS_TYPE_INVALID);

	send_system_method_call(msg);
}
void service_notify_dbus_reboot()
{
	DBusMessage* msg = dbus_message_new_method_call(
		"org.freedesktop.login1",
		"/org/freedesktop/login1",
		"org.freedesktop.login1.Manager",
		"Reboot");

	dbus_bool_t dont_force = 0;
	dbus_message_append_args(msg,
		DBUS_TYPE_BOOLEAN, &dont_force,
		DBUS_TYPE_INVALID);

	send_system_method_call(msg);
}
void service_notify_dbus_hibernate()
{
	DBusMessage* msg = dbus_message_new_method_call(
		"org.freedesktop.login1",
		"/org/freedesktop/login1",
		"org.freedesktop.login1.Manager",
		"Hibernate");

	dbus_bool_t dont_force = 0;
	dbus_message_append_args(msg,
		DBUS_TYPE_BOOLEAN, &dont_force,
		DBUS_TYPE_INVALID);

	send_system_method_call(msg);
}
void service_notify_dbus_firmware()
{
	DBusMessage* msg = dbus_message_new_method_call(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        "RebootWithFlags");

	dbus_bool_t dont_force = 0;
	uint64_t firmware_flag = 1;
	dbus_message_append_args(msg,
		DBUS_TYPE_BOOLEAN, &dont_force,
		DBUS_TYPE_UINT64, &firmware_flag,
		DBUS_TYPE_INVALID);

	send_system_method_call(msg);
}
