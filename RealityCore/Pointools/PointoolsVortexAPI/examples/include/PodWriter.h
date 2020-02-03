//----------------------------------------------------------------------------------------
//
// PodWriter.h
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------------------
// Pointools POD Export SDK
//----------------------------------------------------------------------------------------
// Author: Faraz Ravi, Technical Director, Pointools Ltd
//----------------------------------------------------------------------------------------
//
// Version 1.0.4
//
// Released October 08
//----------------------------------------------------------------------------------------
// Basic usage:
//
// 1. Load the dll at runtime using LoadLibrary*. Here's an example of how to do this:
/* 
		PTIO_STARTSTREAM ptioStartStream;
		PTIO_ENDSTREAM ptioEndStream;
		PTIO_ADDGROUP ptioAddGroup;
		PTIO_ADDCLOUD ptioAddCloud;
		PTIO_ADDPOINT ptioAddPoint;
		PTIO_ADDNULLPOINT ptioAddNullPoint;
		PTIO_SETCOMPRESSIONTOLERANCE ptioSetCompressionTolerance;
		PTIO_SETNORMALQUALITY ptioSetNormalQuality;
		PTIO_BUILDPODFILE ptioBuildPODFile;
		PTIO_SETPROGRESSCALLBACK ptioSetProgressCallback;
		PTIO_GETLASTERROR ptioGetLastError;

		static bool loadPtioLib()
		{
			hPtioLib = LoadLibrary("ptiolib.dll");

			if (hPtioLib)
			{
				ptioStartStream = (PTIO_STARTSTREAM)GetProcAddress(hPtioLib, "ptioStartStreamA"); 
				ptioEndStream = (PTIO_ENDSTREAM)GetProcAddress(hPtioLib, "ptioEndStream"); 
				ptioAddGroup = (PTIO_ADDGROUP)GetProcAddress(hPtioLib, "ptioAddGroupA"); 
				ptioAddCloud = (PTIO_ADDCLOUD)GetProcAddress(hPtioLib, "ptioAddCloudA"); 
				ptioAddPoint = (PTIO_ADDPOINT)GetProcAddress(hPtioLib, "ptioAddPoint"); 
				ptioAddNullPoint = (PTIO_ADDNULLPOINT)GetProcAddress(hPtioLib, "ptioAddNullPoint"); 
				ptioSetCompressionTolerance = (PTIO_SETCOMPRESSIONTOLERANCE)GetProcAddress(hPtioLib, "ptioSetCompressionTolerance"); 
				ptioSetNormalQuality = (PTIO_SETNORMALQUALITY)GetProcAddress(hPtioLib, "ptioSetNormalQuality"); 
				ptioBuildPODFile = (PTIO_BUILDPODFILE)GetProcAddress(hPtioLib, "ptioBuildPODFile"); 
				ptioSetProgressCallback = (PTIO_SETPROGRESSCALLBACK)GetProcAddress(hPtioLib, "ptioSetProgressCallbackA"); 
				ptioGetLastError = (PTIO_GETLASTERROR)GetProcAddress(hPtioLib, "ptioGetLastErrorA"); 
				
				return true;
			}
			return false;
		}

	* Alternatively a Object linking file (.lib) can be provided for MSVC 8 (2005 SP1) clients.

*/
//	2. Start a stream using:
//			
//			ptioStartStream("test.pod");
//
//	3. Add a cloud group:
//
//			ptioAddGroup(POD_GroupCombine, "GroupName");
//
//	4. Add a cloud (this one has RGB):
//
//			ptioAddCloud(POD_CloudRGB, "CloudName", 0, 0, 0);
//
//	5. Add each point in the cloud using:
//
//			ptioAddPoint(pnt, rgb, 0, 0); // double pnt[3]; unsigned char rgb[3];
//			
//	6. End the stream:
//			
//			ptioEndStream();
//
//	7. Build the pod file:
//
//			ptioBuildPODFile();
//
//	You may also want to use the progress callback to keep the user informed about progress.
//	This is quite straight forward. Here's a simple example:
//
//		static void __stdcall ShowProgress(const char*mess, float p)
//		{	
//			std::cout << mess << " " << (p*100) << "%" << std::endl;
//		}
//		...
//		ptioSetProgressCallback(&ShowProgress);
//		...
//
//	Filtering
//	============
//	As of version 1.0.3 you can also filter points spatially to ensure there is only
//  one point per specified spacing. This is useful to even out the varying density
//  of scan data, especially when there are typically a dense area of points near the 
//  scanner. 
//  To do this use the ptioSpatialFilter command with the spacing specified in the first
//  parameter. Currently 2nd, point_selection parameter has no affect - use POD_SpatialFilterAny
//
//	Language
//	============
//	The interface has been kept simple and is accessible from C or C++ with MSVC6.0 to 8.0. 
//	It should also be usable by other win32 compilers. If you want a binding for your 
//	language that is not available please contact support@pointools.com
//
//	As of version 1.0.2 there is a c# binding available. See the csharp_wrapper folder of 
//	the distribution.
//	
//  Unicode Support
//  ================
//	As of version 1.0.06 functions exported by the library that end A have unicode matching 
//  Unicode versions ending in W. To use the Unicode wchar interface import these functions
//  instead of the ascii versions and use the /D "UNICODE" switch
//
//	Memory
//	============
//	The library uses out-of-core methods for handling lengthy (multiple Gb) streams of 
//	input data. It should be possible to pass many Gb of data in points for export without
//	needing the client to perform special memory management.
//
//	Dependencies
//	=============
//	The library has a dependency on SHLWAPI.dll. This is sometimes not present on Win2000
//	if Internet Explorer 4.0 or later is not installed. In this case the library will fail
//	to load
//
//	Versions
//	=============
//	1.0.6	- Crash bug fix. Unicode support
//	1.0.5	- Support for format changes in POD file
//	1.0.4	- Intensity rescaling added.
//	1.0.3	- Spatial filtering added. POD_CloudOrderded renamed to POD_CloudRetainOrder
//			- Some optimisations. Documentation updated.
//	1.0.2	- C# wrapper added
//	1.0.1	- Testing Release
//	1.0.0	- Internal release
//----------------------------------------------------------------------------------------
//	LICENSE
//----------------------------------------------------------------------------------------
//	Released under the 'Pointools Free License':
//	You may use this SDK in a non-commercial or commercial application provided that:
//
//	1.	You clearly indicate that the pod format is the property of Pointools Ltd. 
//		This can be done by listing it in a menu or file browser as 'Pointools .pod'
//
//	2.	For commercial applications you notify Pointools Ltd that you are using this 
//		SDK by email to support@pointools.com indicating the product/s that use it.
//
//	3.	You accept that this SDK is made available WITHOUT ANY WARRANTY; without even 
//		the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
//
//	4.	You do not distribute this SDK to 3rd parties without the express permission
//		of Pointools Ltd.
//	
//----------------------------------------------------------------------------------------
//	(c) Copyright Pointools Ltd 2007-2009. All Rights Reserved
//----------------------------------------------------------------------------------------

