/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include "IStatus.h"

BEGIN_PCLWRAPPER_NAMESPACE

struct IIcp;

typedef BENTLEY_NAMESPACE_NAME::RefCountedPtr<IIcp> IIcpPtr;

struct IIcp : public BENTLEY_NAMESPACE_NAME::RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/
    protected:


    /*__PUBLISH_SECTION_START__*/
    public:
        PCLWRAPPER_EXPORT virtual IStatusPtr setInputCloud (const DPoint3d* pInputCloud, size_t numberOfPoints) = 0;
        PCLWRAPPER_EXPORT virtual IStatusPtr setInputTarget (const DPoint3d* pInputTarget, size_t numberOfPoints) = 0;
        PCLWRAPPER_EXPORT virtual void align (bvector<DPoint3d>& pFinalCloud) = 0;
        PCLWRAPPER_EXPORT virtual double getFitnessScore() = 0;
        PCLWRAPPER_EXPORT virtual void getFinalTransformation (DMatrix4d& transformation) = 0;
    };

END_PCLWRAPPER_NAMESPACE