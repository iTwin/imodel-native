//----------------------------------------------------------------------------------------
//
// PodWriterWrapper.h
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------------------
// POD Writer class
//----------------------------------------------------------------------------------------
#ifndef POINTOOOLS_POD_WRITER_WRAPPER
#define POINTOOOLS_POD_WRITER_WRAPPER

#include <windows.h>
#include <tchar.h>

/* API headers */ 
#include "podwriter.h"

#define POD_CloudRGB			4
#define POD_CloudIntensity		8
#define POD_CloudNormal			16


/* function to load ptiolib.dll. The dll is linked at run-time and functions are imported */ 
/* For most implementations you can simply cut and paste this code for your own use       */ 

class POD_Writer
{
public:
	POD_Writer() : m_hPtioLib(0) {}
	virtual ~POD_Writer()	{};

	bool initialize(bool debugDll=false);

	bool startPODFile( const wchar_t *filename, const wchar_t *name = L"CloudGroup", 
		unsigned int Spec = POD_CloudIntensity | POD_CloudRGB );

	bool endPODFile();

	PTIO_ADDPOINT				addPoint;
	PTIO_ADDNULLPOINT			addNullPoint;
	PTIO_SETCOMPRESSIONTOLERANCE setCompressionTolerance;
	PTIO_SETNORMALQUALITY		setNormalQuality;
	PTIO_SPATIALFILTERING		spatialFiltering;
	PTIO_GETLASTERROR			getLastError;

private:
	static void __stdcall POD_Writer::showProgress(float p);

	PTIO_STARTSTREAM			startStream;
	PTIO_ENDSTREAM				endStream;
	PTIO_ADDGROUP				addGroup;
	PTIO_ADDCLOUD				addCloud;
	PTIO_BUILDPODFILE			buildPODFile;
	PTIO_SETPROGRESSCALLBACK	setProgressCallback;

	HINSTANCE	m_hPtioLib;

};

#endif