//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSisterFileSharing.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSisterFileSharing
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFSisterFileSharing.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLMemFile.h>
#include <Imagepp/all/h/HFCURLEmbedFile.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCMonitor.h>
#include <Imagepp/all/h/HRFUtility.h>

/**----------------------------------------------------------------------------
    Constructor
    This methode instanciate the pointer on the physical instance of the sister
    file if the raster file has sharing access.


    @param pi_rpURL The URL of the original file.
    @param pi_AccessMode The access mode of the original file.
    ------------------------------------------------------------------------ */
HRFSisterFileSharing::HRFSisterFileSharing(const HFCPtr<HFCURL>& pi_rpURL,
                                           HFCAccessMode         pi_AccessMode)
    :HRFSharingControl()
    {
    HPRECONDITION (pi_rpURL != 0);

    m_BypassSharing = false;

    if (pi_rpURL->IsCompatibleWith(HFCURLMemFile::CLASS_ID) || pi_rpURL->IsCompatibleWith(HFCURLEmbedFile::CLASS_ID))
        {
        SetSisterFileURL(pi_rpURL);
        m_BypassSharing = true;
        m_pLockManager = new HFCBinStreamLockManager(0, 0, 4, false);
        }
    else
        {
        HPRECONDITION (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID));

        SetSisterFileURL(pi_rpURL);

#if 0
        // Check if the file exist, don't use HFCStat, because if the file exist, I would like
        // to keep it alive, until open it.
        //
        HAutoPtr<HFCLocalBinStream> pIsExistentFile(
            new HFCLocalBinStream(Filename,
                                  HFC_SHARE_READ_WRITE | HFC_READ_ONLY,
                                  false, false, 0, -1/* didn't throw */) );
        bool IsExistent = true;
        if (dynamic_cast<HFCFileNotFoundException*>(pIsExistentFile->GetLastException()) != 0)
            {
            IsExistent = false;
            }
#endif
        // Compute the acess mode of the sister file from the access mode of it parent (the image)
        m_AccessMode = pi_AccessMode | HFC_READ_ONLY;
        m_AccessMode.m_HasCreateAccess = false;

        m_BypassSharing = !pi_AccessMode.m_HasWriteShare;

        if ((m_AccessMode.m_HasReadShare || m_AccessMode.m_HasWriteShare) && (pi_AccessMode.m_HasWriteAccess))
            {
            HFCURLFile* pURL = (HFCURLFile*)m_pURL.GetPtr();
            WString Filename(pURL->GetHost() + L"\\" + pURL->GetPath());
            bool IsExistent = BeFileName::DoesPathExist(Filename.c_str());

            m_AccessMode.m_HasCreateAccess = IsExistent;

            if (!IsExistent)
                {
                try
                    {
                    m_pSharingControlFile = new HFCLocalBinStream(Filename,                 // File name
                                                                  m_AccessMode,
                                                                  true,                     // Create File (Open always)
                                                                  true,                     // Auto Remove
                                                                  0,                        // File offset
                                                                  5);                       // Retry 5 times before throw
                    m_pSharingControlFile->ThrowOnError(); 
                    }
                catch(HFCFileException& )
                    {
                    HASSERT(m_pSharingControlFile == 0);
                    // Try again, but don't catch it...
                    m_pSharingControlFile = new HFCLocalBinStream(Filename,                 // File name
                                                                  m_AccessMode,
                                                                  true,                     // Create File (Open always)
                                                                  true,                     // Auto Remove
                                                                  0,                        // File offset
                                                                  5);                       // Retry 5 times before throw
                    m_pSharingControlFile->ThrowOnError(); 
                    }

                // Close file
                //        pIsExistentFile = 0;

                if (m_pSharingControlFile != 0)
                    {
                    m_pLockManager = new HFCBinStreamLockManager(m_pSharingControlFile.get(), 0, 4, false);

                    // Set the physical counter to 0
                    if (m_AccessMode.m_HasCreateAccess)
                        {
                        HFCLockMonitor SisterFileLock (m_pLockManager);
                        m_pSharingControlFile->SeekToBegin();
                        m_pSharingControlFile->Write(&m_ModifCount, sizeof(uint32_t));
                        SisterFileLock.ReleaseKey();
                        }
                    else
                        m_ModifCount = GetCurrentModifCount();
                    }
                else
                    m_pLockManager = new HFCBinStreamLockManager(0, 0, 4, false);
                }
            else
                m_pLockManager = new HFCBinStreamLockManager(0, 0, 4, false);
            }
        else
            m_pLockManager = new HFCBinStreamLockManager(0, 0, 4, false);
        }
    }