#include <objidl.h>

#ifndef POINTOOLS_IO_INCLUDE
#define POINTOOLS_IO_INCLUDE 1


#include <tchar.h>

/* Cloud flags */ 
#define POD_CloudRetainOrder	1		//!< Indicates cloud point order to be retained
#define	POD_CloudRowMajor		2		//!< Indicates cloud point order is row first, then columns.
#define POD_CloudRGB			4		//!< Indicates cloud point RGB color value(s) (Red, Green, Blue)
#define POD_CloudIntensity		8		//!< Indicates cloud point intensity value
#define POD_CloudNormal			16		//!< Indicates cloud point surface normal
#define POD_CloudClassification	32		//!< Indicates cloud points classification

/* Group flags */ 
#define POD_GroupCombine			1	//!< Specifies group combine
#define POD_GroupGenNormals			2	//!< Specifies group generation of surface normals
#define POD_GroupRescaleIntensities 4	//!< Specifies group rescale of intensity values to optimize contrast

/* Spatial Filtering */ 
#define POD_SpatialFilterAny		1	//!< Specifies general spatial filter option.
#define POD_SpatialFilterAverage	2	//!< Specifies average spatial filter option.
#define POD_SpatialFilterCenterMost 3	//!< Specifies CenterMost spatial filter option.

