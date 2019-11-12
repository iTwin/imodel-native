/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Foundations/Exception.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


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




END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
