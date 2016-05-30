#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/Status.h>

#ifndef NO_DATA_SOURCE_SERVER
//#include <PTRMI/Manager.h>
#endif

namespace PTRMI
{

bool			Status::logEnabled = false;
std::wstring	Status::logFile;
pt::SimpleTimer	Status::timer;

void Status::set(Error value, bool report)
{
	error = value;

	if(report)
	{
		if(isFailed() && getLogEnabled())
		{
			wchar_t	message[512];

			swprintf(message, sizeof(message) / sizeof(message[0]), L"Status = %d", (int) get());
			log(L"Status Error : ", message);
		}
	}
}


void Status::log(const wchar_t *message1, const wchar_t *message2)
{
	if(getLogEnabled() == false)
		return;

	writeTime();

#if defined (NEEDS_WORK_VORTEX_DGNDB)
	std::wofstream out;
	out.open(logFile.c_str(), std::ios::out | std::ios::app);
	out << "Status : " << message1 << " " << message2 << std::endl;
	out.flush();
	out.close();
#endif
}

void Status::log(const wchar_t *message1, uintptr_t const& value)
{
	if(getLogEnabled() == false)
		return;

	writeTime();

#if defined (NEEDS_WORK_VORTEX_DGNDB)
	std::wofstream out;
	out.open(logFile.c_str(), std::ios::out | std::ios::app);
	out << "Status : " << message1 << " " << value << std::endl;
	out.flush();
	out.close();
#endif
}

#ifndef NO_DATA_SOURCE_SERVER

Status Status::read(PTRMI::DataBuffer &buffer)
{
	ErrorType	readError = Status_Error_Failed;

	buffer >> readError;

	set(static_cast<Error>(readError));

	return Status();
}


Status Status::write(PTRMI::DataBuffer &buffer) const
{
	buffer << static_cast<ErrorType>(get());

	return Status();
}

#endif

Status Status::write(void *buffer) const
{
	if(buffer)
	{
		*static_cast<ErrorType *>(buffer) = static_cast<ErrorType>(get());
		return Status();
	}

	return Status(Status::Status_Error_Bad_Parameter);
}


unsigned int Status::getMaxWriteSize(void)
{
															// Return maximum size of marshalled status
	return sizeof(ErrorType);
}

void Status::writeTime(void)
{
#if defined (NEEDS_WORK_VORTEX_DGNDB)
	double t = timer.getEllapsedTimeSeconds();

	std::wofstream out;
	out.open(logFile.c_str(), std::ios::out | std::ios::app);
	out << "T = " << t;
	out << L" : ";
	out.flush();
	out.close();
#endif
}


} // End PTRMI namespace
