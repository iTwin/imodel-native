//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfs/src/HFSHIB.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//:>---------------------------------------------------------------------------
//:> Methods for class HFSHIB
//:>---------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCHTTPConnection.h>
#include <Imagepp/all/h/HFCSocketConnection.h>
#include <Imagepp/all/h/HFCURLHTTPBase.h>
#include <Imagepp/all/h/HFCURLHTTP.h>
#include <Imagepp/all/h/HFCURLHTTPS.h>
#include <Imagepp/all/h/HFCURLInternetImagingSocket.h>

#include <Imagepp/all/h/HFSException.h>
#include <Imagepp/all/h/HFSHIB.h>
#include <Imagepp/all/h/HFSHIBPHandler.h>
#include <Imagepp/all/h/HFSItemIterator.h>

//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Constructor for this class.

 @param pi_rpConnection
 @param pi_rFolderPath
 @param pi_rpInterestingAttributes

-----------------------------------------------------------------------------*/
HFSHIB::HFSHIB(HFCPtr<HFCConnection>&           pi_rpConnection,
               uint32_t                         pi_ServerTimeOut,
               const WString&                   pi_rFolderPath,
               const HFCPtr<HPMAttributeSet>&   pi_rpInterestingAttributes)
    : HFSFileSystem(),
      m_pConnection(pi_rpConnection),
      m_pInterestingAttributes(pi_rpInterestingAttributes)
    {
    HPRECONDITION(pi_rpConnection->IsCompatibleWith(HFCInternetConnection::CLASS_ID));
    HPRECONDITION(!pi_rFolderPath.empty());
    HPRECONDITION(pi_ServerTimeOut > 0);

    HWARNING(pi_rpInterestingAttributes == 0, L"Interesting attribute not implemented yet with HIBP\n");

    HFCPtr<HFSHIBPHandler> pHIBPHandler = new HFSHIBPHandler((HFCPtr<HFCInternetConnection>&)pi_rpConnection,
                                                             pi_ServerTimeOut);
    m_pCurrentFolder = new HFSHIBPItem(pHIBPHandler,
                                       pi_rFolderPath,
                                       m_pInterestingAttributes);

    if (!m_pCurrentFolder->IsFolder())
        throw HFSInvalidPathException(HFS_INVALID_DIRECTORY_PATH_EXCEPTION, pi_rFolderPath);

    // optimization
    m_Host = ((HFCPtr<HFCInternetConnection>&)m_pConnection)->GetServer();
    m_Host.erase(m_Host.length() - 1);  // remove '/' at the end
    }

/**----------------------------------------------------------------------------
 Constructor for this class.

 @param pi_rpPath                   The URL of the ressource. The URL must be
                                    compatible with HFCURLCommonInternet. A
                                    connection will be create with with this
                                    URL, the caller must be set the timeout to
                                    this connection.
 @param pi_ServerTimeOut
 @param pi_rpInterestingAttributes

 @exception HFSInvalidPathException

-----------------------------------------------------------------------------*/
HFSHIB::HFSHIB(const HFCPtr<HFCURL>&            pi_rpPath,
               uint32_t                         pi_ServerTimeOut,
               const HFCPtr<HPMAttributeSet>&   pi_rpInterestingAttributes)
    : HFSFileSystem(),
      m_pInterestingAttributes(pi_rpInterestingAttributes)
    {
    HPRECONDITION(pi_rpPath != 0);
    HPRECONDITION(pi_rpPath->IsCompatibleWith(HFCURLCommonInternet::CLASS_ID));

    HWARNING(pi_rpInterestingAttributes == 0, L"Interesting attribute not implemented yet with HIBP\n");
    
    if (pi_rpPath->IsCompatibleWith(HFCURLHTTP::CLASS_ID))
        {
        const HFCPtr<HFCURLHTTP> pURL = (const HFCPtr<HFCURLHTTP>&)pi_rpPath;
        HAutoPtr<HFCURLHTTP> pHostURL(new HFCURLHTTP(pURL->GetUser(),
                                                     pURL->GetPassword(),
                                                     pURL->GetHost(),
                                                     pURL->GetPort(),
                                                     L"",
                                                     L""));
        HASSERT(pHostURL != 0);
        m_pConnection = new HFCHTTPConnection(pHostURL->GetURL(),
                                              pHostURL->GetUser(),
                                              pHostURL->GetPassword());
        }
    else if (pi_rpPath->IsCompatibleWith(HFCURLHTTPS::CLASS_ID))
        {
        const HFCPtr<HFCURLHTTPS> pURL = (const HFCPtr<HFCURLHTTPS>&)pi_rpPath;
        HAutoPtr<HFCURLHTTPS> pHostURL(new HFCURLHTTPS(pURL->GetUser(),
                                                       pURL->GetPassword(),
                                                       pURL->GetHost(),
                                                       pURL->GetPort(),
                                                       L"",
                                                       L""));
        HASSERT(pHostURL != 0);
        m_pConnection = new HFCHTTPConnection(pHostURL->GetURL(),
                                              pHostURL->GetUser(),
                                              pHostURL->GetPassword());
        }
    else if (pi_rpPath->IsCompatibleWith(HFCURLInternetImagingSocket::CLASS_ID))
        {
        const HFCPtr<HFCURLInternetImagingSocket> pURL =
            (const HFCPtr<HFCURLInternetImagingSocket>&)pi_rpPath;
        HAutoPtr<HFCURLInternetImagingSocket> pHostURL(
            new HFCURLInternetImagingSocket(pURL->GetUser(),
                                            pURL->GetPassword(),
                                            pURL->GetHost(),
                                            pURL->GetPort(),
                                            L""));
        HASSERT(pHostURL != 0);
        m_pConnection = new HFCSocketConnection(pHostURL->GetURL(),
                                                pHostURL->GetUser(),
                                                pHostURL->GetPassword());
        }
    else
        throw HFSInvalidPathException(HFS_URL_SCHEME_NOT_SUPPORTED_EXCEPTION, pi_rpPath->GetURL());


    HFCPtr<HFSHIBPHandler> pHIBPHandler = new HFSHIBPHandler((HFCPtr<HFCInternetConnection>&)m_pConnection,
                                                             pi_ServerTimeOut);
    m_pCurrentFolder = new HFSHIBPItem(pHIBPHandler,
                                       L"\\" + ((const HFCPtr<HFCURLCommonInternet>&)pi_rpPath)->GetURLPath(),
                                       m_pInterestingAttributes);

    if (!m_pCurrentFolder->IsFolder())
        throw HFSInvalidPathException(m_pCurrentFolder->GetPath());

    // optimization
    m_Host = ((HFCPtr<HFCInternetConnection>&)m_pConnection)->GetServer();
    m_Host.erase(m_Host.length() - 1);  // remove '/' at the end
    }

