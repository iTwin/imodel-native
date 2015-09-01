//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSharingControl.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSharingControl
//   --------------------------------------------------------------------------

#pragma once

#include "HFCAccessMode.h"

BEGIN_IMAGEPP_NAMESPACE
//:Ignore
class HFCBinStream;
class HFCBinStreamLockManager;
//:End Ignore

/** ---------------------------------------------------------------------------
    This class implements a system to HASSERT that the file sharing of the raster
    files will be done without any risk of corrupting the files.
    It is the base class for to descendants. HRFSisterFileSharing implents the
    sharing control within a sister file and HFRCacheFileSharing implents it
    directly into the TIFF file. It will only be use for the file format of HMR,
    HRM, iTiff and cTiff.

    @see HRFSisterFileSharing
    @see HRFCacheFileSharing
// ------------------------------------------------------------------------- */
class HRFSharingControl
    {
    HDECLARE_BASECLASS_ID(HRFSharingControlId_Base)

public:
    //:> Default constructor. We must use the create method with this constructor
    HRFSharingControl();

    //:> The destructor attempt to delete the physical file, but should not be able
    //:> to do it if an other instance of this class has the file opened.
    virtual ~HRFSharingControl();

    //:> This function return true if the logical and physical counter are different.
    virtual bool   NeedSynchronization ();

    //:> This method synchronize the physical and logical counter
    virtual void    Synchronize();

    //:> Add 1 to the current physical and logical count. They must be synchronize.
    virtual void    IncrementCurrentModifCount();

    //:> Return true if a lock has already been perform without it corresponding
    //:> unlock
    virtual bool   IsLocked() const = 0;

    //:> Return a pointer on the lock manager object
    virtual HFCBinStreamLockManager* GetLockManager() = 0;

protected:
    //:> Returns the internal modification count
    virtual uint32_t GetCurrentModifCount();

    //:> Returns a pointer on the sister file stream.
    virtual HFCBinStream* GetSisterFilePtr() const = 0;

    //:> Attributes
    uint32_t                     m_ModifCount;
    uint64_t                    m_Offset;
    HFCAccessMode                m_AccessMode;
    //bool                        m_IsOpen;

private:

    //:> Disabled methods
    HRFSharingControl(const HRFSharingControl&);
    HRFSharingControl& operator=(const HRFSharingControl&);
    };
END_IMAGEPP_NAMESPACE

#include "HRFSharingControl.hpp"