/* Return codes */ 
#define POD_False				0		//!< Error code. Specifies false.
#define POD_Success				1		//!< Error code. Specifies operation succeeded.

#define POD_Fail				2		//!< Error code. Specifies operation failure.
#define POD_WriteFailure		3		//!< Error code. Specifies that a output writing operation failed.
#define POD_Uninitialised		4		//!< Error code. Specifies an uninitialized state.
#define POD_InvalidParameter	5		//!< Error code. Specifies that a given parameter is incorrect.
#define POD_ReadStreamFailure	6		//!< Error code. Specifies a failure reading from a stream.
#define POD_OutOfMemory			7		//!< Error code. Specifies that memory was not available for an operation.
#define POD_NoPoints			8		//!< Error code. Specifies no points.


#define PTIO_RESULT unsigned long		//!< Error code result type. See #define values for specific errors.
typedef void (__stdcall *PTIO_PROGRESSCALLBACK)(float); //!< Progress callback prototype type

#ifdef PTIO_USE_IMPORT_LIB

//! \file PODWriter.h Main Pointools PODWriter API header file

															//! Starts writing to an output stream

															//! This should be called before writing to the stream.
															//! ptioEndStream() should be called when writing is complete

															//! \param filepath File path of the output POD
															//! \return Error code
															//! \see ptioEndStream()
 PTIO_RESULT ptioStartStream(const TCHAR* filepath);

															//! Ends the input stream.
															
															//! This should be called after all the data is entered and before ptioBuildPODFile()
															//! \param void
															//! \return Error code
															//! \see ptioStartStream()
 PTIO_RESULT ptioEndStream(void);


															//! Starts a new cloud group.
															
															//! This must be called on an open stream.
															//! This must be used before any calls to Add Cloud
															//! \flags Option flags.
															//! \param identifier C string identifying cloud (max 32 characters)
															//! \return Error code
															//! \see ptioStartStream()
															//! \see ptioEndStream()
 PTIO_RESULT ptioAddGroup(unsigned long flags, const TCHAR* identifier);

															//! Creates a single new point cloud

															//! This must be called on an open stream.
															//! \param flags Options flags. The following flags can be used in combination:
															//! - POD_CloudRetainOrder:	Stores point order information with the Cloud. This will increase file size and will not render correctly in View 1.7 RC2 and earlier
															//! - POD_CloudRowMajor: Indicates that the points are row major ordered. The default is Column ordered (as is the case with most scanner formats)
															//! - POD_CloudRGB: Indicates that the points have an 3byte RGB value
															//! - POD_CloudIntensity: Indicates that the points have an intensity value
															//! - POD_CloudNormal: Indicates that the points have a normal
															//! \param identifier C string identifying cloud max 32 characters
															//! \param transform Row ordered transformation matrix. Use NULL for no transformation.
															//! \param xres X Resolution of grid for gridded data. Use 0 for ungridded data.
															//! \param yres Y Resolution of grid for gridded data. Use 0 for ungridded data.
															//! \return Error code.
															//! \see ptioStartStream()
															//! \see ptioEndStream()
