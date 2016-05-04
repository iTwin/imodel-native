//----------------------------------------------------------------------------
//
// ExtentsQueryTest.cpp
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#include <ExtentsQueryTest.h>

#include <fstream>
#include <math.h>

const unsigned int numTestFiles = 3;
wchar_t	testFiles[][1024] =
{
	L"New_Orleans.pod",
	L"8825Gatehouse.pod",
	L"robot-cell-clean.pod"
};

const unsigned int numTestFilesClientServer = 97;
wchar_t	testFilesClientServer[][1024] =
{
	L"22862100.pod",
	L"22862102.pod",
	L"22862104.pod",
	L"22862106.pod",
	L"22882098.pod",
	L"22882100.pod",
	L"22882102.pod",
	L"22882104.pod",
	L"22882106.pod",
	L"22902096.pod",
	L"22902098.pod",
	L"22902100.pod",
	L"22902102.pod",
	L"22902104.pod",
	L"22902106.pod",
	L"22922092.pod",
	L"22922094.pod",
	L"22922096.pod",
	L"22922098.pod",
	L"22922100.pod",
	L"22922102.pod",
	L"22922104.pod",
	L"22922106.pod",
	L"22942092.pod",
	L"22942094.pod",
	L"22942096.pod",
	L"22942098.pod",
	L"22942100.pod",
	L"22942102.pod",
	L"22942104.pod",
	L"22942106.pod",
	L"22942108.pod",
	L"22942110.pod",
	L"22942112.pod",
	L"22942114.pod",
	L"22962088.pod",
	L"22962090.pod",
	L"22962092.pod",
	L"22962094.pod",
	L"22962096.pod",
	L"22962098.pod",
	L"22962100.pod",
	L"22962102.pod",
	L"22962104.pod",
	L"22962106.pod",
	L"22962108.pod",
	L"22962110.pod",
	L"22962112.pod",
	L"22962114.pod",
	L"22982088.pod",
	L"22982090.pod",
	L"22982094.pod",
	L"22982096.pod",
	L"22982098.pod",
	L"22982100.pod",
	L"22982102.pod",
	L"22982104.pod",
	L"22982106.pod",
	L"22982108.pod",
	L"22982110.pod",
	L"22982112.pod",
	L"22982114.pod",
	L"23002088.pod",
	L"23002090.pod",
	L"23002092.pod",
	L"23002094.pod",
	L"23002096.pod",
	L"23002098.pod",
	L"23002100.pod",
	L"23002102.pod",
	L"23002104.pod",
	L"23002106.pod",
	L"23002108.pod",
	L"23002110.pod",
	L"23002112.pod",
	L"23022092.pod",
	L"23022094.pod",
	L"23022096.pod",
	L"23022098.pod",
	L"23022100.pod",
	L"23022102.pod",
	L"23022104.pod",
	L"23022106.pod",
	L"23022108.pod",
	L"23022110.pod",
	L"23022112.pod",
	L"23042092.pod",
	L"23042094.pod",
	L"23042096.pod",
	L"23042098.pod",
	L"23042100.pod",
	L"23042102.pod",
	L"23042104.pod",
	L"23042106.pod",
	L"23042108.pod",
	L"23042110.pod",
	L"23042112.pod"

//	L"C:\\a_gatehouse.pod",
//	L"C:\\a_new_orleans.pod"
};



