#include <pt/os.h>
#include <pt/trace.h>

namespace detail
{
pt::TraceContext *_tracecontext=0;
std::ofstream *__logfs = 0;
boost::mutex __mutex;
}
using namespace detail;

pt::TraceContext * pt::tracecontext()
{
	if (!_tracecontext) _tracecontext = new TraceContext();
	return _tracecontext;
}

std::ofstream &pt::FileTrace::file() 
{
	if (!__logfs)
	{
		__logfs = new std::ofstream();

		char trace_path[260];
		char full_path[260];
		int success = ::GetEnvironmentVariableA("VORTEX_TRACE_PATH", trace_path, 260);
		
		if (success)
		{
			sprintf(full_path, "%s\\trace.txt", trace_path);
			__logfs->open(full_path);
		}
		else
			__logfs->open("trace.txt");
	}
	return *__logfs;
}
boost::mutex &pt::FileTrace::mutex()
{
	return __mutex;
}

