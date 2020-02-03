/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/stdafx.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
#pragma once

#pragma managed(push, off)

#include <windows.h>
#include <Imagepp/h/ImageppAPI.h>

#include <tchar.h>

// Temporary patch because we do not want to depend upon Bentley.ImagePP.dll (Virtual Earth and GeoRaster)
// Need to make HRFFileFormats.h more versatile.
#define PROJECTWISE_FILE_FORMATS
#include <Imagepp/all/h/HRFFileFormats.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFPageFileFactory.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HUTClassIDDescriptor.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>

USING_NAMESPACE_IMAGEPP

#pragma managed(pop)

#using <System.dll>
#using <System.Data.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
#using <System.XML.dll>
#using <System.Configuration.dll>
#include "MainUI.h"
