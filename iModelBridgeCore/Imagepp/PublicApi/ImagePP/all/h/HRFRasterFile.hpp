//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFile.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HRFRasterFile
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Public
// CancelCreate
// Call by the creator to notify the HRFFile that the create operation has been
// cancelled before its completion.
//-----------------------------------------------------------------------------
inline void HRFRasterFile::CancelCreate()
    {
    m_IsCreateCancel = true;
    }

//-----------------------------------------------------------------------------
// Public
// IsCreateCancel
// Returns true if the create operation has been cancelled before its
// completion, else false.
//-----------------------------------------------------------------------------
inline bool HRFRasterFile::IsCreateCancel()
    {
    return m_IsCreateCancel;
    }

//-----------------------------------------------------------------------------
// Public
// GetURL
// File information
//-----------------------------------------------------------------------------
inline const HFCPtr<HFCURL>& HRFRasterFile::GetURL() const
    {
    return m_pURL;
    }

//-----------------------------------------------------------------------------
// Public
// GetRelatedURLs
// Related files information
//-----------------------------------------------------------------------------
inline const ListOfRelatedURLs& HRFRasterFile::GetRelatedURLs() const
    {
    return m_ListOfRelatedURLs;
    }


//-----------------------------------------------------------------------------
// Public
// GetKey
// Returns the exclusive key assigned to this object
//-----------------------------------------------------------------------------
inline HFCExclusiveKey& HRFRasterFile::GetKey() const
    {
    return (m_Key);
    }


//-----------------------------------------------------------------------------
// Public
// GetPageWorldIdentificator
//
// The default implementation is for raster file that have the same world id
// for all pages.
//-----------------------------------------------------------------------------
inline const HGF2DWorldIdentificator HRFRasterFile::GetPageWorldIdentificator (uint32_t pi_Page) const
    {
    //Return the same world identificator for all pages by default.
    return GetWorldIdentificator();
    }


//-----------------------------------------------------------------------------
// Public
// CountPages
// File information
//-----------------------------------------------------------------------------
inline uint32_t HRFRasterFile::CountPages() const
    {
    return (uint32_t)m_ListOfPageDescriptor.size();
    }

//-----------------------------------------------------------------------------
// Public
// GetPageDescriptor
// File information
//-----------------------------------------------------------------------------
inline HFCPtr<HRFPageDescriptor> HRFRasterFile::GetPageDescriptor(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return m_ListOfPageDescriptor[pi_Page];
    }

//-----------------------------------------------------------------------------
// Public
// PagesAreRasterFile
// File information
//-----------------------------------------------------------------------------
inline bool HRFRasterFile::PagesAreRasterFile() const
    {
    return false;
    }

//-----------------------------------------------------------------------------
// Public
// GetPageFile
// File information
//-----------------------------------------------------------------------------
inline HFCPtr<HRFRasterFile> HRFRasterFile::GetPageFile(uint32_t pi_Page) const
    {
    HPRECONDITION(PagesAreRasterFile());

    // must be overwritten
    HASSERT(0);

    return 0;
    }

/** -----------------------------------------------------------------------------
 This method return the raster file physical access mode. This access mode is the
 effectif physical file access mode. This is the access mode used to open the
 file.

 As example, a raster file open in read-write can be treated as read-only for some
 reason. In this case, GetPhysicalAccessMode() should return read-write and
 GetLogicalAccessMode() should return read-only.

 @return The access mode returned indicates the physical file access.
 ---------------------------------------------------------------------------------
 */
inline HFCAccessMode HRFRasterFile::GetPhysicalAccessMode() const
    {
    return m_PhysicalAccessMode;
    }

/** -----------------------------------------------------------------------------
 This method return the raster file logical access mode. This access mode is the
 effectif raster file object access mode. It must be equal or smaler to the
 physical access mode.

 As example, a raster file open in read-write can be treated as read-only for some
 reason. In this case, GetPhysicalAccessMode() should return read-write and
 GetLogicalAccessMode() should return read-only.

 @return The access mode returned indicates if the raster file can be used in
         reading, writing, creating etc.
    -----------------------------------------------------------------------------
 */
inline HFCAccessMode HRFRasterFile::GetLogicalAccessMode() const
    {
    return m_LogicalAccessMode;
    }

/** -----------------------------------------------------------------------------
 This method set the raster file logical access mode. This access mode is the
 effectif raster file object access mode. It must be equal or smaler to the
 physical access mode.

 This methode should never be call after the creation of the page descriptor.

 As example, a raster file open in read-write can be treated as read-only for some
 reason. In this case, GetPhysicalAccessMode() should return read-write and
 GetLogicalAccessMode() should return read-only.

 @param pi_AccessMode The new access mode  for this raster file.
 -------------------------------------------------------------------------------
 */
inline void HRFRasterFile::ChangeLogicalAccessMode(HFCAccessMode pi_AccessMode)
    {
    HASSERT(m_PhysicalAccessMode.IsIncluded(pi_AccessMode));

    m_LogicalAccessMode = pi_AccessMode;
    }

/** -----------------------------------------------------------------------------
 This method return the capability logical access mode. This method should be
 replace be GetLogicalAccessMode().

 @see HRFRasterFile::GetLogicalAccessMode()

 @return The access mode returned indicates if the capability applies to
         reading, writing, creating raster file access etc.
 -----------------------------------------------------------------------------
 */
