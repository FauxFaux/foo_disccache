#pragma once
#include "stdafx.h"

class global_refresh_callback : public main_thread_callback
{
public:
	void callback_run();
};
