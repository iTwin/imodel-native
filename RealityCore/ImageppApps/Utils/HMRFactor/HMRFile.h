/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/HMRFile.h $
|    $RCSfile: HMRFile.h,v $
|   $Revision: 1.2 $
|       $Date: 2006/02/14 15:15:03 $
|     $Author: BENTLEY\Simon.Normand $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Imagepp/all/h/HFCPtr.h>

#pragma once

class HMRFile
{
    public:
        HMRFile (TCHAR* pFilename);
        virtual ~HMRFile();

        bool ApplyFactor (double factor);

        int  GetStatus () {return m_Status;};

    private:
        HFCPtr<HRFHMRFile>  m_pHRFHMRFile;
        int                 m_Status;

};