inline HFCAccessMode HRFRasterFile::GetAccessMode() const
    {
    return GetLogicalAccessMode();
    }


//-----------------------------------------------------------------------------
// Public
// GetAccessMode
// Access mode management
//-----------------------------------------------------------------------------
inline uint64_t HRFRasterFile::GetOffset() const
    {
    return m_Offset;
    }


//-----------------------------------------------------------------------------
// Public
// Indicates if the file supports LookAhead
//-----------------------------------------------------------------------------
inline bool HRFRasterFile::HasLookAhead(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return HasLookAheadByBlock(pi_Page) || HasLookAheadByExtent(pi_Page);
    }


//-----------------------------------------------------------------------------
// Public
// Indicates if the file supports LookAhead by block
//-----------------------------------------------------------------------------
inline bool HRFRasterFile::HasLookAheadByBlock(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return (false);
    }

//-----------------------------------------------------------------------------
// Public
// Indicates if the file supports LookAhead by extent
//-----------------------------------------------------------------------------
inline bool HRFRasterFile::HasLookAheadByExtent(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return (false);
    }

//-----------------------------------------------------------------------------
// Public
// Returns a pointer on the LockManager object
//-----------------------------------------------------------------------------
inline HFCBinStreamLockManager* HRFRasterFile::GetLockManager()
    {
    HPRECONDITION (m_pSharingControl != 0);
    return m_pSharingControl->GetLockManager();
    }


//-----------------------------------------------------------------------------
// Public
// Creates an instance of the HRFSharingControl class.
//-----------------------------------------------------------------------------
inline void HRFRasterFile::SharingControlCreate()
    {
    HPRECONDITION (GetURL() != 0);

    if (m_pSharingControl == 0)
        m_pSharingControl = new HRFSisterFileSharing(GetURL(), GetAccessMode());
    }

//-----------------------------------------------------------------------------
// Public
// Returns true if the logical counter is desynchronized with the physical one.
//-----------------------------------------------------------------------------
inline bool HRFRasterFile::SharingControlNeedSynchronization()
    {
    HPRECONDITION (m_pSharingControl != 0);

    return m_pSharingControl->NeedSynchronization();
    }

//-----------------------------------------------------------------------------
// Public
// Synchronizes the logical and physical counters.
//-----------------------------------------------------------------------------
inline void HRFRasterFile::SharingControlSynchronize()
    {
    HPRECONDITION (m_pSharingControl != 0);

    m_pSharingControl->Synchronize();
    }

//-----------------------------------------------------------------------------
// Public
// Increment the physical and logical counters.
//-----------------------------------------------------------------------------
inline void HRFRasterFile::SharingControlIncrementCount()
    {
    HPRECONDITION (m_pSharingControl != 0);

    m_pSharingControl->IncrementCurrentModifCount();
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HRFSharingControl instance.
//-----------------------------------------------------------------------------
inline HRFSharingControl* HRFRasterFile::GetSharingControl()
    {
    return (m_pSharingControl);
    }

//-----------------------------------------------------------------------------
// Public
// Return true if the sharing control file has been locked.
//-----------------------------------------------------------------------------
inline bool HRFRasterFile::SharingControlIsLocked()
    {
    HPRECONDITION (m_pSharingControl != 0);

    return m_pSharingControl->IsLocked();
    }


//-----------------------------------------------------------------------------
// Public
// IsOriginalRasterDataStorage
//-----------------------------------------------------------------------------
inline bool HRFRasterFile::IsOriginalRasterDataStorage() const
    {
    return true;
    }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// The rest of this file is the implentation of the sharing control for the
// Creator struct.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// Get related file URLs.
//
// This method must be overwritten for each "multi-file" raster file creators
//-----------------------------------------------------------------------------
inline bool HRFRasterFileCreator::GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                                  ListOfRelatedURLs&    pio_rRelatedURLs) const
    {
    return false;
    }

//-----------------------------------------------------------------------------
// Public
// Creates an instance of the HRFSharingControl class.
//-----------------------------------------------------------------------------
inline void HRFRasterFileCreator::SharingControlCreate(const HFCPtr<HFCURL>& pi_pURL)
    {
    HPRECONDITION (pi_pURL != 0);

    if (m_pSharingControl == 0)
        m_pSharingControl = new HRFSisterFileSharing(pi_pURL, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HRFSharingControl instance.
//-----------------------------------------------------------------------------
inline HRFSharingControl* HRFRasterFileCreator::GetSharingControl() const
    {
    return (m_pSharingControl);
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HFCBinStreamLockManager instance.
//-----------------------------------------------------------------------------
inline HFCBinStreamLockManager* HRFRasterFileCreator::GetLockManager() const
    {
    HPRECONDITION (m_pSharingControl != 0);

    return m_pSharingControl->GetLockManager();
    }

//-----------------------------------------------------------------------------
// Public
// Create a resolution editor at a specific resolution.
//
// This method must be overwritten if the format is an unlimited resolution
//-----------------------------------------------------------------------------
inline HRFResolutionEditor* HRFRasterFile::CreateUnlimitedResolutionEditor(uint32_t      pi_Page,
                                                                           double       pi_Resolution,
                                                                           HFCAccessMode pi_AccessMode)
    {
    HASSERT(0);

    return 0;
    }
END_IMAGEPP_NAMESPACE