uint64_t ExtentsQueryTest::queryExtents(PThandle scene, PTdouble *localLower, PTdouble *localUpper, QueryType queryType, QueryGeometry geometry)
{
	PThandle query;

	double	center[3];
	double	diagonal[3];

	center[0]	= (localUpper[0] + localLower[0]) * 0.5;
	center[1]	= (localUpper[1] + localLower[1]) * 0.5;
	center[2]	= (localUpper[2] + localLower[2]) * 0.5;

	diagonal[0]	= (localUpper[0] - localLower[0]);
	diagonal[1]	= (localUpper[1] - localLower[1]);
	diagonal[2]	= (localUpper[2] - localLower[2]);

	double diagonalLength = sqrt(diagonal[0] * diagonal[0] + diagonal[1] * diagonal[1] + diagonal[2] * diagonal[2]);
	double radius = diagonalLength * 0.5;

	switch(queryType)
	{
	case QueryTypeQuery:

		switch(geometry)
		{
		case QueryGeometryBoundingBox:

			query = ptCreateBoundingBoxQuery(localLower[0], localLower[1], localLower[2], localUpper[0], localUpper[1], localUpper[2]);
			break;

		case QueryGeometryBoundingSphere:

			query = ptCreateBoundingSphereQuery(center, radius);

		}

		break;

	case QueryTypeSelection:

		switch(geometry)
		{
		case QueryGeometryBoundingBox:

			ptSelectPointsByBox(localLower, localUpper);

			query = ptCreateSelPointsQuery();

			break;

		case QueryGeometryBoundingSphere:

			ptSelectPointsBySphere(center, radius);

			query = ptCreateSelPointsQuery();
		}

		break;
	}

	if(query == NULL)
		return false;

	ptSetQueryDensity(query, PT_QUERY_DENSITY_FULL, 1.0);

	const unsigned int	numBufferPoints = 256*1024;

	double			*	bufferGeom;
	PTubyte			*	bufferRGB;
	PTshort			*	bufferIntensity;
	unsigned int		n;
	uint64_t	totalQueryPoints;

	bufferGeom		= new double[numBufferPoints * 3];
	bufferRGB		= new PTubyte[numBufferPoints * 3];
	bufferIntensity	= new PTshort[numBufferPoints];

	totalQueryPoints = 0;

	do
	{
		n = ptGetDetailedQueryPointsd(query, numBufferPoints, bufferGeom, bufferRGB, bufferIntensity, NULL, NULL, NULL, 0, NULL, NULL);

		totalQueryPoints += n;

	} while (n > 0);

	if(bufferGeom)
		delete []bufferGeom;

	if(bufferRGB)
		delete []bufferRGB;

	if(bufferIntensity)
		delete []bufferIntensity;

	ptDeleteQuery(query);

	if(queryType == QueryTypeSelection)
		ptUnselectAll();

	return totalQueryPoints;
}

uint64_t ExtentsQueryTest::queryMultiExtents(const wchar_t *fakeFilePath, const wchar_t *realFilePath, bool testServer, QueryType queryType, QueryGeometry queryGeometry, const wchar_t *logFile)
{
	double				scales[] = {0.10, 0.25, 0.50, 0.75, 1.0};
	unsigned int		numScales = 5;
	double				extentsLower[3];
	double				extentsUpper[3];
	double				extentsCenter[3];
	double				extentsHalfSize[3];
	unsigned int		s;
	uint64_t	numPoints;
	uint64_t	maxPoints = 0;
	double				localLower[3];
	double				localUpper[3];

	unsigned int		emptyDataSize = 256;
	unsigned char		emptyData[256];

	for(unsigned int t = 0; t < emptyDataSize; t++)
		emptyData[t] = t;

	std::wofstream testOut;

	wchar_t testFilePath[] = L"C:\\Test.txt";

	if(logFile)
	{
		testOut.open(logFile, std::ios::app);
	}

	for(s = 0; s < numScales; s++)
	{
		wprintf(L"  Scale Test: %d  : ", s + 1);

		if(testServer)
		{
			if(ptCreateFakePOD(const_cast<wchar_t *>(realFilePath), emptyData, emptyDataSize, const_cast<wchar_t *>(fakeFilePath)) == false)
				return 0;
		}

		PThandle scene;
															// Open POD file
															// If it fails, return the iteration it failed on
		if(testServer)
		{
			scene = ptOpenPOD(fakeFilePath);
		}
		else
		{
			scene = ptOpenPOD(realFilePath);
		}

		if(scene == NULL)
			return 0;
															// Get local bounding extents
		ptGetLowerBound(localLower);
		ptGetUpperBound(localUpper);

		extentsCenter[0]	= ((localUpper[0] + localLower[0]) * 0.5);
		extentsCenter[1]	= ((localUpper[1] + localLower[1]) * 0.5);
		extentsCenter[2]	= ((localUpper[2] + localLower[2]) * 0.5);

		extentsHalfSize[0]	= localUpper[0] - extentsCenter[0];
		extentsHalfSize[1]	= localUpper[1] - extentsCenter[1];
		extentsHalfSize[2]	= localUpper[2] - extentsCenter[2];

		extentsLower[0]		= extentsCenter[0] - (extentsHalfSize[0] * scales[s]);
		extentsLower[1]		= extentsCenter[1] - (extentsHalfSize[1] * scales[s]);
		extentsLower[2]		= extentsCenter[2] - (extentsHalfSize[2] * scales[s]);

		extentsUpper[0]		= extentsCenter[0] + (extentsHalfSize[0] * scales[s]);
		extentsUpper[1]		= extentsCenter[1] + (extentsHalfSize[1] * scales[s]);
		extentsUpper[2]		= extentsCenter[2] + (extentsHalfSize[2] * scales[s]);

		pt::PerformanceTimer timer;
		timer.start();

		numPoints = queryExtents(scene, extentsLower, extentsUpper, queryType, queryGeometry);

		timer.end();

		unsigned int timeMilliSeconds = (unsigned int) timer.millisecs();

		if(logFile)
		{
			testOut << realFilePath << L"\t" << (s + 1) << L"\t" << numPoints << L"\t" << timeMilliSeconds << L"\n" << std::flush;
		}

		ptRemoveAll();

		if(testServer && fakeFilePath)
		{
			DeleteFile(fakeFilePath);
		}


		wprintf(L"%d\n", numPoints);

		if(numPoints > maxPoints)
			maxPoints = numPoints;
	}

	if(logFile)
	{
		testOut.close();
	}
															// Return max number of points obtained by one query
	return maxPoints;
}

