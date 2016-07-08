#pragma once
#include "DataSourceDefs.h"

class DataSourceStatus
{
public:

	enum DataSourceStatusCode
	{
		Status_NULL,
		Status_OK,
		Status_Error,
		Status_Error_Not_Initialized,

		Status_Error_Bad_Parameters,
		Status_Error_Memory_Allocation,

		Status_Error_Not_File_Path,

		Status_Error_Not_Found,
		Status_Error_Not_Supported,
		Status_Error_Not_Open,
		Status_Error_Read,
		Status_Error_Write,
		Status_Error_EOF,
		Status_Error_Dest_Buffer_Too_Small,
		Status_Error_Seek,

		Status_Error_Unknown_Service,
		Status_Error_Service_Exists,
		Status_Error_Failed_To_Initialize_Subsystem,

		Status_Error_Failed_To_Upload,
		Status_Error_Failed_To_Download,

		Status_Error_Account_Not_Found,
		Status_Error_Failed_To_Create_DataSource,
		Status_Not_Found,

		Status_Error_Test_Failed
	};


protected:

		DataSourceStatusCode	code;

public:

CLOUD_EXPORT								DataSourceStatus			(void);
CLOUD_EXPORT								DataSourceStatus			(const DataSourceStatus &otherStatus);
CLOUD_EXPORT								DataSourceStatus			(DataSourceStatusCode code);

CLOUD_EXPORT		void					set							(DataSourceStatusCode newCode);
CLOUD_EXPORT		void					set							(const DataSourceStatus &newStatus);
CLOUD_EXPORT		DataSourceStatusCode	getCode						(void) const;

CLOUD_EXPORT		bool					isOK						(void) const;
CLOUD_EXPORT		bool					isFailed					(void) const;
};


inline DataSourceStatus::DataSourceStatus(void)
{
	set(Status_OK);
}


inline DataSourceStatus::DataSourceStatus(const DataSourceStatus & otherStatus)
{
	set(otherStatus);
}


inline DataSourceStatus::DataSourceStatus(DataSourceStatusCode code)
{
	set(code);
}


inline void DataSourceStatus::set(const DataSourceStatus &newStatus)
{
	set(newStatus.getCode());
}

inline void DataSourceStatus::set(DataSourceStatusCode newCode)
{
	code = newCode;
}


inline DataSourceStatus::DataSourceStatusCode DataSourceStatus::getCode(void) const
{
	return code;
}


inline bool DataSourceStatus::isOK(void) const
{
	return getCode() == Status_OK;
}


inline bool DataSourceStatus::isFailed(void) const
{
	return (isOK() == false);
}
