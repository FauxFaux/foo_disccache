#pragma once
#include "stdafx.h"

DWORD WINAPI do_readfiles(void *dat);
void do_refresh();

class global_refresh_callback : public main_thread_callback
{
	HANDLE thread;
	pfc::list_t<pfc::string8> paths;
public:
	CRITICAL_SECTION list_cs;
	HANDLE work_to_do_event;
	bool thread_done;

	global_refresh_callback();
	void callback_run();
	void process_paths();
};

