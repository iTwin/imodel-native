#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/Message.h>
#include <PTRMI/DataBuffer.h>


namespace PTRMI
{

unsigned int Message::maxHeaderSize = 0;


Message::Message(void)
{
															// Calculate constant
	if(getMaxHeaderSize() == 0)
	{
		setMaxHeaderSize(calculateMaxHeaderSize());
	}
															// Set default value for total size (version 2)
	setMessageSizeTotal(0);
}


PTRMI::Status Message::sendCallHeader(DataBuffer &buffer)
{
	return sendCallHeader(getMessageVersion(), buffer, getStatus(), getSenderManager(), getSender(), getReceiver(), getMethodName());
}


PTRMI::Status Message::sendCallHeader(MessageVersion version, DataBuffer &buffer, Status status, const PTRMI::GUID &senderManagerV, const PTRMI::GUID &senderV, const PTRMI::Name &receiverV, const PTRMI::Name &methodNameV)
{
															// Record message version specified
	setMessageVersion(version);
															// Record the start of the header and whole message
	buffer.setMessageStartPtr(buffer.getWritePtr());
															// Record the end of the header block (Start of parameter block)
	buffer.setMessageParameterPtr(buffer.getWritePtr() + getMaxHeaderSize());

															// Send message version number
	buffer << version;
															// Record position of status so it can be updated later
	buffer.setMessageStatusPtr(buffer.getWritePtr());
															// Send PTRMI level status
	buffer << status;
															// Send message type
	buffer << static_cast<MessageTypeValue>(Message::MessageType_Call);

															// Send the sending system's Manager GUID
	buffer << senderManagerV;
															// Send GUID of sending interface
	buffer << senderV;
															// Send receiving server interface identification
															// This is the object name if the interace is unknown
															// or the GUID of the receiving interface
	buffer << receiverV;
															// Send identification of method to invoke
	buffer << methodNameV;
															// Record position of status so it can be updated later
	buffer.setMessageSizeTotalPtr(buffer.getWritePtr());
															// Send total message size, including this header
	if(version >= 2)
	{
		buffer << messageSizeTotal;
	}

															// Pad header to end of header block and start of parameter block
	if(buffer.setWriteParameters().isFailed())
	{
		return Status(Status::Status_Error_Send_Header_Failed);
	}
															// If buffer is valid
	if(buffer.isValid())
	{
															// Return OK
		return Status(Status::Status_OK);
	}

															// Otherwise return receive header failure
	return Status(Status::Status_Error_Send_Header_Failed);
}


PTRMI::Status Message::sendReturnHeader(DataBuffer &buffer)
{
	return sendReturnHeader(getMessageVersion(), buffer, getStatus(), getSenderManager(), getSender(), getReceiver());
}


PTRMI::Status Message::sendReturnHeader(MessageVersion version, DataBuffer &buffer, Status status, const PTRMI::GUID &senderManagerV, const PTRMI::GUID &senderV, const PTRMI::Name &receiverV)
{
															// Record message version specified
	setMessageVersion(version);
															// Record the start of the header and whole message
	buffer.setMessageStartPtr(buffer.getWritePtr());
															// Record the end of the header block (Start of parameter block)
	buffer.setMessageParameterPtr(buffer.getWritePtr() + getMaxHeaderSize());

															// Send message version number
	buffer << version;
															// Record position of status so it can be over-ridden later in the buffer
	buffer.setMessageStatusPtr(buffer.getWritePtr());
															// Write given status
	buffer << status;
															// Send message type
	buffer << static_cast<MessageTypeValue>(Message::MessageType_Return);
															// Send the sending system's Manager GUID
	buffer << senderManagerV;
															// Send GUID of sending interface
	buffer << senderV;
															// Send receiving server interface identification
															// This is the object name if the interace is unknown
															// or the GUID of the receiving interface
	buffer << receiverV;
															// Record position of message size total so it can be over-ridden later in the buffer
	buffer.setMessageSizeTotalPtr(buffer.getWritePtr());
															// Send total message size, including this header
	if(version >= 2)
	{
		buffer << messageSizeTotal;
	}
															// Check header size is within spec
	if(buffer.getWritePtr() > getMaxHeaderSize())
	{
		return Status(Status::Status_Error_Send_Header_Size_Too_Large);
	}
															// Pad header to end of header block and start of parameter block
	if(buffer.setWriteParameters().isFailed())
	{
		return Status(Status::Status_Error_Send_Header_Failed);
	}
															// If buffer is valid
	if(buffer.isValid())
	{
															// Return OK
		return Status(Status::Status_OK);
	}

															// Otherwise return receive header failure
	return Status(Status::Status_Error_Send_Header_Failed);

}

PTRMI::Status Message::receiveHeader(DataBuffer &buffer)
{
															// Record the start of the header block and whole message
	buffer.setMessageStartPtr(buffer.getReadPtr());
															// Record the end of the header block (Start of parameter block)
	buffer.setMessageParameterPtr(buffer.getReadPtr() + getMaxHeaderSize());

															// Get message version number
	buffer >> messageVersion;

	buffer >> status;
															// Get type of message
	buffer >> messageType;
															// Get the sending system's Manager GUID
	buffer >> senderManager;
															// Get the sending interface's GUID
	buffer >> sender;
															// Get receiver interface's GUID OR URL to object
	buffer >> receiver;
															// Receive method name if header is a call
	if(messageType == MessageType_Call)
	{
															// Receive name of method to invoke
		buffer >> methodName;
	}
															// Read total message size if Message Version 2 or greater
	if(messageVersion >= 2)
	{
		buffer >> messageSizeTotal;
	}
															// Check header is within spec (should always be)
	if(buffer.getReadPtr() > getMaxHeaderSize())
	{
		return Status(Status::Status_Error_Receive_Header_Size_Too_Large);
	}

															// Skip header padding and Move to parameter block
	if(buffer.setReadParameters().isFailed())
	{
		return Status(Status::Status_Error_Receive_Header_Failed);
	}


	if(buffer.isValid())
	{
															// Return OK
		return Status(Status::Status_OK);
	}

															// Otherwise return receive header failure
	return Status(Status::Status_Error_Receive_Header_Failed);
}

void Message::set(Status initStatus, Message::MessageType type, const PTRMI::GUID &initSenderManager, const PTRMI::GUID &initSender, const PTRMI::Name &initReceiver, const PTRMI::Name &initMethodName)
{
	setStatus(initStatus);

	setMessageType(type);

	setSenderManager(initSenderManager);

	setSender(initSender);

	setReceiver(initReceiver);

	setMethodName(initMethodName);
}

void Message::setSenderManager(const PTRMI::GUID &initSenderManager)
{
	senderManager = initSenderManager;
}

const PTRMI::GUID & Message::getSenderManager(void)
{
	return senderManager;
}

void Message::setSender(const PTRMI::GUID &initSender)
{
	sender = initSender;
}

const PTRMI::GUID & Message::getSender(void)
{
	return sender;
}

void Message::setReceiver(const PTRMI::Name &initReceiver)
{
	receiver = initReceiver;
}

const PTRMI::Name & Message::getReceiver(void)
{
	return receiver;
}

void Message::setMethodName(const PTRMI::Name &initMethodName)
{
	methodName = initMethodName;
}

const PTRMI::Name & Message::getMethodName(void)
{
	return methodName;
}


unsigned int Message::calculateMaxHeaderSize(void)
{
	unsigned int maxSize;
															// Calculate maximum existing requirement (Versions 1 and 2)
	maxSize = static_cast<int>(sizeof(messageVersion)			+
				                status.getMaxWriteSize()		+
				                sizeof(messageType)				+
				                senderManager.getMaxWriteSize()	+
				                sender.getMaxWriteSize()		+
				                receiver.getMaxWriteSize()		+
				                methodName.getMaxWriteSize(PTRMI_MESSAGE_MAX_METHOD_NAME_SIZE) +
				                sizeof(MessageSize));
															// Increase to add reserved space if possible
	maxSize = std::max(maxSize, PTRMI_MESSAGE_PREFERRED_HEADER_SIZE);

	return maxSize;
}

void Message::setMaxHeaderSize(unsigned int maxSize)
{
	maxHeaderSize = maxSize;
}

unsigned int Message::getMaxHeaderSize(void)
{
	return maxHeaderSize;
}


void Message::setMessageVersion(MessageVersion version)
{
	messageVersion = version;
}


Message::MessageVersion Message::getMessageVersion(void)
{
	return messageVersion;
}


void Message::setMessageType(MessageType type)
{
	messageType = type;
}


Message::MessageTypeValue Message::getMessageTypeValue(void)
{
	return messageType;
}


Message::MessageType Message::getMessageType(void)
{
	return static_cast<MessageType>(getMessageTypeValue());
}


void Message::setStatus(Status initStatus)
{
	status = initStatus;
}


Status Message::getStatus(void)
{
	return status;
}


bool Message::setUpdatedStatus(DataBuffer &buffer, Status initStatus)
{
	void *statusAddress;
	
	if((statusAddress = buffer.getPtrAddress(buffer.getMessageStatusPtr())) != NULL)
	{
		initStatus.write(statusAddress);
		return true;
	}

	return false;
}


bool Message::setUpdatedMessageSizeTotal(DataBuffer &buffer, DataSize initSize)
{
	void *messageSizeAddress;

	if((messageSizeAddress = buffer.getPtrAddress(buffer.getMessageSizeTotalPtr())) != NULL)
	{
		if(buffer.setValue(buffer.getMessageSizeTotalPtr(), initSize).isOK())
		{
			return true;
		}
	}

	return false;
}


void Message::setMessageSizeTotal(MessageSize size)
{
	messageSizeTotal = size;
}


Message::MessageSize Message::getMessageSizeTotal(void)
{
	return messageSizeTotal;
}


Message::MessageSize Message::updateMessageSizeTotal(DataBuffer &buffer)
{
															// Only update if message is version 2 or above
	if(getMessageVersion() >= 2)
	{
		MessageSize	size = buffer.getWritePtr() - buffer.getMessageStartPtr();

		setUpdatedMessageSizeTotal(buffer, size);

		return size;
	}

	return 0;
}

Message::MessageVersion Message::getNullMessageVersion(void)
{
															// Return a NULL (Illegal message version number)
	return PTRMI_NULL_MESSAGE_HEADER_VERSION;
}

Message::MessageVersion Message::getDefaultMessageVersion(void)
{
															// Return the default base version number
	return PTRMI_DEFAULT_MESSAGE_HEADER_VERSION;
}


Message::MessageVersion Message::getCurrentMessageVersion(void)
{
															// Return the highest supported message version in this API release
	return PTRMI_CURRENT_MESSAGE_HEADER_VERSION;
}


Message::MessageVersion Message::resolveMessageVersions(MessageVersion versionA, MessageVersion versionB)
{
															// If either versions invalid for some reason, drop to default base version
	if(versionA == getNullMessageVersion() || versionB == getNullMessageVersion())
	{
		return getDefaultMessageVersion();
	}
															// Message version to be used is the lowest version number
	return std::min(versionA, versionB);
}

} // End PTRMI namespace