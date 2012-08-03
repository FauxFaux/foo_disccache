#include "stdafx.h"
#include "refresh.h"

static const int MAX_ITEMS = 50;
static const DWORD MEGABYTES = 1024 * 1024;
static const DWORD MAX_BYTES = 200 * MEGABYTES;
static const DWORD FREQUENCY_SECONDS = 60;

// "Default" {BFC61179-49AD-4E95-8D60-A22706485505}
static GUID const ORDER_DEFAULT=
{ 0xBFC61179, 0x49AD, 0x4E95, { 0x8D, 0x60, 0xA2, 0x27, 0x06, 0x48, 0x55, 0x05 } };

// "Repeat (playlist)" {681CC6EA-60AE-4BF9-913B-BB5F4E864F2A}
static GUID const ORDER_REPEAT_PLAYLIST=
{ 0x681CC6EA, 0x60AE, 0x4BF9, { 0x91, 0x3B, 0xBB, 0x5F, 0x4E, 0x86, 0x4F, 0x2A } };

// "Repeat (track)" {4BF4B280-0BB4-4DD0-8E84-37C3209C3DA2}
static GUID const ORDER_REPEAT_TRACK=
{ 0x4BF4B280, 0x0BB4, 0x4DD0, { 0x8E, 0x84, 0x37, 0xC3, 0x20, 0x9C, 0x3D, 0xA2 } };

// "Shuffle (tracks)" {C5CF4A57-8C01-480C-B334-3619645ADA8B}
static GUID const ORDER_SHUFFLE_TRACKS=
{ 0xC5CF4A57, 0x8C01, 0x480C, { 0xB3, 0x34, 0x36, 0x19, 0x64, 0x5A, 0xDA, 0x8B } };

// "Shuffle (albums)" {499E0B08-C887-48C1-9CCA-27377C8BFD30}
static GUID const ORDER_SHUFFLE_ALBUMS=
{ 0x499E0B08, 0xC887, 0x48C1, { 0x9C, 0xCA, 0x27, 0x37, 0x7C, 0x8B, 0xFD, 0x30 } };

// "Shuffle (directories)" {83C37600-D725-4727-B53C-BDEFFE5F8DC7}
static GUID const ORDER_SHUFFLE_DIRECTORIES=
{ 0x83C37600, 0xD725, 0x4727, { 0xB5, 0x3C, 0xBD, 0xEF, 0xFE, 0x5F, 0x8D, 0xC7 } };

#define more() (paths.get_count() < MAX_ITEMS)

static void consume(const pfc::list_t<metadb_handle_ptr> &items, pfc::list_t<pfc::string8> &paths) {
	for (t_size i = 0; i < items.get_count() && more(); ++i)
		paths.add_item(items[i]->get_path());
}


static void add_range(pfc::list_t<pfc::string8> &paths, t_size start, t_size count) {
	static_api_ptr_t<playlist_manager> playlist_api;
	static_api_ptr_t<playback_control> playback_api;

	pfc::list_t<metadb_handle_ptr> items;
	bit_array_range mask(start, count);
	playlist_api->activeplaylist_get_items(items, mask);

	consume(items, paths);
}

static void add_queue_related_paths(pfc::list_t<pfc::string8> &paths) {
	static_api_ptr_t<playlist_manager> playlist_api;
	pfc::list_t<t_playback_queue_item> queue;
	playlist_api->queue_get_contents(queue);

	for (t_size i = 0; i < queue.get_count() && more(); ++i)
		paths.add_item(queue[i].m_handle->get_path());
}


