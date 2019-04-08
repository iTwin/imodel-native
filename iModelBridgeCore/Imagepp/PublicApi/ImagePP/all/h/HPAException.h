//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPAException.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPAException
//---------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException ABSTRACT
//      HPAException ABSTRACT
//          HPANoTokenException
//          HPARecursiveInclusionException
//          HPACannotFindProductionException
//          HPACannotResolveStartRuleException
//          HPANodeLeftToParseException 
//          HPAGenericException       
//----------------------------------------------------------------------------
#pragma once

#include "HFCException.h"

BEGIN_IMAGEPP_NAMESPACE
class HPANode;

//----------------------------------------------------------------------------
// Class HPAException ABSTRACT
//----------------------------------------------------------------------------
class HPAException : public HFCException 
    {
public:
    IMAGEPP_EXPORT virtual        ~HPAException();
    IMAGEPP_EXPORT Utf8String         MakeErrorMsg() const;
    IMAGEPP_EXPORT Utf8String        GetErrorText() const;
    IMAGEPP_EXPORT const HFCPtr<HPANode>&   GetOffendingNode                       () const;
    protected: 
     //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    IMAGEPP_EXPORT HPAException                   (const HPAException&     pi_rObj);
    IMAGEPP_EXPORT HPAException();
    IMAGEPP_EXPORT HPAException(HFCPtr<HPANode>&  pi_rpOffendingNode);
     HFCPtr<HPANode> m_pOffendingNode;
     IMAGEPP_EXPORT  virtual Utf8String _BuildMessage(const ImagePPExceptions::StringId& rsID) const override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<ImagePPExceptions::StringId (*GetStringId)()>
class HPAException_T : public HPAException
{
    public:
    HPAException_T(HFCPtr<HPANode>&  pi_rpOffendingNode) : HPAException(pi_rpOffendingNode){}
    HPAException_T (const HPAException_T& pi_rObj) : HPAException(pi_rObj){} 
    virtual HFCException* Clone() const override {return new HPAException_T(*this);}
    virtual void ThrowMyself() const override {throw *this;} 
    virtual Utf8String GetExceptionMessage() const override
        {
        return HPAException::_BuildMessage(GetStringId());
        }
};
typedef HPAException_T<ImagePPExceptions::HPANoToken> HPANoTokenException;
typedef HPAException_T<ImagePPExceptions::HPARecursiveInclusion> HPARecursiveInclusionException;
typedef HPAException_T<ImagePPExceptions::HPACannotFindProduction> HPACannotFindProductionException;
typedef HPAException_T<ImagePPExceptions::HPACannotResolveStartRule> HPACannotResolveStartRuleException;
typedef HPAException_T<ImagePPExceptions::HPANodeLeftToParse> HPANodeLeftToParseException;

//----------------------------------------------------------------------------
// Class HPAGenericException
//----------------------------------------------------------------------------
class HPAGenericException : public HPAException
{
public:
    IMAGEPP_EXPORT HPAGenericException(HFCPtr<HPANode>&    pi_rpOffendingNode,
                        const Utf8String&        pi_rMessage);
    HPAGenericException(const HPAException* pi_pObj);

    IMAGEPP_EXPORT virtual        ~HPAGenericException();
    Utf8StringCR    GetMessage                        () const;
    IMAGEPP_EXPORT HPAGenericException (const HPAGenericException&     pi_rObj); 
    virtual Utf8String GetExceptionMessage() const override;
    virtual HFCException* Clone() const override; 
    virtual void ThrowMyself() const override {throw *this;} 
protected: 
    Utf8String            m_Message;      
};
END_IMAGEPP_NAMESPACE
