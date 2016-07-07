#pragma once


class DataSourceStoreConfig
{
protected:

	enum AccessType
	{
		Type_NULL,

		Type_Normal,
		Type_Random_Access
	};

	enum DataDivision
	{
		Size_NULL,

		Size_Discrete,
		Size_Large
	};

protected:

	AccessType				accessType;
	DataDivision			dataDivision;

public:

};