static void add_playlist_related_tracks(pfc::list_t<pfc::string8> &paths) {
	static_api_ptr_t<playlist_manager> playlist_api;
	static_api_ptr_t<playback_control> playback_api;

	const GUID playback_order = 
		playlist_api->playback_order_get_guid(playlist_api->playback_order_get_active());

	metadb_handle_ptr curr_track;
	playback_api->get_now_playing(curr_track);

	if (!curr_track.is_empty())
		paths.add_item(curr_track->get_path());

	if (ORDER_REPEAT_TRACK == playback_order)
		return;

	add_queue_related_paths(paths);

	if (ORDER_SHUFFLE_TRACKS == playback_order)
		return;

	if (!more())
		return;

	if (curr_track.is_empty())
		return;

	if (ORDER_SHUFFLE_DIRECTORIES == playback_order) {
#if 0
		pfc::string8 curr = curr_track->get_path();
		curr.truncate(curr.find_last('\\'));
		paths.add_item(curr);
#endif
		return;
	}

	t_size index;
	if (!playlist_api->activeplaylist_find_item(curr_track, index))
		return;

	if (ORDER_DEFAULT == playback_order || ORDER_REPEAT_PLAYLIST == playback_order) {
		add_range(paths, index + 1, MAX_ITEMS - paths.get_count());
	}

	if (!more())
		return;

	if (ORDER_REPEAT_PLAYLIST == playback_order) {
		add_range(paths, 0, MAX_ITEMS - paths.get_count());
		return;
	}
}

DWORD WINAPI do_readfiles(void *dat) {
	global_refresh_callback *us = static_cast<global_refresh_callback*>(dat);
	while (true) {
		WaitForSingleObject(us->work_to_do_event, INFINITE);
		EnterCriticalSection(&us->list_cs);

		if (us->thread_done)
			return 0;

		us->process_paths();

		LeaveCriticalSection(&us->list_cs);
	}
}

pfc::string8 drop_prefix(pfc::string8 path) {
	path.remove_chars(0, strlen("file://"));
	return path;
}

static DWORD read_whole(const pfc::string8 &url) {
	pfc::string8 path = drop_prefix(url);
	HANDLE h = CreateFileA(path.toString(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	DWORD e = GetLastError();
	if (INVALID_HANDLE_VALUE == h)
		return 0;
	const size_t buf_size = 32 * 1024;
	char buf[buf_size];
	DWORD read;
	DWORD total = 0;
	while (ReadFile(h, &buf, buf_size, &read, NULL) && read)
		total += read;

	CloseHandle(h);

	return total;
}

static DWORD read_whole_if_on(const pfc::string8 &path) {
	pfc::string8 a("\\\\.\\");
	a += drop_prefix(path);
	HANDLE h = CreateFileA(a, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == h)
		return 0;

	BOOL on;
	const BOOL powerstate = GetDevicePowerState(h, &on);
	CloseHandle(h);

	if (powerstate && on)
		return read_whole(path);
	return 0;
}

/** always read the first and next file,
  * then read the remaining (up to MAX_BYTES)
  * if the devices are on 
  */
void global_refresh_callback::process_paths() {
	DWORD total = 0;
	const DWORD count = paths.get_count();
	if (0 == count)
		return;

	total += read_whole(paths[0]);
	if (count > 1)
		total += read_whole(paths[1]);

	for (t_size i = 2; i < paths.get_count() && total < MAX_BYTES; ++i) {
		total += read_whole_if_on(paths[i]);
	}
}

void global_refresh_callback::callback_run() {
	if (!TryEnterCriticalSection(&list_cs))
		return;
	const DWORD now = GetTickCount();
	const DWORD diff = now - last_run;
	if (diff > FREQUENCY_SECONDS * 1000) {
		paths.remove_all();
		add_playlist_related_tracks(paths);
		SetEvent(work_to_do_event);
		last_run = now;
	}
	LeaveCriticalSection(&list_cs);
}

global_refresh_callback::global_refresh_callback() {
	last_run = 0;
	thread_done = false;
	InitializeCriticalSection(&list_cs);
	work_to_do_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	thread = CreateThread(NULL, 0, &do_readfiles, this, 0, NULL);
}

service_impl_t<global_refresh_callback> g_refresh_callback;

void do_refresh() {
	g_refresh_callback.callback_run();
}
