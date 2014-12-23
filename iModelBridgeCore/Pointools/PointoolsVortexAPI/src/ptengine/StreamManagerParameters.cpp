#include <ptengine/StreamManagerParameters.h>

namespace pointsengine
{


void StreamManagerParameters::setStreamBudgetPerVoxel(DataSource::DataSize budget)
{
	streamBudgetPerVoxel = budget;	
}


DataSource::DataSize StreamManagerParameters::getStreamBudgetPerVoxel(void)
{
	return streamBudgetPerVoxel;
}


void StreamManagerParameters::setStreamDataSourceStreamHost(StreamHost *host)
{
	streamHost = host;
}


StreamHost *StreamManagerParameters::getStreamDataSourceStreamHost(void)
{
	return streamHost;
}


void StreamManagerParameters::setStreamBudgetTotal(DataSource::DataSize size)
{
	streamBudgetTotal = size;
}


DataSource::DataSize StreamManagerParameters::getStreamBudgetTotal(void)
{
	return streamBudgetTotal;
}


void StreamManagerParameters::setStreamBudgetIteration(DataSource::DataSize budget)
{
	streamBudgetIteration = budget;
}


DataSource::DataSize StreamManagerParameters::getStreamBudgetIteration(void)
{
	return streamBudgetIteration;
}

void StreamManagerParameters::setStreamHostForm(DataSource::DataSourceForm form)
{
	streamHostForm = form;
}


DataSource::DataSourceForm StreamManagerParameters::getStreamHostForm(void)
{
	return streamHostForm;
}

void StreamManagerParameters::setStreamPeriodTotal(StreamHost::StreamTime initStreamPeriodTotal)
{
	streamPeriodTotal = initStreamPeriodTotal;
}

StreamHost::StreamTime StreamManagerParameters::getStreamPeriodTotal(void)
{
	return streamPeriodTotal;
}


} // End pointsengine namespace


