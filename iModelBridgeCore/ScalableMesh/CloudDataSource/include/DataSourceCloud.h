#pragma once
#include "DataSourceDefs.h"
#include "DataSourceCached.h"


class DataSourceCloud : public DataSourceCached
{
protected:

	typedef DataSourceCached	Super;

public:

CLOUD_EXPORT			DataSourceCloud		(DataSourceAccount *sourceAccount);
};