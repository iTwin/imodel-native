
#pragma once

namespace PTRMI
{
	class Event
	{
	protected:

		HANDLE		event;

	public:

		Event(void)
		{
			if((event = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
			{
				Status status(Status::Status_Error_Failed_To_Create_Event);
			}
		}

		~Event(void)
		{
			if(event)
			{
				CloseHandle(event);
			}
		}

		bool wait(unsigned long timeout = INFINITE)
		{
			DWORD	result;

			result = WaitForSingleObject(event, timeout);

			switch(result)
			{
			case WAIT_OBJECT_0:

				return true;

			case WAIT_TIMEOUT:
			case WAIT_ABANDONED:

			default:

				return false;
			}
		}

		bool signal(void)
		{
			if(SetEvent(event))
			{
				return true;
			}

			return false;
		}

	};
}