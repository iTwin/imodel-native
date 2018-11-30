//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDException.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the HRF exception classes.  The exception hierarchy look
// like this:
//
//  HFCException ABSTRACT
//      HCDException ABSTRACT                
//          HCDIJLErrorException    
//          HCDIJGLibraryErrorException
//          HCDCorruptedPackbitsDataException
//----------------------------------------------------------------------------

#pragma once

#include "HFCException.h"

BEGIN_IMAGEPP_NAMESPACE

//----------------------------------------------------------------------------
// Class HCDException ABSTRACT
//----------------------------------------------------------------------------
class HCDException : public HFCException 
{
public:
    virtual ~HCDException();
protected:
    //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    HCDException();
    HCDException (const HCDException& pi_rObj);
    virtual Utf8String _BuildMessage(const ImagePPExceptions::StringId& pi_ID) const override;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<ImagePPExceptions::StringId (*GetStringId)()>
class HCDException_T : public HCDException
{
public:
    HCDException_T() : HCDException(){}
    HCDException_T (const HCDException_T& pi_rObj) : HCDException(pi_rObj){}
    virtual HFCException* Clone() const override {return new HCDException_T(*this);}
    virtual void ThrowMyself() const override {throw *this;} 
    virtual Utf8String GetExceptionMessage() const override
        {
        return HCDException::_BuildMessage(GetStringId());
        }
};

typedef HCDException_T<ImagePPExceptions::HCDIJGLibraryError> HCDIJGLibraryErrorException;
typedef HCDException_T<ImagePPExceptions::HCDCorruptedPackbitsData> HCDCorruptedPackbitsDataException;

//----------------------------------------------------------------------------
// Class HCDException
//----------------------------------------------------------------------------
class HCDIJLErrorException : public HCDException
{
public:
    HCDIJLErrorException(int16_t pi_IJLErrorCode);
    virtual ~HCDIJLErrorException();
    const int16_t GetErrorCode() const;
    HCDIJLErrorException (const HCDIJLErrorException&     pi_rObj); 
    virtual Utf8String GetExceptionMessage() const override; 
    virtual HFCException* Clone() const override; 
    virtual void ThrowMyself() const override {throw *this;} 
protected: 
    int16_t m_IJLErrorCode;  
};

END_IMAGEPP_NAMESPACE
