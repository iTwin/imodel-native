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

	uint64_t queryExtents			(PThandle scene, PTdouble *localLower, PTdouble *localUpper, QueryType queryType, QueryGeometry type);
	uint64_t	queryMultiExtents		(const wchar_t *fakeFilePath, const wchar_t *realFilePath, bool testServer, QueryType queryType, QueryGeometry queryGeometry, const wchar_t *logFile);

public:

	uint64_t	runTest					(unsigned int &completedIterations);
	uint64_t	runTestClientServer		(unsigned int &completedIterations);
};
