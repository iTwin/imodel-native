//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFException.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Implementation of the HRF exception classes.  The exception hierarchy look
// like this:
//
//  HFCException ABSTRACT
//  HFCFileException ABSTRACT                           
//        HRFException ABSTRACT
//            HRFGenericException
//            HRFBadPageNumberException
//            HRFBadSubImageException
//            HRFPixelTypeNotSupportedException
//            HRFCodecNotSupportedException
//            HRFMultiPageNotSupportedException
//            HRFAccessModeForPixelTypeNotSupportedException
//            HRFAccessModeForCodeNotSupportedException
//            HRFTransfoModelNotSupportedException
//            HRFXCHChannelsAreNotOfTheSameFormatException
//            HRFXCHChannelsDoNotHaveSameCompressionException
//            HRFXCHChannelsIsNotAValidGrayscaleException
//            HRFFXHChannelsDoNotHaveTheSameDimensionsException
//            HRFFXHChannelsDoNotHaveTheSameResCountException
//            HRFNeedOpenPasswordException
//            HRFNeedRestrictionPasswordException
//            HRFInvalidOpenPasswordException
//            HRFInvalidRestrictionPasswordException
//            HRFUnsupportedITiffVersionException
//            HRFGeotiffCompressedTableLockException
//            HRFPWNoHandlerException
//            HRFSisterFileSloNotSupportedException
//            HRFInvalidSisterFileException
//            HRFInvalidTransfoForSisterFileException
//            HRFXCHChannelsBlockDimensionsDifferException
//            HRFUnsupportedxWMSVersionException
//            HRFSloNotSupportedException
//            HRFSubResAccessDifferReadOnlyException
//            HRFIntergraphLutReadOnlyException
//            HRFPixelTypeCodecNotSupportedException
//            HRFAnimatedGifReadOnlyException
//            HRFMoreThan24TiePointReadOnlyException
//            HRFUnsupportedBMPVersionException
//            HRFTransfoCannotBeAMatrixException
//            HRFUnsupportedHGRVersionException
//            HRFCannotDownloadToInternetCacheException
//            HRFERSUnmatchRegSpaceCoordTypexception
//            HRFERSUnmatchRegCoordTypeException
//            HRFTooSmallForEcwCompressionException
//            HRFTooBigForEcwCompressionException
//            HRFInvalidExportOptionException
//            HRFOperationRestrictedPDFNotSupportedException
//            HRFDataHaveBeenScaledReadOnlyException
//            HRFAuthenticationMaxRetryCountReachedException
//            HRFAuthenticationCancelledException
//            HRFAuthenticationInvalidLoginException
//            HRFAuthenticationInvalidServiceException
//            HRFChildFileException ABSTRACT              
//                HRFChildFileParameterException
//                HRFCannotOpenChildFileException
//            HRFFileParameterException ABSTRACT
//                HRFSisterFileInvalidParamValueException
//                HRFSisterFileMissingParamException
//                HRFSisterFileMissingParamGroupException
//                HRFInvalidParamValueException
//                HRFMimeFormatNotSupportedException
//                HRFMissingParameterException
//                HRFUSGSBandNotFoundInHeaderFileException
//                HRFERSDataFoundOutsideDatasetHeaderException
//            HRFTiffErrorException        
//            HRFInvalidNewFileDimensionException
//----------------------------------------------------------------------------
#pragma once

#include "HFCException.h"
#include "HTIFFUtils.h"

