/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/HMRFactor.h $
|    $RCSfile: HMRFactor.h,v $
|   $Revision: 1.1 $
|       $Date: 2006/02/14 13:58:42 $
|     $Author: BENTLEY\Simon.Normand $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// HMRFactor.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
        #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"           // main symbols


// HMRFactorApp:
// See HMRFactor.cpp for the implementation of this class
//

class HMRFactorApp : public CWinApp
{
public:
        HMRFactorApp();

// Overrides
        public:
        virtual BOOL InitInstance();

// Implementation

        DECLARE_MESSAGE_MAP()
};

extern HMRFactorApp theApp;