PTIO_RESULT ptioAddCloud(unsigned long flags, const TCHAR *identifier, double *transform, long xres, long yres);

															//! Add a point with attributes

															//! This must be called on an open stream.
															//! For invalid points, use ptioAddNullPoint().
															//! \param pnt Cartesian coords of point as an array of 3 doubles . If a transformation matrix was supplied this should be in untransformed space.
															//! \param rgb Red, Green, Blue as an array of 3 bytes. Use NULL if these values are not used.
															//! \param intensity Intensity value expressed as short. short values range from -32768 to 32767.
															//! The following code shows how to convert a floating point intensity value in the period 0-1 into a short value:
															//! \code short sItensity = (short)((fIntensity - 0.5f) * 65535.0f); \endcode
															//! \param normal Normalised point normal vector as an array of 3 float values. Use NULL if normals are not used.
															//" \param category Point type classification.
															//! \return Error code.
															//! \see ptioAddNullPoint()
															//! \see ptioStartStream()
															//! \see ptioEndStream()
PTIO_RESULT ptioAddPoint(const double *const pnt, const unsigned char * const rgb, short intensity, const float * const normal, unsigned char classification);

															//! Adds a NULL or zero point to the cloud.

															//! This is used to represent a void point in gridded data. This has has no effect if the data is not gridded.
															//! This must be called on an open stream.
															//! \param void
															//! \return Error code.
															//! \see ptioAddPoint()
															//! \see ptioStartStream()
															//! \see ptioEndStream()
															//! \return Error code.
PTIO_RESULT ptioAddNullPoint(void);


															//! Sets the sharp - smooth quality of the normal calculation.

															//! Smoother normals are achieved by take a larger sample of neighbouring points and therefore take longer to compute.
															//! This must be called on an open stream.
															//! \param quality Quality parameter. valid values are from 0 (sharp) to 2.0 (smooth). The default value is 1.0f and works well for most applications.
															//! \return Error code.
															//! \see ptioStartStream()
															//! \see ptioEndStream()
PTIO_RESULT ptioSetNormalQuality(float quality);

															//! Filter points spatially.

															//! This can be useful to even out density of scan data. Default is disabled.
															//! \param spacing Distance between points in each of the 3 dimensions
															//! \param point_selection Point selection policy. This can be one of the following:
															//! - POD_SpatialFilterAny: Picks any point, this is the fastest and most memory efficient
															//! - POD_SpatialFilterAverage: Generates an average point	
															//! - POD_SpatialFilterCenterMost: Picks the best quality point. This only affective on data in scan space and assumes points closer to the scanner are of better quality
															//! - Set to 0 to disable filtering.
															//! \return Error code.
PTIO_RESULT ptioSpatialFiltering(float spacing, unsigned long point_selection);

															//! Sets a metatag value in the POD file.

															//! The tag is specified as a string "section.tag" where the first part indicates the name of the metadata section.
															//! \param sectionDotTagname Specifies the tag to be set, can be one of the following:
															//! - "Instrument.ScannerManufacturer" : The Manufacturer of the sensor used to capture the data
															//! - "Instrument.ScannerModel" : The name of the scanner Model
															//! - "Instrument.ScannerSerial" : The serial number of the scanner
															//! - "Instrument.CameraModel" : The camera model used to capture RGB
															//! - "Instrument.CameraSerial" : The camera serial number
															//! - "Instrument.CameraLens" : The camera lens
															//! - "Survey.Company" : The company that captured the data.
															//! - "Survey.Operator" : The operator name
															//! - "Survey.ProjectName" : The project name
															//! - "Survey.ProjectCode" : The project code, does not have to conform to particular convention
															//! - "Survey.DateOfCapture" : The date of capture, must be specified as YYYY-MM-DD
															//! - "Survey.Site" : Text describing the siteor object  captured
															//! - "Survey.SiteLong" : Site's Longtitude (does not affect data positioning, for information only)
															//! - "Survey.SiteLat" : Sites Latitude	(does not affect data positioning, for information only)
															//! - "Survey.GeoReference" : Georeference information in WKT format
															//! - "Survey.ZipCode" : Zip or Postal code of site
															//! - "Description.Description" : Description of the scans contents
															//! - "Description.Keywords" : Keywords describing data, multiple words seperated by semicolon. 															
															//! - "Description.Category" : Category, one of:
															//!		- "Aerial Lidar"
															//!		- "Terrestrial Phase Based"
															//!		- "Terrestrial Time of Flight"
															//!		- "Mobile mapping"
															//!		- "Bathymetric"
															//!		- "Photogrammetric"
															//!		- "Synthesized"
															//!	- "Audit.ScanPaths" : Original file paths of source input files. This may be set mutliple times to add multiple file paths
															//! - "Audit.OriginalNumScans" : Number of original scans, note this may differ from number of original files
															//! - "Audit.CreatorApp" : The application that created the POD file

															//! \param 	valueAsString String with new value
															//! \return Error code.
															//! \see ptioSetUserMetaTag()
