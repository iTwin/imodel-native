#pragma once


#include "DataSourceURL.h"
#include "DataSourceMode.h"

class DataSourceService;
class DataSourceAccount;


class DataSourceLocator
{

protected:

	DataSourceURL							url;
	DataSourceService					*	service;
	DataSourceAccount					*	account;

	DataSourceURL							containerName;
	DataSourceURL							subPath;
	DataSourceURL							segmentName;

	DataSourceMode							mode;

public:

											DataSourceLocator		(void);
											DataSourceLocator		(DataSourceLocator &locator);

	void									setURL					(DataSourceURL &newURL);
	const DataSourceURL					&	getURL					(void);

	void									setService				(DataSourceService *newService);
	DataSourceService					*	getService				(void);

	void									setAccount				(DataSourceAccount *sourceAccount);
	DataSourceAccount					*	getAccount				(void) const;

	void									setContainerName		(DataSourceURL &name);
	const DataSourceURL					&	getContainerName		(void) const;

	void									setSubPath				(DataSourceURL &path);
	const DataSourceURL					&	getSubPath				(void) const;
	void									getFullSubPath			(DataSourceURL &dest) const;

	void									setSegmentName			(DataSourceURL &segmentName);
	const DataSourceURL					&	getSegmentName			(void) const;

	void									setMode					(DataSourceMode sourceMode);
	DataSourceMode							getMode					(void) const;

};


