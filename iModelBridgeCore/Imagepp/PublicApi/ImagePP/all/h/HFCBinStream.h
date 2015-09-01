//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCBinStream.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCBinStream
//---------------------------------------------------------------------------
#pragma once

#include "HFCAccessMode.h"
#include "HFCPtr.h"
#include <imagepp/all/h/HFCException.h>

BEGIN_IMAGEPP_NAMESPACE

class HFCException;
class HFCURL;

class HNOVTABLEINIT HFCBinStream
    {
public:
    HDECLARE_BASECLASS_ID (HFCBinStreamId_Base);

    // Primary methods

    IMAGEPP_EXPORT                        HFCBinStream();
    IMAGEPP_EXPORT virtual                ~HFCBinStream();

    // This static method can be used instead of the constructor.
    // Use it to create a correctly-typed object for specified URL.
    // The parameter pi_NbRetryBeforeThrow is useful only with LocalBinStream presently:
    //   if the value < 0 then Do not throw an exception, call GetLastExceptionID
    //   othersise an exception will be thrown if error.

    IMAGEPP_EXPORT static HFCBinStream*    Instanciate(HFCPtr<HFCURL> pi_pURL,
                                               HFCAccessMode  pi_AccessMode = HFC_NO_ACCESS,
                                               short pi_NbRetry=0 , bool pi_ThrowOnError = false);

    IMAGEPP_EXPORT static HFCBinStream* Instanciate(HFCPtr<HFCURL> pi_pURL, uint64_t pi_offSet, HFCAccessMode  pi_AccessMode = HFC_NO_ACCESS,
                                                    short pi_NbRetry=0, bool pi_ThrowOnError = false);

    // Information methods

    virtual HFCPtr<HFCURL>  GetURL() const = 0;
    virtual uint64_t       GetSize() const = 0;
    virtual HFCAccessMode   GetAccessMode() const = 0;
    IMAGEPP_EXPORT HFCException const*     GetLastException() const;
    IMAGEPP_EXPORT bool             IsOpened() const;
    IMAGEPP_EXPORT void ThrowOnError() const;


    // Multiuser access

    virtual void            Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share) = 0;
    virtual void            Unlock(uint64_t pi_Pos, uint64_t pi_Size) = 0;

    // File pointer management

    virtual void            Seek(int64_t pi_Delta) = 0;
    virtual void            SeekToPos(uint64_t pi_NewPos) = 0;
    virtual void            SeekToBegin() = 0;
    virtual void            SeekToEnd() = 0;
    virtual uint64_t       GetCurrentPos() = 0;
    virtual bool           EndOfFile() = 0;

    // Content access

    void                    SetNoException();
    virtual size_t          Read(void* po_pData, size_t pi_DataSize) = 0;
    virtual size_t          Write(const void* pi_pData, size_t pi_DataSize) = 0;
    virtual bool           SetEOF() = 0;
    virtual bool           Flush() = 0;

    // Type used to register a class in the stream-type list.

    struct Creator
        {
        virtual HFCBinStream* Create(HFCPtr<HFCURL> pi_pURL,
                                     uint64_t pi_offSet,
                                     HFCAccessMode  pi_AccessMode,
                                     short pi_NbRetryBeforeThrow=0) const = 0;
        };




//DM-Android     Not able to build if private member
    // The type of the stream-type list, based on URL scheme type.
    typedef map<WString, Creator*, CaseInsensitiveStringCompare, allocator<Creator*> >  // From protected
        StreamTypeList;
    // Scheme list access
    static StreamTypeList&  GetStreamTypeList();                // From protected

    // Move that in public part to be able to build.
    friend struct BinStreamTypeListDestroyer;           // From private
    // The scheme list.
    static StreamTypeList*  s_pStreamTypeList;          // From private

protected:


    // Internal use by derived classes.
    std::unique_ptr<HFCException>  m_pLastException;

    // true if the BinStream can be used
    bool       m_BinStreamOpen;

private:

    // Disabled methods

    // Removed WChar version to avoid implicit conversion to UTF8. If required, explicitly do the conversion on read or write bytes.
    // We disable the methods here so we can catch the cases that use WChar and take appropriate action.
    //
    // Here are some observations that lead to this decision:
    // - All this was introduced to support non-English filename in PSS. 
    // - The Read(WChar) used to decode to UTF8 only if a BOOM UTF8 was detected. We never found a place were we set the BOOM.
    // - The Write(WChar) used to convert to UTF8 before writing to file.
    // - This Read/Write(WChar) logic was only implemented for HFCLocalBinStream.
    // - PSS was not decoding to UTF8 the entire file but only the image filename portion. Everything else was threated as multi-bytes. 
    //   It worked fine because the rest of the file was only English (char < 128) were present.
    // - Imagepp 8.11 (last cvs version) cannot handle ASCII with French char(c>127). If the French chars are encoded UTF8 then
    //   everything is fine. So we assumed that PSS must always be UTF8. Note that English char(c<128) are compatible with UTF8.
    // - Its the responsibility of the PSS reader/writer to handle the UTF8 conversion.
    size_t Read(WChar* po_pData, size_t pi_DataSize);
    size_t Write(const WChar* pi_pData, size_t pi_DataSize);

    HFCBinStream(const HFCBinStream& pi_rObj);
    HFCBinStream& operator=(const HFCBinStream& pi_rObj);

    };


END_IMAGEPP_NAMESPACE