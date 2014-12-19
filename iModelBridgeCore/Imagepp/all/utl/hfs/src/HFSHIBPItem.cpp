//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfs/src/HFSHIBPItem.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------
//:> Methods for class HFSHIBItem
//:>---------------------------------------------------------------------------


#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFSHIBPItem.h>
#include <Imagepp/all/h/HFCBuffer.h>
#include <Imagepp/all/h/HFSException.h>


//:>---------------------------------------------------------------------------
//:> public section
//:>---------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Constructor for this class.

 @param pi_rpHIBPHandler            The HIBP handler used by the item.
 @param pi_rItemPath                The starting path.
 @param pi_rpInterestingAttributes

 @exception HFSInvalidPathException
 @exception All exception returned by HFSHIBPHandler

 @see HFSItem
 @see HFSHIBPHandler

-----------------------------------------------------------------------------*/
HFSHIBPItem::HFSHIBPItem(HFCPtr<HFSHIBPHandler>&        pi_rpHIBPHandler,
                         const WString&                 pi_rItemPath,
                         const HFCPtr<HPMAttributeSet>& pi_rpInterestingAttributes)
    : HFSItem(),
      m_pHIBPHandler(pi_rpHIBPHandler),
      m_pInterestingAttributes(pi_rpInterestingAttributes),
      m_Expanded(false)
    {
    HPRECONDITION(pi_rpHIBPHandler != 0);
    HPRECONDITION(!pi_rItemPath.empty());

    HWARNING(pi_rpInterestingAttributes == 0, L"Interesting attribute not implemented yet with HIBP\n");

    // the path can be start with the host name, '\' or '/'
    if (pi_rItemPath[0] == L'\\' || pi_rItemPath[0] == L'/')
        {
        m_Host = pi_rpHIBPHandler->GetConnection()->GetServer();
        m_Host.erase(m_Host.length() - 1);  // remove the '\' or '\' at the end
        m_ParentPath = m_Host + pi_rItemPath;
        }
    else
        {
        // start with the host name, validate that the host is the same as the handler
        HAutoPtr<HFCURL> pUrl(HFCURL::Instanciate(pi_rItemPath));
        if (pUrl == 0 || !pUrl->IsCompatibleWith(HFCURLCommonInternet::CLASS_ID))
            throw HFSInvalidPathException(HFS_URL_SCHEME_NOT_SUPPORTED_EXCEPTION, pi_rItemPath);

        m_Host = pi_rpHIBPHandler->GetConnection()->GetServer();
        m_Host.erase(m_Host.length() - 1);  // remove the '/' at the end

        // at least, we must have the host
        WString Url = pUrl->GetURL();

        if (Url.length() < m_Host.length())
            throw HFSInvalidPathException(pi_rItemPath);

        if (BeStringUtilities::Wcsnicmp(m_Host.c_str(), Url.c_str(), m_Host.length()) != 0)
            throw HFSInvalidPathException(pi_rItemPath);

        m_ParentPath = pi_rItemPath;
        }

    WString::size_type Pos;

    // remove the item name
    if (m_ParentPath[m_ParentPath.length() - 1] == L'\\' ||
        m_ParentPath[m_ParentPath.length() - 1] == L'/')
        m_ParentPath.erase(m_ParentPath.length() - 1);

    if ((Pos = m_ParentPath.find_last_of(L"\\/")) == WString::npos || Pos < m_Host.length())
        {
        // we build the root
        SetName(m_Host);
        m_Folder = true;
        m_ParentPath = L"";
        }
    else
        {
        // extract the item name
        WString ItemName = m_ParentPath.substr(Pos + 1);

        // keep only the path
        m_ParentPath.resize(Pos);

        WString Path(m_ParentPath.empty()? L"\\" : m_ParentPath.substr(m_Host.length()));
        if (Path.empty())
            Path = L"\\";
        m_pHIBPHandler->GetEntries(Path);
        uint32_t EntryIndex;
        WString  EntryName;
        bool ItemFound = false;
        while (!ItemFound && m_pHIBPHandler->ReadEntry(&EntryIndex, &EntryName, &m_Folder))
            ItemFound = BeStringUtilities::Wcsicmp(ItemName.c_str(), EntryName.c_str()) == 0;

        if (!ItemFound)
            throw HFSInvalidPathException(pi_rItemPath);

        SetName(ItemName);
        }
    }


/**----------------------------------------------------------------------------
 Constructor for this class.

 @param pi_rpHIBPHandler            The HIBP handler use by the item.
 @param pi_rItemName                The item name
 @param pi_Folder                   Indicate if the item is a folder or not.
 @param pi_rParent                  The parent item
 @param pi_rpInterestingAttributes

 @see HFSItem

-----------------------------------------------------------------------------*/
HFSHIBPItem::HFSHIBPItem(HFCPtr<HFSHIBPHandler>&        pi_rpHIBPHandler,
                         const WString&                 pi_rItemName,
                         bool                          pi_Folder,
                         const HFCPtr<HFSHIBPItem>&     pi_rParent,
                         const HFCPtr<HPMAttributeSet>& pi_rpInterestingAttributes)
    : HFSItem(pi_rItemName,
              (const HFCPtr<HFSItem>&)pi_rParent,
              0),
    m_pHIBPHandler(pi_rpHIBPHandler),
    m_pInterestingAttributes(pi_rpInterestingAttributes),
    m_Folder(pi_Folder),
    m_Expanded(false)
    {
    HPRECONDITION(pi_rpHIBPHandler != 0);
    HPRECONDITION(!pi_rItemName.empty());

    HWARNING(pi_rpInterestingAttributes == 0, L"Interesting attribute not implemented yet with HIBP\n");

    m_Host = pi_rpHIBPHandler->GetConnection()->GetServer();
    m_Host.erase(m_Host.length() - 1);  // remove the '/' at the end
    }

