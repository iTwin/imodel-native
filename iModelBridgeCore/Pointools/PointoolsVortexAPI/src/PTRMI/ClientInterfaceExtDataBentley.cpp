
#include <PTRMI/ClientInterfaceExtDataBentley.h>
#include <PTRMI/Status.h>

namespace PTRMI
{

const ClientInterfaceExtDataBentley::FileHeader ClientInterfaceExtDataBentley::CLIENT_INTERFACE_EXT_DATA_BENTLEY_HEADER = 0x504D54444F505450;




ClientInterfaceExtDataBentley::ClientInterfaceExtDataBentley(void)
{
															// Assign a unique GUID to the object (May be over-written on read)
	generateFileGUID();
}

PTRMI::Status ClientInterfaceExtDataBentley::set(const wchar_t *originalServerFile, void *bentleyData, unsigned int bentleyDataSize)
{
	Status	status;
															// Set the original server file path
	if((status = setOriginalServerFile(URL(originalServerFile))).isFailed())
		return status;
															// Copy the given Bentley data block
	if((status = setBentleyData(reinterpret_cast<DataBuffer::Data *>(bentleyData), bentleyDataSize)).isFailed())
		return status;
															// Return OK
	return Status();
}


PTRMI::Status ClientInterfaceExtDataBentley::readFile(const wchar_t *filepath)
{
	DataBuffer		buffer;
	Status			status;
															// Open file and read header
	if((status = readHeader(filepath, buffer)).isFailed())
	{
		return status;
	}
															// Read Bentley Data
															// IMPORTANT !! The BentleyData position in the file MUST NOT MOVE !!
	if((status = readBentleyData(buffer)).isFailed())
	{
		return status;
	}
															// Read original server file path
	originalServerFile.read(buffer);

															// Return OK
	return Status();
}


PTRMI::Status ClientInterfaceExtDataBentley::readHeader(const wchar_t *filepath, DataBuffer &buffer)
{
															// Set up buffer (Note: Switch to dynamic buffer sizing soon)
	buffer.createInternalBuffer(CLIENT_INTERFACE_EXT_DATA_BENTLEY_MAX_SIZE);

	Status status;

															// Read whole file into bufffer for parsing
															// or just up to the limit of the max data size
	if((status = buffer.readFileToBuffer(filepath, true)).isFailed())
	{
		Status::log(L"ClientInterfaceExtDataBentley::readHeader failed to read", filepath);
		return status;
	}

	FileHeader		header;
	FileVersion		fileVersion;
	PTRMI::GUID		guid;
															// Get header
	buffer >> header;
															// Check header matches. If not, it's not a POD client ext file
	if(header != CLIENT_INTERFACE_EXT_DATA_BENTLEY_HEADER)
	{
		return Status(Status::Status_File_Not_Ext_Data);
	}

															// Get file version
	buffer >> fileVersion;

	if(fileVersion > CLIENT_INTERFACE_EXT_DATA_CURRENT_VERSION)
	{
		return Status(Status::Status_Error_File_Ext_Data_Newer_Version);
	}

															// Get the file GUID
	buffer >> guid;

	if(guid.isValidGUID() == false)
	{
		return Status(Status::Status_Error_Invalid_GUID);
	}
															// Set the GUID
	setFileGUID(guid);

															// Return OK
	return Status();
}


PTRMI::Status ClientInterfaceExtDataBentley::writeHeader(DataBuffer &buffer)
{
															// Write header to buffer
	buffer << CLIENT_INTERFACE_EXT_DATA_BENTLEY_HEADER;
															// Write current version to buffer
	buffer << CLIENT_INTERFACE_EXT_DATA_CURRENT_VERSION;
															// Write the file GUID
	buffer << (*getFileGUID());
															// Return OK
	return Status();
}


PTRMI::Status ClientInterfaceExtDataBentley::readBentleyData(DataBuffer &buffer)
{
	DataBuffer::DataSize	dataSize;
	Status					status;
															// Read the size of the data block
	buffer >> dataSize;
															// Allocate buffer
	if((status = bentleyData.createInternalBuffer(dataSize)).isFailed())
		return status;
															// Copy data from read buffer to bentleyData buffer
	if((status = bentleyData.WriteToBufferFromBuffer(buffer, dataSize)).isFailed())
		return status;
															// Return OK
	return Status();
}


PTRMI::Status ClientInterfaceExtDataBentley::writeFile(const wchar_t *filepath)
{
	Status			status;
	DataBuffer		buffer;
															// Set up buffer (Note: Switch to dynamic buffer sizing soon)
	buffer.createInternalBuffer(CLIENT_INTERFACE_EXT_DATA_BENTLEY_MAX_SIZE);
															// Write the file header
	if((status = writeHeader(buffer)).isFailed())
		return status;
															// Copy Bentley data to output buffer
	if((status = writeBentleyData(buffer)).isFailed())
		return status;
															// Write the original server file path
	originalServerFile.write(buffer);

															// Write whole final buffer to file
	if((status = buffer.writeFileFromBuffer(filepath)).isFailed())
		return status;
															// Return status
	return Status();
}


PTRMI::Status ClientInterfaceExtDataBentley::writeBentleyData(DataBuffer &buffer)
{
	DataBuffer::DataSize	dataSize;
	Status					status;
															// Go to start of bentley data
	bentleyData.resetRead();
															// Get size of data to write
	dataSize = bentleyData.getDataSize();
															// Read the size of the data block
	buffer << dataSize;
															// Copy data from read buffer to bentleyData buffer
	if((status = buffer.WriteToBufferFromBuffer(bentleyData, bentleyData.getInternalBufferSize())).isFailed())
		return status;
															// Return OK
	return Status();
}


Status ClientInterfaceExtDataBentley::setOriginalServerFile(URL &filepath)
{
															// Check if file name is defined
	if(filepath.getLength() > 0)
	{
															// Set original server filepath
		originalServerFile = filepath;
		return Status();
	}
															// Return bad filename
	return Status(Status::Status_Error_File_Name_Bad);
}


Status ClientInterfaceExtDataBentley::getOriginalServerFile(URL &filepath)
{
															// If it is, copy and return
	filepath = originalServerFile;
															// Return bad filename
	return Status();
}


PTRMI::Status ClientInterfaceExtDataBentley::setBentleyData(DataBuffer::Data *data, DataBuffer::DataSize dataSize)
{
	Status	status;
															// Check defined buffer is given
	if(data == NULL || dataSize == 0)
		return Status(Status::Status_Error_Bad_Parameter);
															// Create buffer for data
	if((status = bentleyData.createInternalBuffer(dataSize)).isFailed())
		return status;
															// Read given data into buffer
	if((bentleyData.writeToBuffer(data, dataSize)) != dataSize)
		return Status(Status::Status_Error_Failed_To_Set_Client_Interface_Ext_Data);

															// Return OK
	return Status();
}


DataBuffer::Data *ClientInterfaceExtDataBentley::getBentleyData(void)
{
	return bentleyData.getBuffer();
}


DataBuffer::Data *ClientInterfaceExtDataBentley::getExtData(DataBuffer::DataSize *size)
{
	*size = getBentleyDataSize();
															// Bentley data is the ext data payload
	return getBentleyData();
}


DataBuffer::DataSize ClientInterfaceExtDataBentley::getBentleyDataSize(void)
{
															// Size is the total size of the buffer
	return bentleyData.getInternalBufferSize();
}


DataBuffer::DataSize ClientInterfaceExtDataBentley::getExtDataSize(void)
{
															// Ext data size is the bentley data payload
	return getBentleyDataSize();
}

void ClientInterfaceExtDataBentley::setFileGUID(PTRMI::GUID &guid)
{
															// Set the unique file GUID
	fileGUID = guid;
}


PTRMI::GUID *ClientInterfaceExtDataBentley::getFileGUID(void)
{
															// Return the unique file GUID
	return &fileGUID;
}


void ClientInterfaceExtDataBentley::generateFileGUID(void)
{
															// Generate GUID
	fileGUID.generate();
}


} // End PTRMI namespace

