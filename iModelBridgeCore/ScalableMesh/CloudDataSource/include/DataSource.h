/*--------------------------------------------------------------------------------------+
|
|     $Source: CloudDataSource/include/DataSource.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once


// ***********************************************************************************************************************************************
// Includes
// ***********************************************************************************************************************************************

#include <string>
#include "DataSourceDefs.h"
#include "DataSourceURL.h"
#include "DataSourceStatus.h"
#include "DataSourceLocator.h"
#include "DataSourceMode.h"
#include "DataSourceBuffer.h"


// ***********************************************************************************************************************************************
// Opaque types
// ***********************************************************************************************************************************************

class DataSourceService;
class DataSourceAccount;
class DataSourceStoreConfig;


// ***********************************************************************************************************************************************
// @bsiclass    :	DataSource
// Description	:	Base class for abstracted data sources for file or cloud access.
// Author		:	Lee Bull
// Date			:	18 Feb 2016
// Notes		:	
//
// ***********************************************************************************************************************************************


class DataSource : public DataSourceLocator
{

public:

	typedef unsigned long long int				DataPtr;
	typedef unsigned long long int				DataSize;
	typedef unsigned char 						Buffer;

	typedef	DataSourceBuffer::TimeoutStatus		TimeoutStatus;
	typedef DataSourceBuffer::Timeout			Timeout;

	typedef std::wstring						Name;

protected:

	Timeout										timeout;

protected:

	void										setStoreConfig		(DataSourceStoreConfig *newConfig);
	DataSourceStoreConfig					*	getStoreConfig		(void);

public:
CLOUD_EXPORT
												DataSource			(DataSourceAccount *sourceAccount);
CLOUD_EXPORT	virtual						   			   ~DataSource			(void);

CLOUD_EXPORT	DataSourceService						*	getService			(void);

CLOUD_EXPORT	virtual			DataSourceStatus			open				(const DataSourceURL & sourceURL, DataSourceMode sourceMode);
CLOUD_EXPORT	virtual			DataSourceStatus			close				(void) = 0;

CLOUD_EXPORT	virtual			DataSourceStatus			read				(Buffer *dest,   DataSize destSize, DataSize &readSize, DataSize size = 0) = 0;
CLOUD_EXPORT	virtual			DataSourceStatus			write				(Buffer *source, DataSize size) = 0;

CLOUD_EXPORT	virtual			bool						isValid				(void);

CLOUD_EXPORT	virtual			void						setTimeout			(Timeout timeMilliseconds);
CLOUD_EXPORT	virtual			Timeout						getTimeout			(void);

CLOUD_EXPORT	virtual			void						setCachingEnabled	(bool enabled);
CLOUD_EXPORT	virtual			bool						getCachingEnabled	(void);
};