#pragma once


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

											DataSourceLocator		(void);
											DataSourceLocator		(DataSourceLocator &locator);

	void									getURL					(DataSourceURL &url);

	void									setService				(DataSourceService *newService);
	DataSourceService					*	getService				(void);

	void									setAccount				(DataSourceAccount *sourceAccount);
	DataSourceAccount					*	getAccount				(void) const;


	void									setPrefixPath			(const DataSourceURL &path);
	const DataSourceURL					&	getPrefixPath			(void);

	void									setSubPath				(const DataSourceURL &path);
	const DataSourceURL					&	getSubPath				(void);

	void									setSegmentName			(DataSourceURL &segmentName);
	const DataSourceURL					&	getSegmentName			(void) const;

//	void									getFullSubPath			(DataSourceURL &dest) const;

	void									setMode					(DataSourceMode sourceMode);
	DataSourceMode							getMode					(void) const;

};


