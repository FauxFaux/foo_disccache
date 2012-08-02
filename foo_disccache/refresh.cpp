#include "stdafx.h"
#include "refresh.h"

void global_refresh_callback::callback_run() {
	MessageBox(NULL, L"Hi!", L"Hi!", 0);
}