PTIO_RESULT ptioSetMetaTag( const TCHAR *sectionDotTagname, const TCHAR *valueAsString );

															//! Sets a user meta tag

															//! \param sectionDotTag String with tag name in "section.tag" form
															//! \param valueAsString String with value for metatag
															//! \return Error code.
															//! \see ptioSetMetaTag
PTIO_RESULT ptioSetUserMetaTag( const TCHAR *sectionDotTag, const TCHAR *valueAsString );

															//! Returns the last error as a string

															//! \param void
															//! \return Error string.
const TCHAR* ptioGetLastError(void);

															//! Writes the POD file.

															//! This should be called  once all the data has been entered.
															//! Tis must be called after the stream has been closed using a call to ptioEndStream().
															//! \param void
															//! \return Error code.
															//! \see ptioEndStream()
PTIO_RESULT ptioBuildPODFile(void);

															//! Sets the maximum error as the distance from the original point, tolerated by the compression algorithm.

															//! This value represents the worst case so most data will be considerably closer to the original data.
															//! It is important to use a sensible value, too smaller value will result in excessive indexing times and poor performance. 
															//! For example with terrestrial scan data with an accuracy of +-4mm, a value of 1mm works well.
															//! \param distance Maximum error distance from an original point. The default value is 1mm.
															//! \return Error code.
PTIO_RESULT ptioSetCompressionTolerance(float distance);

															//! Sets a user supplied progress callback function.

															//! Allows you to specify your own function that will be called during the progress on the POD file creation.
															//! This can be used to display a progress message or progress bar. A simple example showing the correct function prototype is:
															//! \code
															/*! static void ShowProgress(const char*mess, float p)
															{
															std::cout << mess << " " << (p*100) << "%" << std::endl;
															}
															*/
															//! \param callback Callback function with required prototype of form PTIO_PROGRESSCALLBACK.
															//! \return Error code.
PTIO_RESULT ptioSetProgressCallback(PTIO_PROGRESSCALLBACK callback);


#else
/* PTIO_RESULT ptioStartStream(const char *filepath); */ 
/*!Start Stream
filepath: the file path of the output pod

*/
typedef PTIO_RESULT (__stdcall *PTIO_STARTSTREAM)(const TCHAR* filepath);

/* PTIO_RESULT ptioEndStream() */ 
/*! EndStream

Ends the input stream. This should be called after all the data is entered and before BuildPODFile
*/
typedef PTIO_RESULT (__stdcall *PTIO_ENDSTREAM)(void);

/* PTIO_RESULT ptioAddGroup(unsigned long flags, const char *identifier);*/ 
/*!Add Group

starts a new cloud group. This must be used before any calls to Add Cloud
identifier: C string  
flags:

*/
typedef PTIO_RESULT (__stdcall *PTIO_ADDGROUP)(unsigned long flags, const TCHAR* identifier);

