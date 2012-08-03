#include "stdafx.h"
#include "refresh.h"

class queuecontents_queue_callback : public playback_queue_callback {
public:
	virtual void on_changed(t_change_origin p_origin)
	{
		do_refresh();
	}
};

static service_factory_t< queuecontents_queue_callback > queuecontents_qcallback;
