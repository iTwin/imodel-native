//----------------------------------------------------------------------------
//
// ExtentsQueryTest.h
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#pragma once

#include "VortexExampleApp.h"


class ExtentsQueryTest
{

public:

	enum QueryType
	{
		QueryTypeQuery,
		QueryTypeSelection
	};

	enum QueryGeometry
	{
		QueryGeometryBoundingBox,
		QueryGeometryBoundingSphere
	};

protected:

	unsigned __int64	queryExtents			(PThandle scene, PTdouble *localLower, PTdouble *localUpper, QueryType queryType, QueryGeometry type);
	unsigned __int64	queryMultiExtents		(const wchar_t *fakeFilePath, const wchar_t *realFilePath, bool testServer, QueryType queryType, QueryGeometry queryGeometry, const wchar_t *logFile);

public:

	unsigned __int64	runTest					(unsigned int &completedIterations);
	unsigned __int64	runTestClientServer		(unsigned int &completedIterations);
};
