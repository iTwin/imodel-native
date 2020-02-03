/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/HMRStress.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef HMRStress_H
#define HMRStress_H

// The following code is for performing application testing with
// the internet functionnality.
// To compile this code, STRESS_APPS must be defined.

#ifdef STRESS_APPS

#define IMAGE_FILE	   _TEXT("c:\\image.ini")
#define IMAGE_SECTION  _TEXT("image section")
#define NB_IMAGE	   _TEXT("nb images")
#define IMAGE_NAME     _TEXT("image")
#define SERVER_SECTION _TEXT("server section")
#define IP             _TEXT("IP")
#define PORT           _TEXT("port")

typedef list< WChar*, allocator <WChar*> > ImageList;

// This is the global image list declaration.

void LoadImageList(ImageList& pi_ImageList);
void LoadServerInfo();

extern CString gServerIp;
extern unsigned short gServerPort;
extern ImageList gImageList;

#endif // STRESS_APPS
#endif //HMRStress_H