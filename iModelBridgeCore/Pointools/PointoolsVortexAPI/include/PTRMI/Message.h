#pragma once

#include <bitset>

#include <PTRMI/GUID.h>
#include <PTRMI/Name.h>
#include <PTRMI/Status.h>
#include <ptds/DataSource.h>

namespace PTRMI
{

const unsigned char PTRMI_NULL_MESSAGE_HEADER_VERSION		= 0;
const unsigned char PTRMI_DEFAULT_MESSAGE_HEADER_VERSION	= 1;	// Oldest version to default to.
const unsigned char PTRMI_CURRENT_MESSAGE_HEADER_VERSION	= 2;	// Current version.

const unsigned int	PTRMI_MESSAGE_MAX_METHOD_NAME_SIZE		= 64;

const unsigned int	PTRMI_MESSAGE_PREFERRED_HEADER_SIZE		= 512;


class Message
{

public:

	typedef unsigned short			MessageVersion;

	enum MessageType
	{
		MessageType_Call,
		MessageType_Return,

		MessageType_Count
	};

	typedef unsigned char			MessageTypeValue;

	typedef ptds::DataSize			MessageSize;

protected:

	MessageVersion					messageVersion;
	Status							status;
	MessageTypeValue				messageType;
	PTRMI::GUID						senderManager;
	PTRMI::GUID						sender;
	Name							receiver;
	Name							methodName;

	static unsigned int				maxHeaderSize;

	MessageSize						messageSizeTotal;

protected:

	void							setMaxHeaderSize			(unsigned int maxSize);
	unsigned int					calculateMaxHeaderSize		(void);

	MessageTypeValue				getMessageTypeValue			(void);

public:

									Message						(void);

	void							set							(Status initStatus, const Message::MessageType type, const PTRMI::GUID &senderManager, const PTRMI::GUID &sender, const PTRMI::Name &receiver, const PTRMI::Name &methodName);

	void							setStatus					(Status initStatus);
	Status							getStatus					(void);
	bool							setUpdatedStatus			(DataBuffer &buffer, Status initStatus);
	bool							setUpdatedMessageSizeTotal	(DataBuffer &buffer, DataSize initSize);

	MessageSize						updateMessageSizeTotal		(DataBuffer &buffer);

	void							setMessageVersion			(MessageVersion version);
	MessageVersion					getMessageVersion			(void);

	void							setMessageType				(MessageType type);
	MessageType						getMessageType				(void);

	void							setSenderManager			(const PTRMI::GUID &guid);
	const PTRMI::GUID			&	getSenderManager			(void);

	void							setSender					(const PTRMI::GUID &guid);
	const PTRMI::GUID			&	getSender					(void);

	void							setReceiver					(const PTRMI::Name &name);
	const PTRMI::Name			&	getReceiver					(void);

	void							setMethodName				(const PTRMI::Name &method);
	const PTRMI::Name			&	getMethodName				(void);

	void							setMessageSizeTotal			(MessageSize size);
	MessageSize						getMessageSizeTotal			(void);

	static unsigned int				getMaxHeaderSize			(void);

	Status							sendCallHeader				(DataBuffer &buffer);
	Status							sendCallHeader				(MessageVersion messageVersion, DataBuffer &buffer, Status status, const PTRMI::GUID &senderManager, const PTRMI::GUID &sender, const PTRMI::Name &receiver, const PTRMI::Name &methodName);

	Status							sendReturnHeader			(DataBuffer &buffer);
	Status							sendReturnHeader			(MessageVersion messageVersion, DataBuffer &buffer, Status status, const PTRMI::GUID &senderManager, const PTRMI::GUID &sender, const PTRMI::Name &receiver);

	Status							receiveHeader				(DataBuffer &buffer);

	static MessageVersion			getNullMessageVersion		(void);
	static MessageVersion			getDefaultMessageVersion	(void);
	static MessageVersion			getCurrentMessageVersion	(void);
	static MessageVersion			resolveMessageVersions		(MessageVersion versionA, MessageVersion versionB);
};

} // End PTRMI namespace



