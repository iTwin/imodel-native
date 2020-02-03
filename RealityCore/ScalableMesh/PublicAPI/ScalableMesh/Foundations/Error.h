/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Foundations/Definitions.h>
#include <memory>

BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Error
    {
private:
    friend struct                       ErrorItem;

    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

    virtual Error*                      _Clone                                     () const = 0;

protected:
    FOUNDATIONS_DLLE explicit           Error                                      (const WChar*            msg, 
                                                                                    StatusInt               code = 0);

    FOUNDATIONS_DLLE                    Error                                      (const Error&            rhs);

    // NTERAY: I don't like the idea of adding this operator, but at first glance it now seem required by the 
    //         vc10 compiler... Try to find another way around as having this would allow for someone with a 
    //         pointer to base to truncate an existing exception instance. This should be derived from Unassignable.
    FOUNDATIONS_DLLE Error&             operator=                                  (const Error&            rhs);

public:
    FOUNDATIONS_DLLE virtual            ~Error                                     () = 0;

    FOUNDATIONS_DLLE StatusInt          GetErrorCode                               () const;
    FOUNDATIONS_DLLE const WChar*       what                                       () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Template base class for error that automatically implement the
*               virtual constructor pattern assuming that derived WarningT provides
*               a valid copy constructor.
*    
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ErrorT>
struct ErrorMixinBase : public Error
    {
private:
    virtual Error*                      _Clone                             () const override
        {
        return new ErrorT(static_cast<const ErrorT&>(*this));
        }

protected:
    typedef ErrorMixinBase<ErrorT>      super_class;

    explicit                            ErrorMixinBase                     (const WChar*          msg,
                                                                            StatusInt               errorCode = 0)
        :   Error(msg, errorCode) {}

    virtual                             ~ErrorMixinBase                    () {};

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CustomError : public ErrorMixinBase<CustomError>
    {
    explicit                            CustomError                        (const WChar*          msg, 
                                                                            StatusInt               code = 0) : ErrorMixinBase<CustomError>(msg, code) {}
    };

END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
