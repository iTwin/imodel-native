/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/HMRStress.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "HMRStress.h"

#ifdef STRESS_APPS

// This is the global image list declaration.
ImageList gImageList;
CString gServerIp;
unsigned short gServerPort;

//-----------------------------------------------------------------------------
// LoadImageList
//-----------------------------------------------------------------------------
void LoadImageList(ImageList& pi_ImageList)
{
	uint32_t ret = 0, nbImage;
	WChar  name[255];
	CString tmp;
	
	// Find the number of images stored in file
	nbImage = GetPrivateProfileInt(IMAGE_SECTION, NB_IMAGE, 0, IMAGE_FILE);
	
	if( nbImage != 0 )
	{
		// Load each image
		for(uint32_t index = 0; index < nbImage; index++)
		{
			// Get the image name
			tmp.Format(_TEXT("%s%u"), IMAGE_NAME, index);
			GetPrivateProfileString(IMAGE_SECTION, tmp, _TEXT(""), name, 254, IMAGE_FILE);
			
			// Put the image in the list
			if( name != _TEXT("") )
			{
				WChar* image;
				image = new WChar[_tcslen(name) + 1];
				
				_tcscpy(image, name);
				pi_ImageList.push_back(image);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// LoadServerInfo
//-----------------------------------------------------------------------------
void LoadServerInfo()
{
	uint32_t ret = 0;
	WChar name[255];
	
	// Find the server ip
	GetPrivateProfileString(SERVER_SECTION, IP, _TEXT(""), name, 254, IMAGE_FILE);
	ret = GetPrivateProfileInt(SERVER_SECTION, PORT, 0, IMAGE_FILE);
	
	gServerIp   = name;
	gServerPort = ret;
}

#endif //STRESS_APPS