/* PTIO_RESULT ptioAddCloud(unsigned long flags, const char *identifier, double* transform, long xres, long yres); */ 
/*! Add Cloud

flags:	The following flags can be used in combination:
POD_CloudRetainOrder-	Stores point order information with the Cloud. This will increase file size and will not render correctly in 
						View 1.7 RC2 and earlier
POD_CloudRowMajor	-	Indicates that the points are row major ordered. The default is Column ordered (as is the case with most scanner formats)
POD_CloudRGB		-	Indicates that the points have an 3byte RGB value
POD_CloudIntensity	-	Indicates that the points have an intensity value
POD_CloudNormal		-	Indicates that the points have a normal

identifier: C string identifying cloud max 32 characters
transform:	Row ordered transformation matrix. Use NULL for no transformation
xres, yres: x and y resolution of grid for gridded data. Use 0 for ungridded data

*/
typedef PTIO_RESULT (__stdcall *PTIO_ADDCLOUD)(unsigned long flags, const TCHAR *identifier, double *transform, long xres, long yres);

/* PTIO_RESULT ptioAddPoint(double *pnt, unsigned char *rgb, short intensity, float *normal); */ 
/*! Add Point 
pnt:		Cartesian coords of point as an array of 3 doubles . If a transformation matrix was supplied this should be in untransformed space
rgb:		Red, Green, Blue as an array of 3 bytes. Use NULL if these values are not used
intensity:	Intensity value expressed as short. short values range from -32767 to 32767. 
classification:	Point's classfication expressed as an unsigned byte value
The following code shows how to convert a floating point intensity value in the period 0-1 into a short value:

short sItensity = (short)((fIntensity - 0.5f) * 65535.0f);
normal:		Normalised point normal vector as an array of 3 float values. Use NULL if normals are not used

*/
typedef PTIO_RESULT (__stdcall *PTIO_ADDPOINT)(const double *const pnt, const unsigned char *const rgb, short intensity, const float *const normal, unsigned char classification);

/* PTIO_RESULT ptioAddNullPoint(); */ 
/*! AddNullPoint

Adds a NULL or zero point to the cloud. This is used to represent a void point in gridded data - it has no 
effect if the data is not gridded
*/
typedef PTIO_RESULT (__stdcall *PTIO_ADDNULLPOINT)(void);

/* PTIO_RESULT ptioSetNormalQuality(float quality); */ 
/*! SetNormalQuality

Sets the sharp/smooth quality of the normal calculation. Smoother normals are achieved by take a larger sample 
of neighbouring points and therefore take longer to compute.

valid values are from 0 (sharp) to 2.0 (smooth). The default value is 1.0f and works well for most applications.
*/
typedef PTIO_RESULT (__stdcall *PTIO_SETNORMALQUALITY)(float quality);

/* PTIO_RESULT ptioSetCompressionTolerance(float distance);*/ 
/*! SetCompression Tolerance
Sets the maximum error as the distance from the original point, tolerated by the compression algorithm.
This value represents the worst case so most data will be considerably closer to the original data. 
It is important to use a sensible value, too smaller value will result in excessive indexing times and poor performance. 

For example with terrestrial scan data with an accuracy of +-4mm a value of 1mm works well.

The default value is 1mm
*/
typedef PTIO_RESULT (__stdcall *PTIO_SETCOMPRESSIONTOLERANCE)(float distance);

/* PTIO_RESULT ptioSetProgressCallback(PTIO_PROGRESSCALLBACK callback); */ 
/*!SetProgressCallback

Allows you to specify your own function that will be called during the progress on the POD file creation. This can
be used to display a progress message or progress bar. An simple example showing the correct function prototype is shown below:

// simple progress callback
static void ShowProgress(const char*mess, float p)
{
std::cout << mess << " " << (p*100) << "%" << std::endl;
}
ptioSetProgressCallback((unsigned long)&ShowProgress);

*/
typedef PTIO_RESULT (__stdcall *PTIO_SETPROGRESSCALLBACK)(PTIO_PROGRESSCALLBACK callback);

/* PTIO_RESULT ptioBuildPODFile(); */ 
/*! BuildPODFile

Writes the POD file. This should be called  once all the data has been entered
*/
typedef PTIO_RESULT (__stdcall *PTIO_BUILDPODFILE)(void);

