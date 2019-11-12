/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Foundations/Error.h>


BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE

typedef Error                           Exception;

/*---------------------------------------------------------------------------------**//**
* @description  Same as ErrorMixinBase. As template aliases do not exist yet we
*               added this class for clarity purpose.
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ExceptionT>
struct ExceptionMixinBase : public Exception
    {
private:
    virtual Exception*                  _Clone                             () const override
        {
        return new ExceptionT(static_cast<const ExceptionT&>(*this));
        }

protected:
    typedef ExceptionMixinBase<ExceptionT>
                                        super_class;

    explicit                            ExceptionMixinBase                 (const WChar*          msg,
                                                                            StatusInt               errorCode = 0)
        :   Exception(msg, errorCode) {}

    virtual                             ~ExceptionMixinBase                () {};

    };

typedef CustomError                     CustomException;

END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
