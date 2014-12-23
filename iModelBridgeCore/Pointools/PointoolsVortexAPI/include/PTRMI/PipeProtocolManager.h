#pragma once 

#include <map>

#include <PTRMI/PipeManager.h>


namespace PTRMI
{
	class Name;

	class PipeProtocolManager
	{

	protected:

		typedef std::map<PipeManager::PipeManagerName, PipeManager *>	NamePipeManagerMap;

		typedef PipeManager::PipeManagerName							PipeManagerName;

	protected:

		Mutex					mutex;

		NamePipeManagerMap		pipeManagers;

	protected:

		void					addPipeManager				(PipeManager *pipeManager);

		unsigned int			initializePipeManagers		(void);
		unsigned int			shutdownPipeManagers		(void);

		void					newPipeManager				(const PipeManagerName &managerName);
		void					deletePipeManager			(const PipeManagerName &managerName);

		Pipe			*		newPipe						(Stream &stream);


//		bool					isPipeAdded					(Pipe *pipe);

// No Calls:
//		Pipe			*		getPipe						(const URL &pipeName, PipeManager **pipeManager = NULL);
//		Pipe			*		getPipe						(const PTRMI::GUID &clientManager, PipeManager **manager = NULL);
//		Status					deletePipe					(const PTRMI::GUID &clientManager);
//		Status					discardPipe					(Pipe *pipe);

//		PipeManager		*		getPipeManager				(Pipe *pipe);


// New

		PipeManager *			getPipeManager				(Stream &stream, bool &pipeExist);

	public:
								PipeProtocolManager			(void);

		PipeManager		*		getPipeManager				(const PipeManagerName &managerName);
//		Status					addPipe						(Pipe *pipe, PipeManager *pipeManager);		// Loop Back Try to get rid of these
//		Status					removePipe					(Pipe *pipe);								// Loop Back
		
		// New

		Status					initializeStream			(Stream &stream);

//		Status					deletePipe					(const Name &hostName);
		Status					deletePipe					(Pipe *pipe);

		Status					beginSend					(Stream &stream);
		Status					endSend						(Stream &stream);
		Status					beginReceive				(Stream &stream);
		Status					endReceive					(Stream &stream);

		Status					sendMessage					(Stream &stream, Message::MessageType messageType);

		Status					waitForResult				(Stream &stream, PTRMI::Event &event);
		Status					signalEndMethod				(Stream &stream);
		Status					waitForEndMethod			(Stream &stream);
	};

} // End PTRMI namespace