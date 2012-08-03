#include "stdafx.h"
#include "refresh.h"

class play_callback_changes : public play_callback_static
{
	void on_playback_new_track(metadb_handle_ptr p_track) { 
		do_refresh();
	}

	void on_playback_stop(play_control::t_stop_reason p_reason) {
	}

	void on_playback_dynamic_info_track(const file_info & p_info) {
	}

	void on_playback_edited(metadb_handle_ptr p_track) {
		do_refresh();
	}

	void on_playback_starting(play_control::t_track_command p_command, bool p_paused) {
		do_refresh();
	}

	void on_playback_seek(double p_time) {
		do_refresh();
	}

	void on_playback_pause(bool p_state) {
	}

	void on_playback_dynamic_info(const file_info & p_info) {
	}

	void on_playback_time(double p_time) {
		do_refresh();
	}

	void on_volume_change(float p_new_val) {
	}

	unsigned int get_flags() { 
		return flag_on_playback_new_track 
			| flag_on_playback_edited 
			| flag_on_playback_starting 
			| flag_on_playback_seek 
			| flag_on_playback_time
			;
	}
};

static play_callback_static_factory_t<play_callback_changes> g_playback;
