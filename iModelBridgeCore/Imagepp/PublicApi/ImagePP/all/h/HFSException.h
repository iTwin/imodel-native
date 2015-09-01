//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSException.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HFCException.h"

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException ABSTRACT                                 
//      HFSException ABSTRACT                             
//          HFSHIBPInvalidResponseException     
//          HFSHIBPException ABSTRACT
//              HFSHIBPErrorException
//              HFSHIBPProtocolNotSupportedException
//          HFSInvalidPathException ABSTRACT
//              HFSGenericInvalidPathException
//              HFSUrlSchemeNotSupportedException
//              HFSInvalidDirectoryPathException 
//          HFSCannotStartNetResourceEnumException
//          HFSCannotEnumNextNetResourceException
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Class HFSException ABSTRACT
//----------------------------------------------------------------------------
class HFSException : public HFCException
    {
public:
    IMAGEPP_EXPORT virtual ~HFSException();
protected:
    //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    IMAGEPP_EXPORT virtual WString _BuildMessage(const ImagePPExceptions::StringId& rsID) const override;
    IMAGEPP_EXPORT HFSException();
    IMAGEPP_EXPORT HFSException                   (const HFSException&     pi_rObj); 
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<ImagePPExceptions::StringId (*GetStringId)()>
class HFSException_T : public HFSException
    {
public:
    HFSException_T() : HFSException(){}
    HFSException_T (const HFSException_T& pi_rObj) : HFSException(pi_rObj){} 
    virtual HFCException* Clone() const override {return new HFSException_T(*this);}
    virtual void ThrowMyself() const override {throw *this;} 
    virtual WString GetExceptionMessage() const override
        {
        return HFSException::_BuildMessage(GetStringId());
        }
};
typedef HFSException_T<ImagePPExceptions::HFSCannotStartNexNetResourceEnum> HFSCannotStartNetResourceEnumException;
typedef HFSException_T<ImagePPExceptions::HFSCannotEnumNextNetResource> HFSCannotEnumNextNetResourceException;

//----------------------------------------------------------------------------
// Class HFSInvalidPathException ABSTRACT
//----------------------------------------------------------------------------
class HFSInvalidPathException : public HFSException 
    {
public:
    IMAGEPP_EXPORT virtual ~HFSInvalidPathException();
    IMAGEPP_EXPORT WStringCR GetPath () const;
protected:
    //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    IMAGEPP_EXPORT HFSInvalidPathException (const HFSInvalidPathException&     pi_rObj);
    IMAGEPP_EXPORT HFSInvalidPathException(const WString& pi_rPath);
    IMAGEPP_EXPORT virtual WString _BuildMessage(const ImagePPExceptions::StringId& rsID) const override;
    WString  m_Path;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<ImagePPExceptions::StringId (*GetStringId)()>
class HFSInvalidPathException_T : public HFSInvalidPathException
    {
    public:
    HFSInvalidPathException_T(const WString& pi_rPath) : HFSInvalidPathException(pi_rPath){}
    HFSInvalidPathException_T (const HFSInvalidPathException_T& pi_rObj) : HFSInvalidPathException(pi_rObj){} 
    virtual HFCException* Clone() const override {return new HFSInvalidPathException_T(*this);}
    virtual void ThrowMyself() const override {throw *this;} 
    virtual WString GetExceptionMessage() const override
       {
       return HFSInvalidPathException::_BuildMessage(GetStringId());
       }
};
typedef HFSInvalidPathException_T<ImagePPExceptions::HFSGenericInvalidPath> HFSGenericInvalidPathException;
typedef HFSInvalidPathException_T<ImagePPExceptions::HFSUrlSchemeNotSupported> HFSUrlSchemeNotSupportedException;
typedef HFSInvalidPathException_T<ImagePPExceptions::HFSInvalidDirectory> HFSInvalidDirectoryPathException;
END_IMAGEPP_NAMESPACE
