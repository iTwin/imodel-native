/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Exceptions.h $
|    $RCSfile: Exceptions.h,v $
|   $Revision: 1.10 $
|       $Date: 2011/08/26 18:47:23 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Foundations/Exception.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE


/*
 * Reprojection exceptions
 */
struct IncompatibleGCSException : public ExceptionMixinBase<IncompatibleGCSException>
    {
    explicit        IncompatibleGCSException                   ()   :   super_class(L"Incompatible GCS error!") {}
    };

struct IncompatibleMathematicalDomainException : public ExceptionMixinBase<IncompatibleMathematicalDomainException>
    {
    explicit        IncompatibleMathematicalDomainException    ()   :   super_class(L"Incompatible mathematical domain error!") {}
    };

struct ReprojectionException : public ExceptionMixinBase<ReprojectionException>
    {
    explicit        ReprojectionException                      (StatusInt code)   :   super_class(L"Reprojection error!", code) {}
    };




END_BENTLEY_MRDTM_IMPORT_NAMESPACE