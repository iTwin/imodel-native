/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HRFCapabilitiesExporter/stdafx.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Imagepp/h/ImageppAPI.h>                // must be first for PreCompiledHeader Option
#include <tchar.h>
#include <windows.h>
#include <winbase.h>
#include <stdexcpt.h>
#include <iostream>
#include <shlwapi.h>
#include <shlobj.h>
#include <io.h>
#include <conio.h>

#include <Imagepp/all/h/HUTClassIDDescriptor.h>
#include <Imagepp/all/h/HUTImportFromFileExportToFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HPMPool.h>

USING_NAMESPACE_IMAGEPP

#include "CustomTypes.h"
