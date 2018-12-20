/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Foundations/Exception.h $
|    $RCSfile: Exception.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/26 18:46:12 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Foundations/Error.h>


BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE

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

    virtual                             ~ExceptionMixinBase                () = 0 {};

    };

typedef CustomError                     CustomException;

END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE