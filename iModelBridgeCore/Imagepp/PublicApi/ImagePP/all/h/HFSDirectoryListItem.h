//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSDirectoryListItem.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HFSDirectoryListItem
//-----------------------------------------------------------------------------

#pragma once

//############################
// INCLUDE FILES
//############################

BEGIN_IMAGEPP_NAMESPACE
class HFSDirectoryListItem
    {
public:

    // Construction - Destruction
    HFSDirectoryListItem();
    HFSDirectoryListItem(const WString& pi_rEntryName,
                         uint32_t pi_EntryAttrib,
                         uint32_t pi_EntrySize = 0,
                         const WString& pi_rTimeStamp = L"",
                         bool pi_IsSQLItem = false);
    HFSDirectoryListItem(const HFSDirectoryListItem& pi_rEntry);
    HFSDirectoryListItem& operator=(const HFSDirectoryListItem& pi_rEntry);

    virtual
    ~HFSDirectoryListItem();

    // Operations
    const WString&
    GetEntryName() const;

    uint32_t
    GetEntryAttrib() const;

    uint32_t
    GetEntrySize() const;

    const WString&
    GetTimeStamp() const;

    bool
    IsSQLItem() const;


protected:


private:

    void
    CommonCopy(const HFSDirectoryListItem& pi_rEntry);

    // Attributes
    WString m_EntryName;
    WString m_TimeStamp;
    uint32_t m_EntryAttrib;
    uint32_t m_EntrySize;
    bool   m_IsSQLItem;
    };
END_IMAGEPP_NAMESPACE
#include "HFSDirectoryListItem.hpp"

