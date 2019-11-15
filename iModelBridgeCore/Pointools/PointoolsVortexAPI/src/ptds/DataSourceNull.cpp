/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PointoolsVortexAPIInternal.h"

#include <ptds/DataSourceNull.h>

namespace ptds
{

ptds::DataSourcePtr DataSourceNull::createNew(const FilePath *path)
{
	return new DataSourceNull;
}


DataSource::DataSourceType DataSourceNull::getDataSourceType(void)
{
	return DataSource::DataSourceTypeNull;
}


DataSource::DataSourceForm DataSourceNull::getDataSourceForm(void)
{
	return DataSource::DataSourceFormNULL;
}

} // End ptds namespace