/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/
HFSHIB::~HFSHIB()
    {
    HFCPtr<HFCNode> pRootNode((HFCPtr<HFCNode>&)m_pCurrentFolder);

    //Gets the root node.
    while (pRootNode->GetParent() != 0)
        {
        pRootNode = pRootNode->GetParent();
        }

    RemoveAllDescendantNodes(pRootNode);
    }

/**----------------------------------------------------------------------------
 Get item.

 @param pi_rPath    The name of the item if this item was in the current folder,
                    the path of the item otherwise

 @return const HFCPtr<HFSItem>& The item. 0 if the item was not found.
-----------------------------------------------------------------------------*/
const HFCPtr<HFSItem>& HFSHIB::GetItem(const WString& pi_rPath) const
    {
    HPRECONDITION(!pi_rPath.empty());


    bool AbsolutePath = false;
    WString Path;

    // check if the host are in the path
    if (pi_rPath.length() >= m_Host.length() &&
        BeStringUtilities::Wcsnicmp(pi_rPath.c_str(), m_Host.c_str(), m_Host.length()) == 0)
        {
        Path = pi_rPath.substr(m_Host.length());
        if (Path.empty())   // if we have only the host, add '\'
            Path = L"\\";
        AbsolutePath = true;
        }
    else if (pi_rPath[0] == L'\\' || pi_rPath[0] == L'/')
        {
        Path = pi_rPath;
        AbsolutePath = true;
        }

    WString CurrentPath(m_pCurrentFolder->GetPath());

    WString::size_type BeginPos;
    WString::size_type EndPos;
    WString FolderName;
    if (AbsolutePath)
        {
        HPRECONDITION(Path[0] == L'\\' || Path[0] == L'/');
        Path.erase(0, 1);   // skip the first '\' or '/'

        CurrentPath.erase(0, m_Host.length() + 1);  // remove the host and the first '\' or '/'

        BeginPos = 0;
        while (BeginPos != WString::npos && !CurrentPath.empty())
            {
            EndPos = Path.find_first_of(L"\\/", BeginPos);
            FolderName = Path.substr(BeginPos, (EndPos == WString::npos? EndPos : EndPos - BeginPos));
            // check if is the same folder
            if ((BeStringUtilities::Wcsnicmp(FolderName.c_str(), CurrentPath.c_str(), FolderName.length()) == 0) &&
                (CurrentPath.length() == FolderName.length() ||
                 CurrentPath[FolderName.length()] == L'\\' ||
                 CurrentPath[FolderName.length()] == L'/'))
                {
                // remove the folder from paths
                CurrentPath.erase(0, FolderName.length() + 1);  // remove the '\' or '/'
                Path.erase(0, FolderName.length() + 1);         // remove the '\' or '/'
                BeginPos = (EndPos == WString::npos ? EndPos : 0);
                }
            else
                BeginPos = WString::npos;
            }

        // build a relative path, replace all folder name by '..'
        if (!CurrentPath.empty())
            {
            BeginPos = 0;
            while (BeginPos != WString::npos)
                {
                EndPos = CurrentPath.find_first_of(L"\\/", BeginPos);
                CurrentPath.replace(BeginPos, EndPos - BeginPos, L"..");
                BeginPos = (EndPos == WString::npos? EndPos : BeginPos + 3);
                }
            Path = CurrentPath + L"\\" + Path;
            }
        else
            Path = L".\\" + Path;
        }
    else
        Path = L".\\" + pi_rPath;

    // if the path end with '\' or '/', remove it
    if (Path[Path.length() - 1] == L'\\' || Path[Path.length() - 1] == L'/')
        Path.erase(Path.length() - 1, 1);

    // now, we have a relative path
    m_pResult = m_pCurrentFolder;
    BeginPos = 0;
    while (BeginPos != WString::npos)
        {
        EndPos = Path.find_first_of(L"\\/", BeginPos);
        FolderName = Path.substr(BeginPos, EndPos == WString::npos? EndPos : EndPos - BeginPos);

        if (BeStringUtilities::Wcsicmp(FolderName.c_str(), L"..") == 0)
            {
            if (m_pResult->HasParent())
                {
                HPRECONDITION(m_pResult->GetParent()->IsCompatibleWith(HFSHIBPItem::CLASS_ID));
                m_pResult = (const HFCPtr<HFSHIBPItem>&)m_pResult->GetParent();
                }
            else
                m_pResult = 0;
            }
        else if (BeStringUtilities::Wcsicmp(FolderName.c_str(), L".") == 0)
            {
            // do nothing
            }
        else
            {
            // found the folder entry into the child
            bool ItemFound = false;
            HFCPtr<HFSItem> pItem;
            HAutoPtr<HFCNodeIterator> pItr(m_pResult->CreateIterator());
            HPOSTCONDITION(pItr->IsCompatibleWith(HFSItemIterator::CLASS_ID));
            HFSItemIterator* pItemItr = (HFSItemIterator*)pItr.get();

            if ((pItem = pItemItr->GetFirstItem()) != 0)
                {
                do
                    {
                    HPRECONDITION(pItem->IsCompatibleWith(HFSHIBPItem::CLASS_ID));
                    ItemFound = wcscmp(pItem->GetName().c_str(), FolderName.c_str()) == 0;
                    }
                while (!ItemFound && (pItem = pItemItr->GetNextItem()) != 0);
                }

            if (ItemFound)
                m_pResult = (const HFCPtr<HFSHIBPItem>&)pItem;
            else
                m_pResult = 0;
            }

        BeginPos = EndPos == WString::npos ? EndPos : EndPos + 1;
        }

    return (const HFCPtr<HFSItem>&)m_pResult;
    }

/**----------------------------------------------------------------------------
 Return the URL of the current folder.

 @return HFCPtr<HFCURL> The URL of the current folder
-----------------------------------------------------------------------------*/
HFCPtr<HFCURL> HFSHIB::GetCurrentURLPath() const
    {
    HFCPtr<HFCURL> pURL;
    if (m_pConnection->IsCompatibleWith(HFCHTTPConnection::CLASS_ID))
        {
        WString Host = m_pConnection->GetServer();
        WString Port;
        WString::size_type Pos = Host.find(L":");
        if (Pos != string::npos)
            {
            Port = Host.substr(Pos + 1);
            Host.resize(Pos);
            }

        const HFCPtr<HFCURL>& rpConnectionURL(((HFCPtr<HFCHTTPConnection>&)m_pConnection)->GetURL());

        HASSERT(rpConnectionURL->IsCompatibleWith(HFCURLHTTPBase::CLASS_ID) == true);

        pURL = new HFCURLHTTP(L"",
                              L"",
                              Host,
                              Port,
                              m_pCurrentFolder->GetPath().erase(0, 1), // remove the '\'
                              ((HFCPtr<HFCURLHTTPBase>&)rpConnectionURL)->GetSearchPart());
        }
    else if (m_pConnection->IsCompatibleWith(HFCSocketConnection::CLASS_ID))
        {
        WString Host = m_pConnection->GetServer();
        WString Port;
        WString::size_type Pos = Host.find(L":");
        if (Pos != WString::npos)
            {
            Port = Host.substr(Pos + 1);
            Host.resize(Pos);
            }

        WString Path = m_pCurrentFolder->GetPath();
        pURL = new HFCURLInternetImagingSocket(L"",
                                               L"",
                                               Host,
                                               Port,
                                               m_pCurrentFolder->GetPath().erase(0, 1)); // remove the '\'
        }
    else
        {
        HASSERT(0);
        }
    return pURL;
    }

//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Remove all the node descending (i.e. : child nodes, grandchild nodes,
 grand grandchild nodes, etc...) from the node passed in parameter.

 @param pi_prNode Node from which the descending nodes should be removed.

 @return
-----------------------------------------------------------------------------*/
void HFSHIB::RemoveAllDescendantNodes(const HFCPtr<HFCNode>& pi_prNode) const
    {
    HPRECONDITION(pi_prNode != 0);

    bool                              IsChildRemoved;
    HFCNode::ChildList::const_iterator NodeIter = pi_prNode->GetChildList().begin();

    while (NodeIter != pi_prNode->GetChildList().end())
        {
        RemoveAllDescendantNodes(*NodeIter);

        IsChildRemoved = pi_prNode->RemoveChild(*NodeIter);
        HASSERT(IsChildRemoved == true);
        NodeIter = pi_prNode->GetChildList().begin();
        }
    }

//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------