uint64_t ExtentsQueryTest::runTest(unsigned int &completedIterations)
{
	unsigned int		t;
	unsigned int		emptyDataSize = 256;
	uint64_t	maxPoints;

	const wchar_t		logFile[]			= L"C:\\Test.txt";
	wchar_t				realFilePathBase[]	= L"D:\\PointoolsData\\General\\POD\\";


	DeleteFile(logFile);

	completedIterations = 0;

	wchar_t *filepath = NULL;

//	QueryGeometry queryGeometry = ExtentsQueryTest::QueryGeometryBoundingBox;
	QueryGeometry queryGeometry = ExtentsQueryTest::QueryGeometryBoundingSphere;


	for(t = 0; t < numTestFiles; t++)
	{
		unsigned int testFileIndex = t;

		filepath = testFiles[testFileIndex];

		wprintf(L"%d : %s\n", t + 1, filepath);

		std::wstring realFilePath = std::wstring(realFilePathBase) + filepath;

															// If a query fails, return the iteration it failed on
		if((maxPoints = queryMultiExtents(NULL, realFilePath.c_str(), false, ExtentsQueryTest::QueryTypeQuery, queryGeometry, logFile)) == 0)
		{
			wprintf(L"\nWarning: No points returned in any query for %s\n", realFilePath);
		}

		completedIterations++;

		wprintf(L"\n");
															// Remove all scenes
		ptRemoveAll();
	}


	for(t = 0; t < numTestFiles; t++)
	{
		unsigned int testFileIndex = t;

		filepath = testFiles[testFileIndex];

		wprintf(L"%d : %s\n", t + 1, filepath);

		std::wstring realFilePath = std::wstring(realFilePathBase) + filepath;

															// If a query fails, return the iteration it failed on
		if((maxPoints = queryMultiExtents(NULL, realFilePath.c_str(), false, ExtentsQueryTest::QueryTypeSelection, queryGeometry, logFile)) == 0)
		{
			wprintf(L"\nWarning: No points returned in any query for %s\n", realFilePath);
		}

		completedIterations++;

		wprintf(L"\n");
															// Remove all scenes
		ptRemoveAll();
	}


	wprintf(L"\nCompleted.\n");
															// Return final iteration file name
	return t;
}


uint64_t ExtentsQueryTest::runTestClientServer(unsigned int &completedIterations)
{
	unsigned int		t;
	unsigned int		emptyDataSize = 256;
	uint64_t	maxPoints;

	wchar_t				fakeFilePathBase[] = L"C:\\test\\atps\\PointCloudTester\\PointCloudTest\\baselinefiles\\PointCloudTester_BoundaryQueryServerTest\\Data - InternalOnly\\pod\\BoxHundredPercent\\Data - InternalOnly_pod_Apple Canyon_";
	wchar_t				realFilePathBase[] = L"C:\\DataSet\\PointCloud\\PointCloud\\Data - InternalOnly\\pod\\Apple Canyon\\";

// Data - InternalOnly_pod_Apple Canyon_22862102.pod.pw.pod

	completedIterations = 0;

	wchar_t *filepath = NULL;

	for(t = 0; t < numTestFiles; t++)
	{
		unsigned int testFileIndex = t;

		filepath = testFilesClientServer[testFileIndex];

		wprintf(L"%d : %s\n", t + 1, filepath);

		std::wstring realFilePath = std::wstring(realFilePathBase) + filepath;
		std::wstring fakeFilePath = std::wstring(fakeFilePathBase) + std::wstring(filepath) + L".pw.pod";
															// If a query fails, return the iteration it failed on
		if((maxPoints = queryMultiExtents(fakeFilePath.c_str(), realFilePath.c_str(), false, ExtentsQueryTest::QueryTypeQuery, ExtentsQueryTest::QueryGeometryBoundingBox, L"C:\\Test.txt")) == 0)
		{
			wprintf(L"\nWarning: No points returned in any query for %s\n", fakeFilePath);
//			return t;
		}

 		completedIterations++;

		wprintf(L"\n");
															// Remove all scenes
		ptRemoveAll();

		DeleteFile(fakeFilePath.c_str());
	}

	wprintf(L"\nCompleted.\n");
															// Return final iteration file name
	return t;
}