/**----------------------------------------------------------------------------
    Constructor
    Special constructor: Bypass sharing control
    ------------------------------------------------------------------------ */
HRFSisterFileSharing::HRFSisterFileSharing(const HFCPtr<HFCURL>& pi_rpURL,
                                           HFCAccessMode         pi_AccessMode,
                                           bool                 pi_BypassSharing)
    :HRFSharingControl()
    {
    m_BypassSharing = pi_BypassSharing;

    if (!pi_BypassSharing)
        {
        // Default constructor
        HRFSisterFileSharing(pi_rpURL, pi_AccessMode);
        }
    else
        {
        m_AccessMode = pi_AccessMode | HFC_READ_ONLY;

        HFCPtr<HFCURL> pTempURL = new HFCURLFile(L"file:\\nullsisterfile.null");
        SetSisterFileURL(pTempURL);

        m_pLockManager = new HFCBinStreamLockManager(0, 0, 4, false);
        }
    }


/** ---------------------------------------------------------------------------
    Destructor
    Public
    This function try to delete the sister file from it physical support. The
    sister file file only be deleted if it is the last instance of it in the
    file system of the computer.
    ------------------------------------------------------------------------ */
HRFSisterFileSharing::~HRFSisterFileSharing()
    {
    //:> This function shall try to delete the sister file. This attemp has to
    //:> fail if another instance of this class has the same sister file opened.
    //:> It is automatically done by the destructor of the HFCLocalBinStream.
    }

/** ---------------------------------------------------------------------------
    OpenFile
    Private
    This procedure open the sister file if it exist. Here, we presume that the
    Create method has already been called but in Read-Only mode and the file
    was not existent at this moment. We must ensure that the sister file exist
    before calling this method.
    ------------------------------------------------------------------------ */
void HRFSisterFileSharing::OpenFile()
    {
    HPRECONDITION (m_pSharingControlFile == 0);

    HFCURLFile* pURL = (HFCURLFile*)m_pURL.GetPtr();
    WString Filename(pURL->GetHost() + L"\\" + pURL->GetPath());


    bool IsExistent = BeFileName::DoesPathExist(Filename.c_str());

#if 0
    // Check if the file exist, don't use HFCStat, because if the file exist, I would like
    // to keep it alive, until open it.
    //
    HAutoPtr<HFCLocalBinStream> pIsExistentFile(
        new HFCLocalBinStream(Filename,
                              HFC_SHARE_READ_WRITE | HFC_READ_ONLY,
                              false, false, 0, -1/* didn't throw */) );
    bool IsExistent = true;
    if (dynamic_cast<HFCFileNotFoundException*>(pIsExistentFile->GetLastException()) != 0)
        {
        IsExistent = false;
        }
#endif
    if (IsExistent)
        {
        try
            {
            m_pSharingControlFile = new HFCLocalBinStream(Filename,             // File name
                                                          m_AccessMode,
                                                          true,                 // Create File (Open existent)
                                                          true,                 // Auto remove
                                                          0,                    // File offset
                                                          5);
            m_pSharingControlFile->ThrowOnError(); 

            // Set the pointer of the Sister file binStream in the LockManager object.
            m_pLockManager->SetSisterFileStream(m_pSharingControlFile);

            m_ModifCount = GetCurrentModifCount();
            }
        catch(HFCFileNotFoundException&)
            {
            m_pSharingControlFile = 0;
            }
        }
    }

/** ---------------------------------------------------------------------------
    SetSisterFileURL
    Private
    This method set the sister file name from the name of it raster file.

    @param pi_rpURL The URL of the original file.
    ------------------------------------------------------------------------ */
void HRFSisterFileSharing::SetSisterFileURL(const HFCPtr<HFCURL>& pi_rpURL)
    {
    HPRECONDITION (pi_rpURL != 0);

    WString ComposedName(pi_rpURL->GetURL());

    ComposedName += L".sharing.tmp";

    m_pURL = HFCURL::Instanciate(ComposedName);

    HPOSTCONDITION(m_pURL->IsCompatibleWith(HFCURLFile::CLASS_ID) || m_pURL->IsCompatibleWith(HFCURLMemFile::CLASS_ID) ||
                   m_pURL->IsCompatibleWith(HFCURLEmbedFile::CLASS_ID));
    }