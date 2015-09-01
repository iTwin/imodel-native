//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCStat.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCStat
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURLHTTP.h>
#include <Imagepp/all/h/HFCURLHTTPS.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLMemFile.h>
#include <Imagepp/all/h/HFCURLEmbedFile.h>
#include <Imagepp/all/h/interface/IFileReaderHandler.h>

//-----------------------------------------------------------------------------
// Static member Initialization
//-----------------------------------------------------------------------------

HFCStat::ImplList* HFCStat::s_pImplList = 0;
struct HFCStatImplListDestroyer
    {
    ~HFCStatImplListDestroyer()
        {
        delete HFCStat::s_pImplList;
        HFCStat::s_pImplList = 0;
        }
    } s_ImplListDestroyer;

//-----------------------------------------------------------------------------
// File Implementation Declaration
//-----------------------------------------------------------------------------

class HFCStatFileImpl : public HFCStatImpl
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    HFCStatFileImpl();
    virtual             ~HFCStatFileImpl();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Indicates if an impl can handle an URL
    virtual bool       CanHandle(const HFCURL& pi_rURL) const override;

    // Resource time
    virtual time_t      GetCreationTime(const HFCURL& pi_rURL) const override;
    virtual time_t      GetLastAccessTime(const HFCURL& pi_rURL) const override;
    virtual time_t      GetModificationTime(const HFCURL& pi_rURL) const override;
    virtual void        SetModificationTime(const HFCURL& pi_rURL, time_t pi_NewTime) const override;

    // Resource size
    virtual uint64_t   GetSize(const HFCURL& pi_rURL) const override;

    virtual HFCStat::AccessStatus
    DetectAccess(const HFCURL& pi_rURL) const override;

    // Resource access mode
    virtual HFCAccessMode
    GetAccessMode(const HFCURL& pi_rURL) const override;
    };

