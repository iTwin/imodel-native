//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPSException.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Class : HPSException
//---------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException ABSTRACT
//      HPSException ABSTRACT                   
//            HPSTypeMismatchException  
//            HPSOutOfRangeException    
//            HPSAlreadyDefinedException
//            HPSIncludeNotFoundException
//            HPSInvalidObjectException
//            HPSQualigierExpectedException
//            HPSExpressionExpectedException
//            HPSInvalidNumericException
//            HPSInvalidUrlException
//            HPSFileNotFoundException
//            HPSNoImageInFileException
//            HPSAlphaPaletteNotSupportedException
//            HPSInvalidWorldException
//            HPSTooFewParamException
//            HPSTooManyParamException
//            HPSTransfoParameterInvalidException
//            HPSImageHasNoSizeException
//            HPSInvalidCoordsException
//            HPSPageNotFoundException
//            HPSInvalidCPolygonException
//            HPSShapeExpectedException
//            HPSWorldAlreadyUsedException
//            HPSTranslucentInfoNotFoundException
//----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HPAException.h>

BEGIN_IMAGEPP_NAMESPACE
//----------------------------------------------------------------------------
// Class HPSException ABSTRACT
//----------------------------------------------------------------------------
class HPSException : public HPAException 
    {
public:
    virtual        ~HPSException();
protected:
    //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    HPSException(HFCPtr<HPANode>   pi_pOffendingNode);
    HPSException                   (const HPSException&     pi_rObj);
    virtual WString _BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const override;
    };
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<ImagePPExceptions::StringId (*GetStringId)()>
class HPSException_T : public HPSException
{
public:
    HPSException_T(HFCPtr<HPANode>  pi_rpOffendingNode) : HPSException(pi_rpOffendingNode){}
    HPSException_T (const HPSException_T& pi_rObj) : HPSException(pi_rObj){} 
    virtual HFCException* Clone() const override {return new HPSException_T(*this);}
    virtual void ThrowMyself() const override {throw *this;} 
    virtual WString GetExceptionMessage()const override
        {
        return HPSException::_BuildMessage(GetStringId());
        }
};
typedef HPSException_T<ImagePPExceptions::HPSIncludeNotFound> HPSIncludeNotFoundException;
typedef HPSException_T<ImagePPExceptions::HPSInvalidObject> HPSInvalidObjectException;
typedef HPSException_T<ImagePPExceptions::HPSQualifierExpected> HPSQualifierExpectedException;
typedef HPSException_T<ImagePPExceptions::HPSExpressionExpected> HPSExpressionExpectedException;
typedef HPSException_T<ImagePPExceptions::HPSInvalidNumeric> HPSInvalidNumericException;
typedef HPSException_T<ImagePPExceptions::HPSInvalidUrl> HPSInvalidUrlException;
typedef HPSException_T<ImagePPExceptions::HPSFileNotFound> HPSFileNotFoundException;
typedef HPSException_T<ImagePPExceptions::HPSNoImageInFile> HPSNoImageInFileException;
typedef HPSException_T<ImagePPExceptions::HPSAlphaPaletteNotSupported> HPSAlphaPaletteNotSupportedException;
typedef HPSException_T<ImagePPExceptions::HPSInvalidWorld> HPSInvalidWorldException;
typedef HPSException_T<ImagePPExceptions::HPSTooFewParam> HPSTooFewParamException;
typedef HPSException_T<ImagePPExceptions::HPSTooManyParam> HPSTooManyParamException;
typedef HPSException_T<ImagePPExceptions::HPSTransfoParameterInvalid> HPSTransfoParameterInvalidException;
typedef HPSException_T<ImagePPExceptions::HPSImageHasNoSize> HPSImageHasNoSizeException;
typedef HPSException_T<ImagePPExceptions::HPSInvalidCoord> HPSInvalidCoordsException;
typedef HPSException_T<ImagePPExceptions::HPSPageNotFound> HPSPageNotFoundException;
typedef HPSException_T<ImagePPExceptions::HPSInvalidPolygon> HPSInvalidCPolygonException;
typedef HPSException_T<ImagePPExceptions::HPSShapeExpected> HPSShapeExpectedException;
typedef HPSException_T<ImagePPExceptions::HPSWorldAlreadyUsed> HPSWorldAlreadyUsedException;
typedef HPSException_T<ImagePPExceptions::HPSTranslucentInfoNotFound> HPSTranslucentInfoNotFoundException;

//----------------------------------------------------------------------------
// Class HPSTypeMismatchException
//----------------------------------------------------------------------------
class HPSTypeMismatchException : public HPSException 
{
public:
    enum ExpectedType
        {
        STRING,
        NUMBER,
        OBJECT,
        IMAGE,
        TRANSFO,
        WORLD,
        COLOR_SET,
        OBJECT_OR_NUMBER,
        IMAGE_CONTEXT,
        FILTER,
        INTEGER,
        QUALIFIER
        };

    HPSTypeMismatchException(HFCPtr<HPANode>    pi_pOffendingNode, ExpectedType        pi_ExpectedType);
    virtual        ~HPSTypeMismatchException();
    const ExpectedType    GetExpectedType                        () const;
    HPSTypeMismatchException                   (const HPSTypeMismatchException&     pi_rObj);
    virtual HFCException* Clone() const override; 
    virtual WString GetExceptionMessage() const override;
    virtual void ThrowMyself() const override {throw *this;} 
protected: 
    ExpectedType m_ExpectedType;
};

//----------------------------------------------------------------------------
// Class HPSOutOfRangeException
//----------------------------------------------------------------------------
class HPSOutOfRangeException : public HPSException
    {
public:
    HPSOutOfRangeException(HFCPtr<HPANode>         pi_pOffendingNode, double                pi_Lower,
                           double              pi_Upper);
    virtual        ~HPSOutOfRangeException();
    const double    GetLower                        () const;
    const double    GetUpper                        () const;
    HPSOutOfRangeException                   (const HPSOutOfRangeException&     pi_rObj); 
    virtual WString GetExceptionMessage() const override;
    virtual HFCException* Clone() const override; 
    virtual void ThrowMyself() const override {throw *this;} 
    protected:
    double         m_Lower;
    double         m_Upper;
    };

//----------------------------------------------------------------------------
// Class HPSAlreadyDefinedException
//----------------------------------------------------------------------------
class HPSAlreadyDefinedException : public HPSException
    {
public:
    HPSAlreadyDefinedException(HFCPtr<HPANode>        pi_pOffendingNode, const WString&       pi_rName);
    virtual        ~HPSAlreadyDefinedException();
    WStringCR    GetName                        () const;
    HPSAlreadyDefinedException                   (const HPSAlreadyDefinedException&     pi_rObj); 
    virtual WString GetExceptionMessage() const override;
    virtual HFCException* Clone() const override; 
    virtual void ThrowMyself() const override {throw *this;} 
    protected :
    WString         m_Name;   
    };

END_IMAGEPP_NAMESPACE
