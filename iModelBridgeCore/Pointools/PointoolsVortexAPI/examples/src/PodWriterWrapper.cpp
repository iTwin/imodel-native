//----------------------------------------------------------------------------
//
// PodWriterWrapper.cpp
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#include "PodWriterWrapper.h"

/* C++ headers */ 
#include <iostream>

/* C stuff */ 
#include <math.h>

/* Simple Macro to handle errors */ 
#define HandleResult(code) \
	if (code != POD_Success) { std::wcout << L"ERROR: " << getLastError() << std::endl; return false; }

// simple progress callback 
void __stdcall POD_Writer::showProgress(float p)
{
	std::wcout << (p*100) << L"%" << std::endl;
}

bool POD_Writer::startPODFile( const wchar_t *filename, const wchar_t *name, unsigned int spec )
{
	if (m_hPtioLib)
	{
		HandleResult( startStream( filename ) );
		HandleResult( addGroup(POD_GroupCombine, name) );
		HandleResult( addCloud( spec, L"cloud", 0, 0, 0) );
		return true;
	}
	else return false;
}

bool POD_Writer::endPODFile()
{
	if (m_hPtioLib)
	{
		HandleResult( endStream() );	
		HandleResult( buildPODFile() );	
		return true;
	}
	return false;
}
bool POD_Writer::initialize(bool debug)
{
	m_hPtioLib = LoadLibrary(debug ? _T("PODWriterd.dll") : _T("PODWriter.dll"));

	if (m_hPtioLib )
	{
		/* Note that functions ending in W also have ascii equivalents ending in A */ 
		startStream = (PTIO_STARTSTREAM)GetProcAddress(m_hPtioLib, "ptioStartStreamW"); 
		if (!startStream) return false;

		endStream = (PTIO_ENDSTREAM)GetProcAddress(m_hPtioLib, "ptioEndStream"); 
		if (!endStream) return false;

		addGroup = (PTIO_ADDGROUP)GetProcAddress(m_hPtioLib, "ptioAddGroupW"); 
		if (!addGroup) return false;

		addCloud = (PTIO_ADDCLOUD)GetProcAddress(m_hPtioLib, "ptioAddCloudW"); 
		if (!addCloud) return false;

		addPoint = (PTIO_ADDPOINT)GetProcAddress(m_hPtioLib, "ptioAddPoint"); 
		if (!addPoint) return false;

		addNullPoint = (PTIO_ADDNULLPOINT)GetProcAddress(m_hPtioLib, "ptioAddNullPoint"); 
		if (!addNullPoint) return false;

		setCompressionTolerance = (PTIO_SETCOMPRESSIONTOLERANCE)GetProcAddress(m_hPtioLib, "ptioSetCompressionTolerance"); 
		if (!setCompressionTolerance) return false;

		setNormalQuality = (PTIO_SETNORMALQUALITY)GetProcAddress(m_hPtioLib, "ptioSetNormalQuality"); 
		if (!setNormalQuality) return false;

		buildPODFile = (PTIO_BUILDPODFILE)GetProcAddress(m_hPtioLib, "ptioBuildPODFile"); 
		if (!buildPODFile) return false;

		setProgressCallback = (PTIO_SETPROGRESSCALLBACK)GetProcAddress(m_hPtioLib, "ptioSetProgressCallbackW"); 
		if (!setProgressCallback) return false;

		getLastError = (PTIO_GETLASTERROR)GetProcAddress(m_hPtioLib, "ptioGetLastErrorW"); 
		if (!getLastError) return false;

		spatialFiltering = (PTIO_SPATIALFILTERING)GetProcAddress(m_hPtioLib, "ptioSpatialFiltering"); 
		if (!spatialFiltering) return false;

		// Set up the progress callback			
		setProgressCallback( &showProgress );
	
		// some defaults
		setCompressionTolerance(0.001f);

		return true;
	}
	return false;
}