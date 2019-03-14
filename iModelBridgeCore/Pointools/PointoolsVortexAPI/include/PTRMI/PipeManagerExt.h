#pragma once

#include <PTRMI/PipeManager.h>
#include <PTRMI/PipeExt.h>
#include <PTRMI/Name.h>
#include <PTRMI/DataBuffer.h>

namespace PTRMI
{


class PipeManagerExt : public PipeManager
{

public:

		typedef unsigned int			(*ExternalCallFunction)				(void *sendData, unsigned int sendDataSize, void *extData, unsigned int extDataSize, void **receiveData, unsigned int *receiveDataSize);
		typedef bool					(*ExternalReleaseBufferFunction)	(void *buffer);
		typedef unsigned int			(*ExternalProcessRequestCallback)	(void *dataSend, unsigned int dataSendSize, uint64_t clientIDFirst64, uint64_t clientIDSecond64);

		typedef unsigned long			ExternalErrorCode;

		enum ExtErrorCode
		{
			EXT_PIPE_ERROR_OK						= 0001,

			EXT_PIPE_ERROR_EXT_DATA_EMPTY			= 1003,		// PTW_ERROR_BENTLEYDATAEMPTY            
			EXT_PIPE_ERROR_COMMUNICATION_FAILURE	= 1004,		// PTW_ERROR_PWERROR                     
			EXT_PIPE_ERROR_BAD_DATA					= 1005,		// PTW_ERROR_GETDATAERROR
			EXT_PIPE_ERROR_DATA_OUT_OF_DATE			= 1006		// PTW_ERROR_FILEOUTDATED
		};

protected:

		ExternalCallFunction			externalCallFunction;
		ExternalReleaseBufferFunction	externalReleaseBufferFunction;

protected:

//	PipeExt							*	newPipe								(const PTRMI::GUID &pipeGUID);

	Status								getExternalErrorCodeStatus			(ExternalErrorCode);

public:

										PipeManagerExt						(const PipeManagerName &name);
									   ~PipeManagerExt						(void);

		Pipe						*	newPipe								(const Name &hostName);

		Status							deletePipe							(Pipe *pipe);

//		Pipe						*	newPipe								(Stream &stream, const URL &pipeHostAddress);
//		Pipe						*	newPipe								(Stream &stream, const PTRMI::GUID &hostGUID);

//		bool							deletePipe							(const URL &pipeHostAddress);
// 		bool							deletePipe							(const PTRMI::GUID &hostGUID);
// 		bool							deletePipe							(Pipe *pipe);

		void							setSendExternalCallFunction			(ExternalCallFunction function);
		ExternalCallFunction			getSendExternalCallFunction			(void);

		void							setReleaseExternalBufferFunction	(ExternalReleaseBufferFunction function);
		ExternalReleaseBufferFunction	getReleaseExternalBufferFunction	(void);

		Status							sendExternalCall					(Stream &stream);
		Status							receiveExternalCall					(void *receive, unsigned int receiveSize, void **send, unsigned int *sendSize, PTRMI::GUID *guid, ExternalProcessRequestCallback externalProcessRequestCallback = NULL);
		Status							receiveExternalCallCB				(void *receive, unsigned int receiveSize, PTRMI::GUID *clientManager, ExternalProcessRequestCallback externalProcessRequestCallback);

		void							releaseExternalBuffer				(PipeExt &pipeExt);

};

} // End PTRMI namespace


