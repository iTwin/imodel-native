//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFException.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException ABSTRACT
//      HGFException ABSTRACT             
//          HGFDomainException
//          HGFmzGCoordException
//----------------------------------------------------------------------------
#pragma once

#include "HFCException.h"

BEGIN_IMAGEPP_NAMESPACE
/*----------------------------------------------------------------------------+
|Class HGFException ABSTRACT
|
|Description: Exception thrown when an error occurred in HGF.
|
+----------------------------------------------------------------------------*/
class HGFException : public HFCException 
    {
public:
    virtual        ~HGFException();  
protected:
    //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    IMAGEPP_EXPORT HGFException                   (const HGFException&     pi_rObj);   
    IMAGEPP_EXPORT HGFException();
    IMAGEPP_EXPORT virtual WString _BuildMessage(const ImagePPExceptions::StringId& rsID) const override;
    };
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<ImagePPExceptions::StringId (*GetStringId)()>
class HGFException_T : public HGFException
{
public:
    HGFException_T() : HGFException(){}
    HGFException_T (const HGFException_T& pi_rObj) : HGFException(pi_rObj){}; 
    virtual HFCException* Clone() const override {return new HGFException_T(*this);}
    virtual void ThrowMyself() const override {throw *this;} 
    virtual WString GetExceptionMessage() const override
        {
        return HGFException::_BuildMessage(GetStringId());
        }
};
typedef HGFException_T<ImagePPExceptions::HGFDomain> HGFDomainException;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
class HGFmzGCoordException : public HGFException
    {
public :
    IMAGEPP_EXPORT HGFmzGCoordException(int32_t pi_StatusCode);
    IMAGEPP_EXPORT virtual ~HGFmzGCoordException();
    IMAGEPP_EXPORT const int32_t GetStatusCode() const;
    IMAGEPP_EXPORT HGFmzGCoordException(const HGFmzGCoordException&     pi_rObj); 
    IMAGEPP_EXPORT virtual WString GetExceptionMessage() const override;
    virtual HFCException* Clone() const override;
    IMAGEPP_EXPORT WString GetErrorText() const;
    virtual void ThrowMyself() const override {throw *this;} 
protected:
    int32_t  m_StatusCode;    
};

END_IMAGEPP_NAMESPACE