//----------------------------------------------------------------------------
// Class HRFException ABSTRACT
//----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRFException : public HFCFileException 
{
public:
    IMAGEPP_EXPORT virtual         ~HRFException();
protected:
    //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    IMAGEPP_EXPORT HRFException            (const WString&          pi_rFileName);
    IMAGEPP_EXPORT HRFException                   (const HRFException&     pi_rObj);
    IMAGEPP_EXPORT virtual WString            _BuildMessage(const ImagePPExceptions::StringId& rsId) const override;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<ImagePPExceptions::StringId (*GetStringId)(), bool IsInvalidAccessMode>
class HRFException_T : public HRFException
{
public:
    HRFException_T(const WString& pi_rFileName) : HRFException(pi_rFileName){}
    HRFException_T (const HRFException_T& pi_rObj) : HRFException(pi_rObj){}
    virtual HFCException* Clone() const override {return new HRFException_T(*this);}
    virtual void ThrowMyself() const {throw *this;}
    virtual bool IsInvalidAccess() const override {return IsInvalidAccessMode;} 
    virtual WString GetExceptionMessage()const override
        {
        return HRFException::_BuildMessage(GetStringId());
        }
};

typedef HRFException_T<ImagePPExceptions::HRFGeneric, false> HRFGenericException;
typedef HRFException_T<ImagePPExceptions::HRFBadPageNumber, false> HRFBadPageNumberException;
typedef HRFException_T<ImagePPExceptions::HRFBadSubImage, false> HRFBadSubImageException;
typedef HRFException_T<ImagePPExceptions::HRFPixelTypeNotSupported, false> HRFPixelTypeNotSupportedException;
typedef HRFException_T<ImagePPExceptions::HRFCodecNotSupported, false> HRFCodecNotSupportedException;
typedef HRFException_T<ImagePPExceptions::HRFMultiPageNotSupported, false> HRFMultiPageNotSupportedException;
typedef HRFException_T<ImagePPExceptions::HRFAccessModeForPixelTypeNotSupported, true> HRFAccessModeForPixelTypeNotSupportedException;
typedef HRFException_T<ImagePPExceptions::HRFAccessModeForCodecNotSupported, true> HRFAccessModeForCodeNotSupportedException;
typedef HRFException_T<ImagePPExceptions::HRFTransfoModelNotSupported, false> HRFTransfoModelNotSupportedException;
typedef HRFException_T<ImagePPExceptions::HRFXCHChannelsAreNotOfTheSameFormat, false> HRFXCHChannelsAreNotOfTheSameFormatException;
typedef HRFException_T<ImagePPExceptions::HRFXCHChannelsDoNotHaveSameCompression, false> HRFXCHChannelsDoNotHaveSameCompressionException;
typedef HRFException_T<ImagePPExceptions::HRFXCHChannelsIsNotAValidGrayscale, false> HRFXCHChannelsIsNotAValidGrayscaleException;
typedef HRFException_T<ImagePPExceptions::HRFFXHChannelsDoNotHaveTheSameDimensions, false> HRFFXHChannelsDoNotHaveTheSameDimensionsException;
typedef HRFException_T<ImagePPExceptions::HRFFXHChannelsDoNotHaveTheSameResCount, false> HRFFXHChannelsDoNotHaveTheSameResCountException;
typedef HRFException_T<ImagePPExceptions::HRFNeedOpenPassword, false> HRFNeedOpenPasswordException;
typedef HRFException_T<ImagePPExceptions::HRFNeedRestrictionPassword, false> HRFNeedRestrictionPasswordException;
typedef HRFException_T<ImagePPExceptions::HRFInvalidOpenPassword, false> HRFInvalidOpenPasswordException;
typedef HRFException_T<ImagePPExceptions::HRFInvalidRestrictionPassword, false> HRFInvalidRestrictionPasswordException;
typedef HRFException_T<ImagePPExceptions::HRFUnsupportedITiffVersion, false> HRFUnsupportedITiffVersionException;
typedef HRFException_T<ImagePPExceptions::HRFGeotiffCompressedTableLock, false> HRFGeotiffCompressedTableLockException;
typedef HRFException_T<ImagePPExceptions::HRFPWNoHandler, false> HRFPWNoHandlerException;
typedef HRFException_T<ImagePPExceptions::HRFSisterFileSlotNotSupported, false> HRFSisterFileSloNotSupportedException;
typedef HRFException_T<ImagePPExceptions::HRFInvalidSisterFile, false> HRFInvalidSisterFileException;
typedef HRFException_T<ImagePPExceptions::HRFInvalidTransfoForSisterFile, false> HRFInvalidTransfoForSisterFileException;
typedef HRFException_T<ImagePPExceptions::HRFXCHChannelsBlockDimensionsDiffer, false> HRFXCHChannelsBlockDimensionsDifferException;
typedef HRFException_T<ImagePPExceptions::HRFUnsupportedxWMSVersion, false> HRFUnsupportedxWMSVersionException;
typedef HRFException_T<ImagePPExceptions::HRFSloNotSupported, false> HRFSloNotSupportedException;
typedef HRFException_T<ImagePPExceptions::HRFSubResAcessDifferReadOnly, true> HRFSubResAccessDifferReadOnlyException;
typedef HRFException_T<ImagePPExceptions::HRFIntergraphLutReadOnly, true> HRFIntergraphLutReadOnlyException;
typedef HRFException_T<ImagePPExceptions::HRFPixelTypeCodecNotSupported, false> HRFPixelTypeCodecNotSupportedException;
typedef HRFException_T<ImagePPExceptions::HRFAnimatedGifReadOnly, true> HRFAnimatedGifReadOnlyException;
typedef HRFException_T<ImagePPExceptions::HRFMoreThan24TiePointReadOnly, true> HRFMoreThan24TiePointReadOnlyException;
typedef HRFException_T<ImagePPExceptions::HRFUnsupportedBMPVersion, false> HRFUnsupportedBMPVersionException;
typedef HRFException_T<ImagePPExceptions::HRFTransfoCannotBeAMatrix, false> HRFTransfoCannotBeAMatrixException;
typedef HRFException_T<ImagePPExceptions::HRFUnsupportedHGRVersion, false> HRFUnsupportedHGRVersionException;
typedef HRFException_T<ImagePPExceptions::HRFCannotDownloadToInternetCache, false> HRFCannotDownloadToInternetCacheException;
typedef HRFException_T<ImagePPExceptions::HRFERSUnmatchRegSpaceCoordType, false> HRFERSUnmatchRegSpaceCoordTypexception;
typedef HRFException_T<ImagePPExceptions::HRFERSUnmatchRegCoordType, false> HRFERSUnmatchRegCoordTypeException;
typedef HRFException_T<ImagePPExceptions::HRFTooSmallForEcwCompression, false> HRFTooSmallForEcwCompressionException;
typedef HRFException_T<ImagePPExceptions::HRFTooBigForEcwCompression, false> HRFTooBigForEcwCompressionException;
typedef HRFException_T<ImagePPExceptions::HRFInvalidExportOption, false> HRFInvalidExportOptionException;
typedef HRFException_T<ImagePPExceptions::HRFOperationRestrictedPDFNotSupported, false> HRFOperationRestrictedPDFNotSupportedException;
typedef HRFException_T<ImagePPExceptions::HRFDataHaveBeenScaleReadOnly, true> HRFDataHaveBeenScaledReadOnlyException;
typedef HRFException_T<ImagePPExceptions::HRFAuthenticationMaxRetryCountReadched, false> HRFAuthenticationMaxRetryCountReachedException;
typedef HRFException_T<ImagePPExceptions::HRFAuthenticationCancelled, false> HRFAuthenticationCancelledException;
typedef HRFException_T<ImagePPExceptions::HRFAuthenticationInvalidLogin, false> HRFAuthenticationInvalidLoginException;
typedef HRFException_T<ImagePPExceptions::HRFAuthenticationInvalidService, false> HRFAuthenticationInvalidServiceException;

//----------------------------------------------------------------------------
// Class HRFFileParameterException ABSTRACT
//----------------------------------------------------------------------------
class HRFFileParameterException : public HRFException
    {
public:
    virtual        ~HRFFileParameterException();
    WStringCR    GetParameterName                        () const; 
protected:
    //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    HRFFileParameterException(const WString& pi_rFileName,const WString& pi_rParamName);
    HRFFileParameterException                   (const HRFFileParameterException&     pi_rObj);
    WString m_ParameterName;
    virtual WString _BuildMessage(const ImagePPExceptions::StringId& rsID) const override;
private:
    virtual bool HasFilenameInMessageString() const {return false;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<ImagePPExceptions::StringId (*GetStringId)(), bool HasFilenameInMessageStringTemplateParam>
class HRFFileParameterException_T : public HRFFileParameterException
    {
public:
    HRFFileParameterException_T(const WString& pi_rFileName, const WString& pi_rParamName) : HRFFileParameterException(pi_rFileName, pi_rParamName){}
    HRFFileParameterException_T(const HRFFileParameterException_T& pi_rObj) : HRFFileParameterException(pi_rObj){}
    virtual HFCException* Clone() const override {return new HRFFileParameterException_T(*this);}
    virtual void ThrowMyself() const override {throw *this;} 
    virtual WString GetExceptionMessage() const override
        {
        return HRFFileParameterException::_BuildMessage(GetStringId());
        }
private:
    virtual bool HasFilenameInMessageString() const override
        {
        return HasFilenameInMessageStringTemplateParam;
        }
    };

typedef HRFFileParameterException_T<ImagePPExceptions::HRFSisterFileInvalidParamValue, true> HRFSisterFileInvalidParamValueException;
typedef HRFFileParameterException_T<ImagePPExceptions::HRFSisterFileMissing, true> HRFSisterFileMissingParamException;
typedef HRFFileParameterException_T<ImagePPExceptions::HRFSisterFileMissingParamGroup, true> HRFSisterFileMissingParamGroupException;
typedef HRFFileParameterException_T<ImagePPExceptions::HRFInvalidParamValue, false> HRFInvalidParamValueException;
typedef HRFFileParameterException_T<ImagePPExceptions::HRFMimeFormatNotSupported, false> HRFMimeFormatNotSupportedException;
typedef HRFFileParameterException_T<ImagePPExceptions::HRFMissingParameter, false> HRFMissingParameterException;
typedef HRFFileParameterException_T<ImagePPExceptions::HRFUSGSBandNotFoundInHeaderFile, false> HRFUSGSBandNotFoundInHeaderFileException;
typedef HRFFileParameterException_T<ImagePPExceptions::HRFERSDataFoundOutsideDatasetHeader, true> HRFERSDataFoundOutsideDatasetHeaderException;


//----------------------------------------------------------------------------
// Class HRFChildFileException ABSTRACT
//----------------------------------------------------------------------------
class HRFChildFileException : public HRFException 
{
public:
    virtual        ~HRFChildFileException();
    WStringCR    GetChildFileName                        () const;
protected:
    //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    HRFChildFileException(const WString& pi_rParentFileName,const WString& pi_rChildFileName);
    HRFChildFileException                   (const HRFChildFileException&     pi_rObj); 
        WString            m_ChildFileName;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
class HRFCannotOpenChildFileException : public HRFChildFileException 
    {
public:
    HRFCannotOpenChildFileException(const WString& pi_rParentFileName,
                          const WString& pi_rChildFileName);
    virtual HFCException* Clone() const override;
    virtual        ~HRFCannotOpenChildFileException();
    HRFCannotOpenChildFileException                   (const HRFCannotOpenChildFileException&     pi_rObj); 
    virtual WString       GetExceptionMessage() const override;
    virtual void ThrowMyself() const override {throw *this;} 
    };


//----------------------------------------------------------------------------
// Class HRFChildFileParameterException
//----------------------------------------------------------------------------
class HRFChildFileParameterException : public HRFChildFileException 
{
public:
    HRFChildFileParameterException(const WString& pi_rParentFileName, const WString& pi_rChildFileName,
                                   const WString& pi_rParamName);
    virtual        ~HRFChildFileParameterException();
    virtual HFCException* Clone() const override; 
    virtual void ThrowMyself() const override {throw *this;} 
    WStringCR      GetParameterName() const;
    HRFChildFileParameterException (const HRFChildFileParameterException&     pi_rObj); 
    virtual WString GetExceptionMessage() const override;
protected: 
    WString m_ParameterName;   
};

//----------------------------------------------------------------------------
// Class HRFTiffErrorException
//----------------------------------------------------------------------------
class HRFTiffErrorException : public HRFException
{
public:
    HRFTiffErrorException(const WString&    pi_rFileName, const HTIFFError& pi_rErrInfo);
    virtual        ~HRFTiffErrorException();
    const HTIFFError    GetError                        () const;
    HRFTiffErrorException                   (const HRFTiffErrorException&     pi_rObj); 
    virtual WString GetExceptionMessage() const override;
    virtual HFCException* Clone() const override; 
    virtual void ThrowMyself() const override {throw *this;} 
protected:
    HTIFFError m_ErrorInfo;
};

//-----------------------------------------------------------------------------
// HRFInvalidNewFileDimensionException class
//-----------------------------------------------------------------------------
class HRFInvalidNewFileDimensionException : public HRFException
{
public :
    HRFInvalidNewFileDimensionException(const WString& pi_rFileName, uint64_t pi_WidthLimit, uint64_t pi_HeightLimit);
    virtual ~HRFInvalidNewFileDimensionException();
    const uint64_t GetWidthLimit () const;
    const uint64_t GetHeightLimit () const;
    HRFInvalidNewFileDimensionException (const HRFInvalidNewFileDimensionException&     pi_rObj);
    virtual WString GetExceptionMessage() const override;
    virtual HFCException* Clone() const override; 
    virtual void ThrowMyself() const override {throw *this;} 
protected:
    uint64_t m_WidthLimit;
    uint64_t m_HeightLimit;
};
END_IMAGEPP_NAMESPACE
