#include <pt/os.h>
#include <pt/trace.h>
#include <ptapi/PointoolsVortexAPI.h>

namespace detail
{
pt::TraceContext *_tracecontext=0;
std::wofstream *__logfs = 0;
boost::mutex __mutex;
}
using namespace detail;

pt::TraceContext * pt::tracecontext()
{
	if (!_tracecontext) _tracecontext = new TraceContext();
	return _tracecontext;
}

std::wofstream &pt::FileTrace::file() 
{
	if (!__logfs)
	{
		__logfs = new std::wofstream();

		wchar_t trace_path[260];
		wchar_t full_path[260];
		int success = ::GetEnvironmentVariableW(L"VORTEX_TRACE_PATH", trace_path, 260);
		
		if (success)
		{
			swprintf(full_path, L"%s\\trace.txt", trace_path);
			__logfs->open(full_path);

			(*__logfs) << ptGetVersionString() << " Log File" << std::endl;

			// timestamp
			time_t rawtime;
			struct tm * timeinfo;
			char time_buffer[80];

			time (&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(time_buffer,80,"%d-%m-%Y %I:%M:%S",timeinfo);

			(*__logfs) << "Generated " << time_buffer << std::endl;

			std::wcout << L"Logging to " << full_path << std::endl;
		}
		else
		{
			std::cout << "Log path not found" << std::endl;
#ifndef _DEBUG
			exit(0);
#else
			__logfs->open("vortex_log.txt");
#endif
		}
	}
	return *__logfs;
}
boost::mutex &pt::FileTrace::mutex()
{
	return __mutex;
}

