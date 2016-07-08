#pragma once

#include "DataSourceDefs.h"
#include "DataSourceURL.h"
#include "DataSourceMode.h"

class DataSourceService;
class DataSourceAccount;


class DataSourceLocator
{

protected:

	DataSourceService					*	service;
	DataSourceAccount					*	account;

	DataSourceURL							prefixPath;
	DataSourceURL							subPath;
	DataSourceURL							segmentName;

	DataSourceMode							mode;


protected:

	void									setURL					(const DataSourceURL &newURL);
	void									setURL					(const DataSourceURL &prefix, DataSourceURL &subPath);

public:

CLOUD_EXPORT											DataSourceLocator		(void);
CLOUD_EXPORT											DataSourceLocator		(DataSourceLocator &locator);

CLOUD_EXPORT	void									getURL					(DataSourceURL &url);

CLOUD_EXPORT	void									setService				(DataSourceService *newService);
CLOUD_EXPORT	DataSourceService					*	getService				(void);

CLOUD_EXPORT	void									setAccount				(DataSourceAccount *sourceAccount);
CLOUD_EXPORT	DataSourceAccount					*	getAccount				(void) const;


CLOUD_EXPORT	void									setPrefixPath			(const DataSourceURL &path);
CLOUD_EXPORT	const DataSourceURL					&	getPrefixPath			(void);

CLOUD_EXPORT	void									setSubPath				(const DataSourceURL &path);
CLOUD_EXPORT	const DataSourceURL					&	getSubPath				(void);

CLOUD_EXPORT	void									setSegmentName			(DataSourceURL &segmentName);
CLOUD_EXPORT	const DataSourceURL					&	getSegmentName			(void) const;

//	void									getFullSubPath			(DataSourceURL &dest) const;

CLOUD_EXPORT	void									setMode					(DataSourceMode sourceMode);
CLOUD_EXPORT	DataSourceMode							getMode					(void) const;

};