/* PTIO_RESULT ptioSpatialFiltering(float spacing, unsigned long point_selection) */ 
/*! Spatial Filtering

Filtering points spatially. This can be useful to even out density of scan data. Default is disabled.

spacing	-	distance between points in each of the 3 dimensions
point_selection - point selection policy. This can be one of the following:

	POD_SpatialFilterAny		- Picks any point, this is the fastest and most memory efficient
	POD_SpatialFilterAverage	- Generates an average point	
	POD_SpatialFilterCenterMost - Picks the best quality point. This only affective on data in scan space and assumes points closer to the scanner are of better quality
	
Set this to 0 to disable filtering.

*/
typedef PTIO_RESULT (__stdcall *PTIO_SPATIALFILTERING)(float spacing, unsigned long point_selection);


/* PTIO_RESULT ptioSetMetaTag( const tchar *sectionDotTagname, const tchar *valueAsString ) */ 
/*! SetMetaTag

Sets a metatag value in the POD file. The tag is specified as a string "section.tag" where the first part indicates the name of the metadata section

	sectionDotTagname		- Specifies the tag to be set, can be one of the following:

							"Instrument.ScannerManufacturer"	- The Manufacturer of the sensor used to capture the data
							"Instrument.ScannerModel"			- The name of the scanner Model
							"Instrument.ScannerSerial"			- The serial number of the scanner
							"Instrument.CameraModel"			- The camera model used to capture RGB
							"Instrument.CameraSerial"			- The camera serial number
							"Instrument.CameraLens"				- The camera lens

							"Survey.Company"					- The company that captured the data
							"Survey.Operator"					- The operator name
							"Survey.ProjectName"				- The project name
							"Survey.ProjectCode"				- The project code, does not have to conform to particular convention
							"Survey.DateOfCapture"				- The date of capture, must be specified as YYYY-MM-DD
							"Survey.Site"						- Text describing the siteor object  captured
							"Survey.SiteLong"					- Site's Longtitude (does not affect data positioning, for information only)
							"Survey.SiteLat"					- Sites Latitude	(does not affect data positioning, for information only)
							"Survey.GeoReference"				- Georeference in WKT format
							"Survey.ZipCode"					- Zip or Postal code of site

							"Description.Description"			- Description of the scans contents
							"Description.Keywords"				- Keywords describing data, multiple words seperated by semicolon. 
							"Description.Category"				- Category, one of:

																	"Aerial Lidar"
																	"Terrestrial Phase Based"
																	"Terrestrial Time of Flight"
																	"Mobile mapping"
																	"Bathymetric"
																	"Photogrammetric"
																	"Synthesized"

							"Audit.ScanPaths"					- Original file paths of source input files. This may be set mutliple times to add multiple file paths
							"Audit.OriginalNumScans"			- Number of original scans, note this may differ from number of original files
							"Audit.CreatorApp"					- The application that created the POD file

	valueAsString		- string with new value


*/
typedef PTIO_RESULT (__stdcall *PTIO_SETMETATAG)( const TCHAR *sectionDotTagname, const TCHAR *valueAsString );


/* PTIO_RESULT ptioSetMetaTag( const TCHAR *sectionDotTagname, const TCHAR *valueAsString ) */ 
/*! SetUserMetaTag

Sets a user meta tag.

	sectionDotTag		- string with tag name in "section.tag" form
	valueAsString		- string with value for metatag

*/ 
typedef  PTIO_RESULT (__stdcall *PTIO_SETUSERMETATAG)( const TCHAR *sectionDotTag, const TCHAR *valueAsString );


/* const TCHAR* ptioGetLastError(); */ 
/*const TCHAR* GetLastError

returns the last error as a string
*/
typedef const TCHAR* (__stdcall *PTIO_GETLASTERROR)(void);



#endif
#endif
