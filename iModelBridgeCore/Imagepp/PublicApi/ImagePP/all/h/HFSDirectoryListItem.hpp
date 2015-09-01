//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSDirectoryListItem.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HFSDirectoryListItem
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------
// HFSDirectoryListItem
//
// Default constructor.
//
//-----------------------------------------------------------------
inline HFSDirectoryListItem::HFSDirectoryListItem()
    {
    m_EntryName   = L"";
    m_EntryAttrib = 0;
    m_EntrySize   = 0;
    m_TimeStamp   = L"";
    m_IsSQLItem   = false;
    }

//-----------------------------------------------------------------
// HFSDirectoryListItem
//
// Main constructor.
//
// const string& pi_rEntryName:
// UInt32 pi_EntryAttrib:
// UInt32 pi_EntrySize:
// const string& pi_rTimeStamp:
//-----------------------------------------------------------------
inline HFSDirectoryListItem::HFSDirectoryListItem(const WString& pi_rEntryName,
                                                  uint32_t pi_EntryAttrib,
                                                  uint32_t pi_EntrySize,
                                                  const WString& pi_rTimeStamp,
                                                  bool pi_IsSQLItem)
    {
    m_EntryName   = pi_rEntryName;
    m_EntryAttrib = pi_EntryAttrib;
    m_EntrySize   = pi_EntrySize;
    m_TimeStamp   = pi_rTimeStamp;
    m_IsSQLItem   = pi_IsSQLItem;
    }

//-----------------------------------------------------------------
// HFSDirectoryListItem
//
// Copy constructor.
//
// const HFSDirectoryListItem& pi_rEntry:
//
//-----------------------------------------------------------------
inline HFSDirectoryListItem::HFSDirectoryListItem(const HFSDirectoryListItem& pi_rEntry)
    {
    CommonCopy(pi_rEntry);
    }

//-----------------------------------------------------------------
// HFSDirectoryListItem& operator=
//
// Assigment operator.
//
// const HFSDirectoryListItem& pi_rEntry:
//
//-----------------------------------------------------------------
inline HFSDirectoryListItem& HFSDirectoryListItem::operator=(const HFSDirectoryListItem& pi_rEntry)
    {
    if( &pi_rEntry != this )
        CommonCopy(pi_rEntry);

    return *this;
    }

//-----------------------------------------------------------------
// ~HFSDirectoryListItem
//
// Destructor
//
//-----------------------------------------------------------------
inline HFSDirectoryListItem::~HFSDirectoryListItem()
    {
    // Do nothing...
    }

//-----------------------------------------------------------------
// const string& GetEntryName
//
// Return the entry name.
//
//-----------------------------------------------------------------
inline const WString& HFSDirectoryListItem::GetEntryName() const
    {
    return m_EntryName;
    }

//-----------------------------------------------------------------
// UInt32 GetEntryAttrib
//
// Return the entry attributes.
//
//-----------------------------------------------------------------
inline uint32_t HFSDirectoryListItem::GetEntryAttrib() const
    {
    return m_EntryAttrib;
    }

//-----------------------------------------------------------------
// UInt32 GetEntrySize
//
// Return the entry size.
//
//-----------------------------------------------------------------
inline uint32_t HFSDirectoryListItem::GetEntrySize() const
    {
    return m_EntrySize;
    }

//-----------------------------------------------------------------
// const string& GetTimeStamp
//
// Return the time stamp of the entry.
//
//-----------------------------------------------------------------
inline const WString& HFSDirectoryListItem::GetTimeStamp() const
    {
    return m_TimeStamp;
    }

//-----------------------------------------------------------------
// void CommonCopy
//
// Copy a source object into this object.
//
// const HFSDirectoryListItem& pi_rSrc:
//
//-----------------------------------------------------------------
inline void HFSDirectoryListItem::CommonCopy(const HFSDirectoryListItem& pi_rSrc)
    {
    m_EntryName   = pi_rSrc.m_EntryName;
    m_EntryAttrib = pi_rSrc.m_EntryAttrib;
    m_EntrySize   = pi_rSrc.m_EntrySize;
    m_TimeStamp   = pi_rSrc.m_TimeStamp;
    m_IsSQLItem   = pi_rSrc.m_IsSQLItem;
    }

//-----------------------------------------------------------------
// bool IsSQLItem
//
// Check if the item is from an sql lister.
//
//
//-----------------------------------------------------------------
inline bool HFSDirectoryListItem::IsSQLItem() const
    {
    return m_IsSQLItem;
    }

END_IMAGEPP_NAMESPACE