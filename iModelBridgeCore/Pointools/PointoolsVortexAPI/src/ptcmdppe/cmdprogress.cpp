#include "PointoolsVortexAPIInternal.h"
#include <ptcmdppe/cmdprogress.h>
#include <ptcmdppe/cmdstate.h>
#include <ptcmdppe/cmdoutput.h>



using namespace ptapp;

namespace cmdppe_private
{
	CmdOutput *_output = 0;
#ifndef POINTOOLS_POD_API
	std::mutex progress_mutex;
#endif
}
using namespace cmdppe_private;

CmdStateIcon::CmdStateIcon(const char *id, int initial_state, int num_states, const char**imagepaths)
{
	_id = id;
	if (_output)
		_output->addStateIcon(id, initial_state, num_states, imagepaths);
}
CmdStateIcon::~CmdStateIcon()
{
	if (_output)
		_output->remStateIcon(_id);
}
void CmdStateIcon::state(int st)
{
	if (_output)
		_output->state(_id, st);
}

CmdProgress::CmdProgress(const char *st, int mn, int mx, bool dedicated_win)
{
	if (_output)
	{
#ifndef POINTOOLS_POD_API
        std::lock_guard<std::mutex> lock(progress_mutex);
#endif

		_output->progressReset();
		_output->progressMax(mx);
		_output->progress(st);
		_output->flush();
	}
#if defined (BENTLEY_WIN32) 
	::SetCursor(LoadCursor(0, IDC_WAIT));
#endif
	/*register job*/ 
}
CmdProgress::~CmdProgress()
{
	if (_output)
	{
#ifndef POINTOOLS_POD_API
        std::lock_guard<std::mutex> lock(progress_mutex);
#endif
	/*unregister job;*/ 
		_output->ready();
		_output->progressReset();
		_output->flush();
	}
}
void CmdProgress::inc(int amount)
{	
	if (_output)
	{	
#ifndef POINTOOLS_POD_API
        std::lock_guard<std::mutex> lock(progress_mutex);
#endif
		_output->progressInc(amount);
		_output->flush();
	}
}
void CmdProgress::inc()
{
	if (_output)
	{
#ifndef POINTOOLS_POD_API
        std::lock_guard<std::mutex>lock(progress_mutex);
#endif
		_output->progressInc();
		_output->flush();
	}
}
void CmdProgress::set(int amount)
{
	if (_output)
	{
#ifndef POINTOOLS_POD_API
        std::lock_guard<std::mutex> lock(progress_mutex);
#endif
		_output->progressSet(amount);
		_output->flush();
	}
}

void CmdProgress::status(const char* st)
{
	if (_output)
	{
#ifndef POINTOOLS_POD_API
        std::lock_guard<std::mutex> lock(progress_mutex);
#endif
		_output->progress(st);
		_output->flush();
	}
}