#include "PointoolsVortexAPIInternal.h"
#include <ptcmdppe/cmdprogress.h>
#include <ptcmdppe/cmdstate.h>
#include <ptcmdppe/cmdoutput.h>
#include <windows.h>
#ifndef POINTOOLS_POD_API
#include <boost/thread/mutex.hpp>
#endif

using namespace ptapp;

namespace cmdppe_private
{
	CmdOutput *_output = 0;
#ifndef POINTOOLS_POD_API
	boost::mutex progress_mutex;
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
		boost::mutex::scoped_lock lock(progress_mutex);
#endif

		_output->progressReset();
		_output->progressMax(mx);
		_output->progress(st);
		_output->flush();
	}
	::SetCursor(LoadCursor(0, IDC_WAIT));
	/*register job*/ 
}
CmdProgress::~CmdProgress()
{
	if (_output)
	{
#ifndef POINTOOLS_POD_API
		boost::mutex::scoped_lock lock(progress_mutex);
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
		boost::mutex::scoped_lock lock(progress_mutex);
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
		boost::mutex::scoped_lock lock(progress_mutex);
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
		boost::mutex::scoped_lock lock(progress_mutex);
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
		boost::mutex::scoped_lock lock(progress_mutex);
#endif
		_output->progress(st);
		_output->flush();
	}
}