//-----------------------------------------------------------------------------
// Instantiate the file implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// File Implementation Definition
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HFCStatFile::Register()
    {
    static HFCStatFileImpl s_impl;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCStatFileImpl::HFCStatFileImpl()
    {
    RegisterImpl(this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCStatFileImpl::~HFCStatFileImpl()
    {
    UnregisterImpl(this);
    }


//-----------------------------------------------------------------------------
// Public
// Indicates if an impl can handle an URL
//-----------------------------------------------------------------------------
bool HFCStatFileImpl::CanHandle(const HFCURL& pi_rURL) const
    {
    return (pi_rURL.GetSchemeType() == HFCURLFile::s_SchemeName());
    }



//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatFileImpl::GetCreationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t    Result = 0;

    // Build a string filename
    WString FileName(((HFCURLFile&)pi_rURL).GetHost());
    FileName += L"\\";
    FileName += ((HFCURLFile&)pi_rURL).GetPath();

    BeFileName::GetFileTime (&Result, 0, 0, FileName.c_str());

    return (Result);
    }

//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatFileImpl::GetLastAccessTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t Result = 0;

    // Build a string filename
    WString FileName(((HFCURLFile&)pi_rURL).GetHost());
    FileName += L"\\";
    FileName += ((HFCURLFile&)pi_rURL).GetPath();

    BeFileName::GetFileTime (0, &Result, 0, FileName.c_str());
    
    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatFileImpl::GetModificationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t Result = 0;

    // Build a string filename
    WString FileName(((HFCURLFile&)pi_rURL).GetHost());
    FileName += L"\\";
    FileName += ((HFCURLFile&)pi_rURL).GetPath();

    BeFileName::GetFileTime (0, 0, &Result, FileName.c_str());
    
    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCStatFileImpl::SetModificationTime(const HFCURL& pi_rURL,
                                          time_t        pi_NewTime) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    // Build a string filename
    WString FileName(((HFCURLFile&)pi_rURL).GetHost());
    FileName += L"\\";
    FileName += ((HFCURLFile&)pi_rURL).GetPath();

    BeFileName::SetFileTime (FileName.c_str(), 0, &pi_NewTime);
    }


//-----------------------------------------------------------------------------
// Public
//
// Resource size
//-----------------------------------------------------------------------------
uint64_t HFCStatFileImpl::GetSize(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    uint64_t FileSize = 0;
    // Build a string filename
    WString FileName(((HFCURLFile&)pi_rURL).GetHost());
    FileName += L"\\";
    FileName += ((HFCURLFile&)pi_rURL).GetPath();

    BeFileName::GetFileSize(FileSize, FileName.c_str());

    return FileSize;
    }

//-----------------------------------------------------------------------------
// Public
// detect access
//-----------------------------------------------------------------------------
HFCStat::AccessStatus HFCStatFileImpl::DetectAccess(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    // Build a string filename
    WString FileName(((HFCURLFile&)pi_rURL).GetHost());
    FileName += L"\\";
    FileName += ((HFCURLFile&)pi_rURL).GetPath();

    // Check if the file exist.
    if (BeFileName::DoesPathExist(FileName.c_str()))
        return HFCStat::AccessGranted;
    else
        return HFCStat::TargetNotFound;
    }


//-----------------------------------------------------------------------------
// Public
// Resource access mode
//-----------------------------------------------------------------------------
HFCAccessMode HFCStatFileImpl::GetAccessMode(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    HFCAccessMode Result = HFC_NO_ACCESS;

    // Build a string filename
    WString FileName(((HFCURLFile&)pi_rURL).GetHost());
    FileName += L"\\";
    FileName += ((HFCURLFile&)pi_rURL).GetPath();

    if (IsExistent(pi_rURL))
        {
        // Try to find the access mode of the specified file.
        if (BeFileName::CheckAccess(FileName.c_str(), BeFileNameAccess::ReadWrite) == BeFileNameStatus::Success)
            Result = HFC_READ_WRITE_OPEN;
        else
            Result = HFC_READ_ONLY;
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Mem file Implementation Declaration
//-----------------------------------------------------------------------------

class HFCStatMemFileImpl : public HFCStatImpl
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    HFCStatMemFileImpl();
    virtual             ~HFCStatMemFileImpl();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Indicates if an impl can handle an URL
    virtual bool       CanHandle(const HFCURL& pi_rURL) const override;

    // Resource time
    virtual time_t      GetCreationTime(const HFCURL& pi_rURL) const override;
    virtual time_t      GetLastAccessTime(const HFCURL& pi_rURL) const override;
    virtual time_t      GetModificationTime(const HFCURL& pi_rURL) const override;
    virtual void        SetModificationTime(const HFCURL& pi_rURL, time_t pi_NewTime) const override;

    // Resource size
    virtual uint64_t   GetSize(const HFCURL& pi_rURL) const override;

    HFCStat::AccessStatus
    DetectAccess(const HFCURL& pi_rURL) const override;

    // Resource access mode
    virtual HFCAccessMode
    GetAccessMode(const HFCURL& pi_rURL) const override;
    };

//-----------------------------------------------------------------------------
// Instantiate the file implementation
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Mem File Implementation Definition
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HFCStatMemFile::Register()
    {
    static HFCStatMemFileImpl s_impl;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCStatMemFileImpl::HFCStatMemFileImpl()
    {
    RegisterImpl(this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCStatMemFileImpl::~HFCStatMemFileImpl()
    {
    UnregisterImpl(this);
    }


//-----------------------------------------------------------------------------
// Public
// Indicates if an impl can handle an URL
//-----------------------------------------------------------------------------
bool HFCStatMemFileImpl::CanHandle(const HFCURL& pi_rURL) const
    {
    return (pi_rURL.GetSchemeType() == HFCURLMemFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatMemFileImpl::GetCreationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    const HFCURLMemFile& memFileURLConst = reinterpret_cast<const HFCURLMemFile&>(pi_rURL);
    time_t Result = memFileURLConst.GetCreationTime();

    return (Result);
    }

//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatMemFileImpl::GetLastAccessTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    //Don't know last access time, just return last modification time
    const HFCURLMemFile& memFileURLConst = reinterpret_cast<const HFCURLMemFile&>(pi_rURL);
    time_t Result = memFileURLConst.GetModificationTime();
    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatMemFileImpl::GetModificationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    const HFCURLMemFile& memFileURLConst = reinterpret_cast<const HFCURLMemFile&>(pi_rURL);
    time_t Result = memFileURLConst.GetModificationTime();
    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCStatMemFileImpl::SetModificationTime(const HFCURL& pi_rURL,
                                             time_t        pi_NewTime) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    const HFCURLMemFile& memFileURLConst = reinterpret_cast<const HFCURLMemFile&>(pi_rURL);
    HFCURLMemFile& memFileURL = const_cast<HFCURLMemFile&>(memFileURLConst);
    memFileURL.SetModificationTime(pi_NewTime);
    }


//-----------------------------------------------------------------------------
// Public
//
// Resource size
//-----------------------------------------------------------------------------
uint64_t HFCStatMemFileImpl::GetSize(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    const HFCURLMemFile& memFileURLConst = reinterpret_cast<const HFCURLMemFile&>(pi_rURL);
    HFCURLMemFile& memFileURL = const_cast<HFCURLMemFile&>(memFileURLConst);

    uint64_t FileSize(memFileURL.GetBuffer()!=NULL ? memFileURL.GetBuffer()->GetDataSize() : 0);

    return FileSize;
    }

//-----------------------------------------------------------------------------
// Public
// Detect existence
//-----------------------------------------------------------------------------
HFCStat::AccessStatus HFCStatMemFileImpl::DetectAccess(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    const HFCURLMemFile& memFileURLConst = reinterpret_cast<const HFCURLMemFile&>(pi_rURL);
    HFCURLMemFile& memFileURL = const_cast<HFCURLMemFile&>(memFileURLConst);

    if(memFileURL.GetBuffer() != NULL && memFileURL.GetBuffer()->GetDataSize() > 0)
        return HFCStat::AccessGranted;

    return HFCStat::TargetNotFound;
    }

//-----------------------------------------------------------------------------
// Public
// Resource access mode
//-----------------------------------------------------------------------------
HFCAccessMode HFCStatMemFileImpl::GetAccessMode(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    HFCAccessMode Result = HFC_READ_WRITE;

    return (Result);
    }

//-----------------------------------------------------------------------------
// Store file Implementation Declaration
//-----------------------------------------------------------------------------

class HFCStatEmbedFileImpl : public HFCStatImpl
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    HFCStatEmbedFileImpl();
    virtual             ~HFCStatEmbedFileImpl();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Indicates if an impl can handle an URL
    virtual bool       CanHandle(const HFCURL& pi_rURL) const override;

    // Resource time
    virtual time_t      GetCreationTime(const HFCURL& pi_rURL) const override;
    virtual time_t      GetLastAccessTime(const HFCURL& pi_rURL) const override;
    virtual time_t      GetModificationTime(const HFCURL& pi_rURL) const override;
    virtual void        SetModificationTime(const HFCURL& pi_rURL, time_t pi_NewTime) const override;

    // Resource size
    virtual uint64_t   GetSize(const HFCURL& pi_rURL) const override;

    HFCStat::AccessStatus
    DetectAccess(const HFCURL& pi_rURL) const override;

    // Resource access mode
    virtual HFCAccessMode
    GetAccessMode(const HFCURL& pi_rURL) const override;
    };

//-----------------------------------------------------------------------------
// Instantiate the file implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Mem File Implementation Definition
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HFCStatEmbedFile::Register()
    {
    static HFCStatEmbedFileImpl s_impl;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCStatEmbedFileImpl::HFCStatEmbedFileImpl()
    {
    RegisterImpl(this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCStatEmbedFileImpl::~HFCStatEmbedFileImpl()
    {
    UnregisterImpl(this);
    }


//-----------------------------------------------------------------------------
// Public
// Indicates if an impl can handle an URL
//-----------------------------------------------------------------------------
bool HFCStatEmbedFileImpl::CanHandle(const HFCURL& pi_rURL) const
    {
    return (pi_rURL.GetSchemeType() == HFCURLEmbedFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatEmbedFileImpl::GetCreationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    const HFCURLEmbedFile& embedFileURLConst = reinterpret_cast<const HFCURLEmbedFile&>(pi_rURL);
    time_t Result = embedFileURLConst.GetCreationTime();

    return (Result);
    }

//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatEmbedFileImpl::GetLastAccessTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    //Don't know last access time, just return last modification time
    const HFCURLEmbedFile& embedFileURLConst = reinterpret_cast<const HFCURLEmbedFile&>(pi_rURL);
    time_t Result = embedFileURLConst.GetModificationTime();
    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatEmbedFileImpl::GetModificationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    const HFCURLEmbedFile& embedFileURLConst = reinterpret_cast<const HFCURLEmbedFile&>(pi_rURL);
    time_t Result = embedFileURLConst.GetModificationTime();
    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCStatEmbedFileImpl::SetModificationTime(const HFCURL& pi_rURL,
                                               time_t        pi_NewTime) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    const HFCURLEmbedFile& embedFileURLConst = reinterpret_cast<const HFCURLEmbedFile&>(pi_rURL);
    HFCURLEmbedFile& embedFileURL = const_cast<HFCURLEmbedFile&>(embedFileURLConst);
    embedFileURL.SetModificationTime(pi_NewTime);
    }


//-----------------------------------------------------------------------------
// Public
//
// Resource size
//-----------------------------------------------------------------------------
uint64_t HFCStatEmbedFileImpl::GetSize(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    const HFCURLEmbedFile& embedFileURLConst = reinterpret_cast<const HFCURLEmbedFile&>(pi_rURL);
    HFCURLEmbedFile& embedFileURL = const_cast<HFCURLEmbedFile&>(embedFileURLConst);

    uint64_t FileSize;
    embedFileURL.GetFileReaderHandler()->GetSize(FileSize);

    return FileSize;
    }

//-----------------------------------------------------------------------------
// Public
// Detect existence
//-----------------------------------------------------------------------------
HFCStat::AccessStatus HFCStatEmbedFileImpl::DetectAccess(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    const HFCURLEmbedFile& embedFileURLConst = reinterpret_cast<const HFCURLEmbedFile&>(pi_rURL);
    HFCURLEmbedFile& embedFileURL = const_cast<HFCURLEmbedFile&>(embedFileURLConst);

    if(embedFileURL.GetFileReaderHandler() != NULL)
        return HFCStat::AccessGranted;

    return HFCStat::TargetNotFound;
    }

//-----------------------------------------------------------------------------
// Public
// Resource access mode
//-----------------------------------------------------------------------------
HFCAccessMode HFCStatEmbedFileImpl::GetAccessMode(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    HFCAccessMode Result = HFC_READ_ONLY;

    return (Result);
    }


///-----------------------------------------------------------------------------
// Http file HFCStat Implementation Declaration
//-----------------------------------------------------------------------------

class HFCStatHttpFileImpl : public HFCStatImpl
    {
    private:
        bool IsURLInternetImaging(const HFCURL* pi_pURL) const;

    public:
        //--------------------------------------
        // Construction / Destruction
        //--------------------------------------

        HFCStatHttpFileImpl();
        virtual             ~HFCStatHttpFileImpl();


        //--------------------------------------
        // Methods
        //--------------------------------------

        // Indicates if an impl can handle an URL
        virtual bool       CanHandle(const HFCURL& pi_rURL) const override;

        // Resource time
        virtual time_t      GetCreationTime     (const HFCURL& pi_rURL) const override;
        virtual time_t      GetLastAccessTime   (const HFCURL& pi_rURL) const override;
        virtual time_t      GetModificationTime (const HFCURL& pi_rURL) const override;
        virtual void        SetModificationTime (const HFCURL& pi_rURL, time_t pi_NewTime) const override;

        // Resource size
        virtual uint64_t   GetSize(const HFCURL& pi_rURL) const override;

        virtual HFCStat::AccessStatus
            DetectAccess(const HFCURL& pi_rURL) const override;

        // Resource access mode
        virtual HFCAccessMode
            GetAccessMode(const HFCURL& pi_rURL) const override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HFCStatHttpFile::Register()
    {
    static HFCStatHttpFileImpl s_impl;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCStatHttpFileImpl::HFCStatHttpFileImpl()
    {
    RegisterImpl(this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCStatHttpFileImpl::~HFCStatHttpFileImpl()
    {
    UnregisterImpl(this);
    }

//-----------------------------------------------------------------------------
// Indicates if the given URL is a HTTP-IIP url
//-----------------------------------------------------------------------------
bool HFCStatHttpFileImpl::IsURLInternetImaging(const HFCURL* pi_pURL) const
    {
    HPRECONDITION(pi_pURL != 0);
    bool Result = false;

    if ((pi_pURL->GetSchemeType() == HFCURLHTTP::s_SchemeName()) ||
        (pi_pURL->GetSchemeType() == HFCURLHTTPS::s_SchemeName()))
        {
        // get the first 4 bytes of the search part and lower case them
        WString SearchPart(((const HFCURLHTTPBase*)pi_pURL)->GetSearchPart(), 0, 4);

        // verify that it starts with "fif="
        Result = CaseInsensitiveStringTools().AreEqual(SearchPart, L"fif=");
        }

    return (Result);
    }

//-----------------------------------------------------------------------------
// Public
// Indicates if an impl can handle an URL
//-----------------------------------------------------------------------------
bool HFCStatHttpFileImpl::CanHandle(const HFCURL& pi_rURL) const
    {
    return (((pi_rURL.GetSchemeType() == HFCURLHTTP::s_SchemeName())) &&
            (!IsURLInternetImaging(&pi_rURL))) ;
    }


//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatHttpFileImpl::GetCreationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t Result = 0;

    /* NEEDS_WORK
        It looks like there is no easy/safe way to retrieve file information without downloading the file.

    try
    {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an web file but use the cache copy only. i.e. do not download the file
        Result = HFCStat(HRFWebFile(pURL, HFC_READ_ONLY|HFC_SHARE_READ_WRITE).GetLocalURL()).GetCreationTime();
    }
    catch(...)
    {
        HASSERT(false);
    }
    */

    return Result;
    }

//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatHttpFileImpl::GetLastAccessTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t Result = 0;

    /* NEEDS_WORK
        It seems like there is no easy/safe way to retrieve file information without downloading the file.

    try
    {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an web file but use the cache copy only. i.e. do not download the file
        Result = HFCStat(HRFWebFile(pURL, HFC_READ_ONLY|HFC_SHARE_READ_WRITE).GetLocalURL()).GetLastAccessTime();
    }
    catch(...)
    {
        HASSERT(false);
    }
    */

    return Result;
    }

//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HFCStatHttpFileImpl::GetModificationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t Result = 0;

    /* NEEDS_WORK
        It looks like there is no easy/safe way to retrieve file information without downloading the file.

    try
    {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an web file but use the cache copy only. i.e. do not download the file
        Result = HFCStat(HRFWebFile(pURL, HFC_READ_ONLY|HFC_SHARE_READ_WRITE).GetLocalURL()).GetModificationTime();
    }
    catch(...)
    {
        HASSERT(false);
    }
    */

    return Result;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCStatHttpFileImpl::SetModificationTime(const HFCURL& pi_rURL, time_t pi_NewTime) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    // Cannot so do nothing
    }

//-----------------------------------------------------------------------------
// Public
// Resource size
//-----------------------------------------------------------------------------
uint64_t HFCStatHttpFileImpl::GetSize(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    uint64_t Result = 0;

    /* NEEDS_WORK
        It looks like there is no easy/safe way to retrieve file information without downloading the file.

    try
    {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an web file but use the cache copy only. i.e. do not download the file
        Result = HFCStat(HRFWebFile(pURL, HFC_READ_ONLY|HFC_SHARE_READ_WRITE).GetLocalURL()).GetSize();
    }
    catch(...)
    {
        HASSERT(false);
    }
    */

    return Result;
    }

//-----------------------------------------------------------------------------
// Public
// Detect existence
//-----------------------------------------------------------------------------
HFCStat::AccessStatus HFCStatHttpFileImpl::DetectAccess(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    return HFCStat::AccessGranted;
    }

//-----------------------------------------------------------------------------
// Public
// Resource access mode
//-----------------------------------------------------------------------------
HFCAccessMode HFCStatHttpFileImpl::GetAccessMode(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    return (HFC_READ_ONLY);
    }