/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/

HFSHIBPItem::~HFSHIBPItem()
    {
    }


/**----------------------------------------------------------------------------
 Check if the items was a folder.

 @return bool true if the item is a folder, false otherwise.
-----------------------------------------------------------------------------*/
bool HFSHIBPItem::IsFolder() const
    {
    return m_Folder;
    }


/**----------------------------------------------------------------------------
 Build the child list for this node.

 @exception All exception returned by HFSHIBPHandler
-----------------------------------------------------------------------------*/
void HFSHIBPItem::Expand()
    {
    HPRECONDITIONSHAREABLE();

    if (!m_Expanded && m_Folder)
        {
        HPRECONDITION(GetChildList().size() == 0);

        WString Path = GetPath();
        // remove the host
        Path.erase(0, m_Host.length());
        if (Path.empty())
            Path = L"\\";
        m_pHIBPHandler->GetEntries(Path);

        HFCPtr<HFSHIBPItem> pItem;
        uint32_t EntryIndex;
        WString EntryName;
        bool   Folder;

        while (m_pHIBPHandler->ReadEntry(&EntryIndex, &EntryName, &Folder))
            {
            pItem = new HFSHIBPItem(m_pHIBPHandler,
                                    EntryName,
                                    Folder,
                                    this,
                                    m_pInterestingAttributes);
            AddChild((const HFCPtr<HFCNode>&)pItem);
            }

        m_Expanded = true;
        }
    }


/**----------------------------------------------------------------------------
 Get the path of the entry.

 @return WString The path of the entry.
-----------------------------------------------------------------------------*/
WString HFSHIBPItem::GetPath() const
    {
    if (!m_ParentPath.empty())
        return m_ParentPath + L"\\" + GetName();
    else if (HasParent())
        {
        HPRECONDITION(GetParent()->IsCompatibleWith(HFSHIBPItem::CLASS_ID));
        return ((const HFCPtr<HFSHIBPItem>&)GetParent())->GetPath() + L"\\" + GetName();
        }
    else
        return GetName();
    }


/**----------------------------------------------------------------------------
 Get a specific item into this folder.

 @note if you iterate into the folder, this method can be change the current
       item.
 @return const HFCPtr<HFSItem>& See HFSItem::GetItem for returned value.

 @see HFSItem::GetItem
 @see GetFirstItem
 @see GetNextItem
-----------------------------------------------------------------------------*/
const HFCPtr<HFSItem>& HFSHIBPItem::GetItem(const WString& pi_rItemName) const
    {
    (const_cast<HFSHIBPItem*>(this))->Expand();

    return HFSItem::GetItem(pi_rItemName);
    }


/**----------------------------------------------------------------------------
 Create iterator.

 @return HFSNodeIterator*
-----------------------------------------------------------------------------*/
HFCNodeIterator* HFSHIBPItem::CreateIterator() const
    {
    const_cast<HFSHIBPItem*>(this)->Expand();

    return HFSItem::CreateIterator();
    }

/**----------------------------------------------------------------------------
 Count the number of item.

 @note This method was overloaded from HFCNode,

 @return UInt32 The number of item.
-----------------------------------------------------------------------------*/
size_t HFSHIBPItem::CountChild() const
    {

    (const_cast<HFSHIBPItem*>(this))->Expand();

    return HFSItem::CountChild();
    }

/**----------------------------------------------------------------------------
 This method check if the item has a parent.

 @note This method was overloaded from HFCNode. This method is recursive,
       cannot be inline

 @return bool true if the item has a parent, false if the item has a root.
-----------------------------------------------------------------------------*/
bool HFSHIBPItem::HasParent() const
    {
    bool Result;
    if (!m_ParentPath.empty())
        Result = true;
    else
        Result = HFCNode::HasParent();

    return Result;
    }


/**----------------------------------------------------------------------------
 Get the parent item.

 @note This method was overloaded from HFCNode,

 @return const HFCPtr<HFCNode>& The parent item. 0 if the item is a root.
-----------------------------------------------------------------------------*/
const HFCPtr<HFCNode>& HFSHIBPItem::GetParent() const
    {
    if (!m_ParentPath.empty())
        {
        HPRECONDITION(HFCNode::GetParent() == 0);

        HFCPtr<HFSHIBPItem> pParent = new HFSHIBPItem(m_pHIBPHandler,
                                                      m_ParentPath,
                                                      m_pInterestingAttributes);
        (const_cast<HFSHIBPItem*>(this))->SetParent((const HFCPtr<HFCNode>&)pParent);
        m_ParentPath = L"";
        }

    return HFCNode::GetParent();
    }


//:>---------------------------------------------------------------------------
//:> protected section
//:>---------------------------------------------------------------------------

//:>---------------------------------------------------------------------------
//:> private section
//:>---------------------------------------------------------------------------

