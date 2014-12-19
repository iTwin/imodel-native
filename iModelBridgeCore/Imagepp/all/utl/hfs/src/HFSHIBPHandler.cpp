//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfs/src/HFSHIBPHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Methods for class HFSHIBPHandler
//:>---------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFSHIBPHandler.h>
#include <Imagepp/all/h/HFSException.h>
#include <Imagepp/all/h/HFCThread.h>
#include <Imagepp/all/h/HFCEncodeDecodeASCII.h>


#include <Imagepp/all/h/HFCSocketConnection.h>
static Byte sEndMark[2] = {'\r', '\n'};
static Byte sEndMarkSize = 2;
static Byte sValueMark = ',';

#define HIBP_PROTOCOL       4
#define HIBP_UTF8_PROTOCOL  5

static const HFCVersion s_HIBP0110_UTF8(WString(L""), WString(L""), 3, 1, 1, HIBP_UTF8_PROTOCOL);
static const HFCVersion s_HIBP0110(WString(L""), WString(L""), 3, 1, 1, HIBP_PROTOCOL);
static const HFCVersion s_HIBP0100(WString(L""), WString(L""), 3, 1, 0, HIBP_PROTOCOL);

//:>---------------------------------------------------------------------------
//:> public section
//:>---------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Constructor for this class.

 @param pi_rpConnection     The connection use.

 @exception HFSHIBPInvalidResponseException
 @exception HFSHIBPInvalidProtocolVersionException
 @exception All exception returned by HFCInternetConnection

 @see HFCInternetConnection
-----------------------------------------------------------------------------*/
HFSHIBPHandler::HFSHIBPHandler(HFCPtr<HFCInternetConnection>&   pi_rpConnection,
                               uint32_t                         pi_ServerTimeOut)
    : m_pConnection(pi_rpConnection),
      m_ServerTimeOut(pi_ServerTimeOut),
      m_UseUTF8Protocol(false)

    {
    HPRECONDITION(pi_rpConnection != 0);
    HPRECONDITION(pi_ServerTimeOut);

    m_pBuffer = new HFCBuffer(1024);

    // get the version of the HIBP protocol

    // Send the request
    if (m_pConnection->ValidateConnect(m_ServerTimeOut))
        {
        if (!(ConnectToServer(s_HIBP0110_UTF8) ||
              ConnectToServer(s_HIBP0110)      ||
              ConnectToServer(s_HIBP0100)))
            {
            m_pConnection->Disconnect();
            throw HFSException(HFS_IBP_PROTOCOL_NOT_SUPPORTED_EXCEPTION);
            }
        m_pConnection->Disconnect();
        }
    else
        throw HFCInternetConnectionException(m_pConnection->GetServer(),
                                             HFCInternetConnectionException::CANNOT_CONNECT);
    }

/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/
HFSHIBPHandler::~HFSHIBPHandler()
    {
    }


/**----------------------------------------------------------------------------
 Get the number of item into the folder.

 @pi_rPath  The path of the folder.

 @return UInt32 The number of item.

 @exception
-----------------------------------------------------------------------------*/
uint32_t HFSHIBPHandler::CountEntries(const WString& pi_rPath)
    {
    uint32_t Result = 0;

    // Send the request
    if (m_pConnection->ValidateConnect(m_ServerTimeOut))
        {
        m_CurrentRequest.str(L"");
        m_CurrentRequest << L"Path=";
        m_CurrentRequest << pi_rPath;
        m_CurrentRequest << L"&Range=-1";
        m_CurrentRequest << L"&HIBP=" << m_Version;
        m_CurrentRequest << L"\r\n";

        Utf8String utf8Str;
        BeStringUtilities::WCharToUtf8(utf8Str,m_CurrentRequest.str().c_str());

        m_pConnection->Send((Byte*)utf8Str.c_str(),utf8Str.size());


        // Wait and read data
        size_t DataAvailableSize;
        bool Complete = false;
        while (!Complete &&
               m_pConnection->IsConnected() &&
               (DataAvailableSize = m_pConnection->WaitDataAvailable(m_ServerTimeOut)) > 0)
            {
            m_pConnection->Receive(m_pBuffer->PrepareForNewData(DataAvailableSize),
                                   DataAvailableSize);
            m_pBuffer->SetNewDataSize(DataAvailableSize);
            Complete = m_pBuffer->GetDataSize() > sEndMarkSize &&
                       *(m_pBuffer->GetData() + m_pBuffer->GetDataSize() - 2) == '\r' &&
                       *(m_pBuffer->GetData() + m_pBuffer->GetDataSize() - 1) == '\n';
            }

        m_pConnection->Disconnect();

        // the buffer must be contain hibp,version/length:<data>CRLF
        // the respond header are in UTF8, we can parse it like ascii character
        if (m_pBuffer->GetDataSize() < 5)
            throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

        if (BeStringUtilities::Strnicmp((const char*)m_pBuffer->GetData(), "hibp,", 5) == 0)
            {
            WString Version;
            uint32_t DataSize;
            ReadHIBPPathRespondHeader(m_pBuffer,
                                      &Version,
                                      &DataSize);

            uint32_t Compression;
            uint32_t CompressionInfo;
            uint32_t CurrentPass;
            ReadHIBPPathDataHeader(m_pBuffer,
                                   &Compression,
                                   &CompressionInfo,
                                   &Result,
                                   &CurrentPass);
            }
        else
            HandleHIBPError(m_pBuffer);
        }
    else
        throw HFCInternetConnectionException(m_pConnection->GetServer(),
                                             HFCInternetConnectionException::CANNOT_CONNECT);

    return Result;
    }

/**----------------------------------------------------------------------------
 Get items into the folder.

 @note This method execute the command "path" and retrieve the result. This
       method must be call before to call GetFirstItem.

 @param pi_rPath            The path of the folder.
 @param pi_FirstEntryIndex  The index of the first entry.
 @param pi_MaxCount         The maximun entries retrieved.

 @exception HFSHIBPInvalidResponseException
 @exception All exception returned by HandleHIBPError()
 @exception All exception returned by ReadHIBPPathRespondHeader()
 @exception All exception returned by ReadHIBPPathDataHeader()
 @exception All exception returned by HFCInternetConnection

 @see GetFirstItem
 @see HFCInternetConnection
-----------------------------------------------------------------------------*/
void HFSHIBPHandler::GetEntries(const WString&   pi_rPath,
                                uint32_t         pi_FirstEntryIndex,
                                uint32_t         pi_MaxCount)
    {
    HPRECONDITION(!pi_rPath.empty());


    m_CurrentRequest.str(L"");
    m_CurrentRequest << L"path=";
    m_CurrentRequest << pi_rPath;
    if (pi_FirstEntryIndex != 0 || pi_MaxCount != ULONG_MAX)
        {
        m_CurrentRequest << L"&range=" << pi_FirstEntryIndex << L"-" << pi_FirstEntryIndex + pi_MaxCount;
        }

    m_CurrentRequest << L"&hibp=" << m_Version;
    m_CurrentRequest << L"\r\n";

    if (m_pConnection->ValidateConnect(m_ServerTimeOut))
        {
        m_pBuffer->Clear();
        // send request in UTF8
        size_t  destinationBuffSize = WString(m_CurrentRequest.str().c_str()).GetMaxLocaleCharBytes();
        char*  pRequest= (char*)_alloca (destinationBuffSize);

        size_t RequestSize;
        if (m_UseUTF8Protocol)
            {
            Utf8String utf8Request;
            BeStringUtilities::WCharToUtf8(utf8Request, m_CurrentRequest.str().c_str(),BeStringUtilities::AsManyAsPossible);
            BeStringUtilities::Strncpy(pRequest,destinationBuffSize,utf8Request.c_str(),BeStringUtilities::AsManyAsPossible);
            RequestSize = strlen(pRequest) + 1;
            }
        else
            {
            BeStringUtilities::WCharToCurrentLocaleChar(pRequest, m_CurrentRequest.str().c_str(),destinationBuffSize);
            RequestSize = strlen(pRequest) + 1;
            }
        m_pConnection->Send((const Byte*)pRequest, RequestSize);

        // Wait and read data
        size_t DataAvailableSize;
        bool Complete = false;
        bool TimeOut = false;
        time_t StartTime = time(NULL);

        while (!Complete && !TimeOut && m_pConnection->IsConnected())
            {
            if ((DataAvailableSize = m_pConnection->WaitDataAvailable(m_ServerTimeOut)) > 0)
                {
                m_pConnection->Receive(m_pBuffer->PrepareForNewData(DataAvailableSize),
                                       DataAvailableSize);
                m_pBuffer->SetNewDataSize(DataAvailableSize);
                Complete = m_pBuffer->GetDataSize() > sEndMarkSize &&
                           *(m_pBuffer->GetData() + m_pBuffer->GetDataSize() - 2) == '\r' &&
                           *(m_pBuffer->GetData() + m_pBuffer->GetDataSize() - 1) == '\n';
                }
            else
                {
                TimeOut = ((time_t)(difftime(time(NULL), StartTime) * 1000) > (time_t)m_ServerTimeOut);
                HFCThread::Sleep(100);
                }
            }

        m_pConnection->Disconnect();

        if (m_pBuffer->GetDataSize() < 5)
            throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

        // the respond are in UTF8, we can parse the header like ascii
        if (BeStringUtilities::Strnicmp((const char*)m_pBuffer->GetData(), "hibp,", 5) == 0)
            {
            WString Version;
            uint32_t DataSize;
            ReadHIBPPathRespondHeader(m_pBuffer,
                                      &Version,
                                      &DataSize);

            HPRECONDITION(BeStringUtilities::Wcsicmp(Version.c_str(), m_Version.c_str()) == 0);

            uint32_t Compression;
            uint32_t CompressionInfo;
            uint32_t NbEntries;
            uint32_t CurrentPass;

            ReadHIBPPathDataHeader(m_pBuffer,
                                   &Compression,
                                   &CompressionInfo,
                                   &NbEntries,
                                   &CurrentPass);
            HPOSTCONDITION(Compression == 0);
            HPOSTCONDITION(CompressionInfo == 0);
            HPOSTCONDITION(CurrentPass == 0);
            }
        else
            HandleHIBPError(m_pBuffer);
        }
    else
        throw HFCInternetConnectionException(m_pConnection->GetServer(),
                                             HFCInternetConnectionException::CANNOT_CONNECT);
    }


/**----------------------------------------------------------------------------
 Get first entry.

 @note This method process the data received from the command "path" executed
       by the method GetEntries. The method GetEntries must be executed
       before the first call to this method.


 @param po_pEntryIndex  The index of the entry.
 @param po_pItemName    The item name
 @param po_pFolder      The item status, true if the item is a folder, false
                        otherwise

 @exception HFSHIBPInvalidResponseException

 @see GetEntries
 @see
-----------------------------------------------------------------------------*/
bool HFSHIBPHandler::ReadEntry(uint32_t*     po_pEntryIndex,
                                WString*    po_pItemName,
                                bool*      po_pFolder)
    {
    HPRECONDITION(po_pEntryIndex != 0);
    HPRECONDITION(po_pItemName != 0);
    HPRECONDITION(po_pFolder != 0);

    bool Result = false;

    // first, validate if we have reach the end of the buffer
    size_t EndEntryPos = m_pBuffer->SearchFor(sEndMark, sEndMarkSize);

    if (EndEntryPos == (size_t)-1 || EndEntryPos == 0)
        goto WARPUP;

    // read the entry index
    size_t SeparatorPos;
    if ((SeparatorPos = m_pBuffer->SearchFor(sValueMark)) == (size_t)-1 || SeparatorPos > EndEntryPos)
        throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

    *(const_cast<Byte*>(m_pBuffer->GetData()) + SeparatorPos) = 0;
    *po_pEntryIndex = atoi((const char*)m_pBuffer->GetData());
    *(const_cast<Byte*>(m_pBuffer->GetData()) + SeparatorPos) = sValueMark;

    SeparatorPos++;
    m_pBuffer->MarkReadData(SeparatorPos);
    EndEntryPos -= SeparatorPos;

    // read the item name
    if ((SeparatorPos = m_pBuffer->SearchFor(sValueMark)) == (size_t)-1,
        SeparatorPos > EndEntryPos)
        throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

    *(const_cast<Byte*>(m_pBuffer->GetData()) + SeparatorPos) = 0;

    BeStringUtilities::Utf8ToWChar(*po_pItemName,(const char*)m_pBuffer->GetData());

    HFCEncodeDecodeASCII::EscapeToASCII(*po_pItemName);
    *(const_cast<Byte*>(m_pBuffer->GetData()) + SeparatorPos) = sValueMark;

    SeparatorPos++;
    m_pBuffer->MarkReadData(SeparatorPos);
    EndEntryPos -= SeparatorPos;

    // we must have at least 3 character, one for the entry type and two for end entry mark
    if (m_pBuffer->GetDataSize() < 3)
        throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

    // read the item type
    *po_pFolder = *m_pBuffer->GetData() == '1';

    // mark entry has read
    m_pBuffer->MarkReadData(EndEntryPos + 2);

    Result = true;

WARPUP:
    return Result;
    }


/**----------------------------------------------------------------------------
 Get the connection.

 @return const HFCPtr<HFCInternetConnection>& The connection sue by the handler
-----------------------------------------------------------------------------*/
const HFCPtr<HFCInternetConnection>& HFSHIBPHandler::GetConnection() const
    {
    return m_pConnection;
    }

//:>---------------------------------------------------------------------------
//:> protected section
//:>---------------------------------------------------------------------------

//:>---------------------------------------------------------------------------
//:> private section
//:>---------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Read the header of the respond of the command path.

 @note The header must be contain "hibp,version/size:"

 @param pio_pBuffer
 @param po_pVersion
 @param po_pDataSize

 @exception HFSHIBException

 @see HFSHIBPException
-----------------------------------------------------------------------------*/
void HFSHIBPHandler::ReadHIBPPathRespondHeader(HFCBuffer* pio_pBuffer,
                                               WString*   po_pVersion,
                                               uint32_t*    po_pDataSize) const
    {
    HPRECONDITION(pio_pBuffer != 0);
    HPRECONDITION(pio_pBuffer->GetDataSize() >= 5);
    HPRECONDITION(po_pVersion != 0);
    HPRECONDITION(po_pDataSize != 0);

    // the buffer are in UTF8, we can parse the header like ascii
    if (BeStringUtilities::Strnicmp((const char*)pio_pBuffer->GetData(), "hibp,", 5) == 0)
        {
        // skip 'hibp,' into the buffer
        pio_pBuffer->MarkReadData(5);

        // read the version
        size_t TokenPos = pio_pBuffer->SearchFor('/');
        if (TokenPos == (size_t)-1)
            throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

        // replace / by \0
        *(const_cast<Byte*>(pio_pBuffer->GetData()) + TokenPos) = '\0';

        uint32_t codePage;
        BeStringUtilities::GetCurrentCodePage(codePage);
        BeStringUtilities::LocaleCharToWChar( *po_pVersion,(char*)pio_pBuffer->GetData(),codePage);

        // replace the original character
        *(const_cast<Byte*>(pio_pBuffer->GetData()) + TokenPos) = '/';
        pio_pBuffer->MarkReadData(TokenPos + 1);

        // read the data size
        TokenPos = pio_pBuffer->SearchFor(':');
        if (TokenPos == (size_t)-1)
            throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

        // replace : by \0
        *(const_cast<Byte*>(pio_pBuffer->GetData()) + TokenPos) = '\0';
        *po_pDataSize = atol((const char*)pio_pBuffer->GetData());

        // replace the original character
        *(const_cast<Byte*>(pio_pBuffer->GetData()) + TokenPos) = ':';
        pio_pBuffer->MarkReadData(TokenPos + 1);
        }
    else if (BeStringUtilities::Strnicmp((const char*)pio_pBuffer->GetData(), "error", 5) == 0)
        HandleHIBPError(pio_pBuffer);
    else
        throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());
    }


/**----------------------------------------------------------------------------
 Read the data header of the respond of the command path.

 @note The data header has this format

       data =  00 - 03     - header size (lets call it N)
               04 - 07     - Compression Type                  (unused for now)
               08 - 11     - Compression Info                  (unused for now)
               12 - 15     - Number of entries in current dir  (unused for now)
               16 - 19     - Pass Number                       (unused for now)
               ...
               N - size    - The entries

 @note For this release, we just skip the header data

 @param pio_pBuffer
 @param po_pCompression
 @param po_pCompressionInfo
 @param po_pNbEntries
 @param po_pCurrentPass

 @exception HFSHIBInvalidRespondException

 @see HFSHIBPException
-----------------------------------------------------------------------------*/
void HFSHIBPHandler::ReadHIBPPathDataHeader(HFCBuffer*  pio_pBuffer,
                                            uint32_t*     po_pCompression,
                                            uint32_t*     po_pCompressionInfo,
                                            uint32_t*     po_pNbEntries,
                                            uint32_t*     po_pCurrentPass) const
    {
    HPRECONDITION(pio_pBuffer != 0);
    HPRECONDITION(pio_pBuffer->GetDataSize() >= 4); // we must have at least the size of the header
    HPRECONDITION(po_pCompression != 0);
    HPRECONDITION(po_pCompressionInfo != 0);
    HPRECONDITION(po_pNbEntries != 0);
    HPRECONDITION(po_pCurrentPass != 0);

    uint32_t HeaderSize;
    // read header size
    HeaderSize = HFCSocketConnection::ntohl2(*((uint32_t*)pio_pBuffer->GetData()));

    if (HeaderSize != 5 * sizeof(uint32_t))       // the header size include the size of the header members
        throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

    pio_pBuffer->MarkReadData(sizeof(uint32_t));
    //UInt32 HeaderReadSize = sizeof(UInt32);

    // read compression
    *po_pCompression = HFCSocketConnection::ntohl2(*((uint32_t*)pio_pBuffer->GetData()));
    pio_pBuffer->MarkReadData(sizeof(uint32_t));

    // read compression info
    *po_pCompressionInfo = HFCSocketConnection::ntohl2(*((uint32_t*)pio_pBuffer->GetData()));
    pio_pBuffer->MarkReadData(sizeof(uint32_t));

    // read number of entries
    *po_pNbEntries = HFCSocketConnection::ntohl2(*((uint32_t*)pio_pBuffer->GetData()));
    pio_pBuffer->MarkReadData(sizeof(uint32_t));

    // read the current pass
    *po_pCurrentPass = HFCSocketConnection::ntohl2(*((uint32_t*)pio_pBuffer->GetData()));
    pio_pBuffer->MarkReadData(sizeof(uint32_t));
    }

/**----------------------------------------------------------------------------
 Handle error returned by the HIBP protocol

 @note This method throw many exception, this method never return.

 @param pi_pBuffer  The buffer contain an error.

 @exception HFSHIBPInvalidResponseException
 @exception HFSHIBPErrorException
-----------------------------------------------------------------------------*/
void HFSHIBPHandler::HandleHIBPError(HFCBuffer* pi_pBuffer) const
    {
    HPRECONDITION(pi_pBuffer != 0);

    if (pi_pBuffer->GetDataSize() < 6)
        throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

    if (BeStringUtilities::Strnicmp((const char*)pi_pBuffer->GetData(), "error/", 6) != 0)
        throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

    pi_pBuffer->MarkReadData(6);

    size_t TokenPos;

    if ((TokenPos = pi_pBuffer->SearchFor(':')) == (size_t)-1)
        throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

    pi_pBuffer->MarkReadData(TokenPos + 1);

    if ((TokenPos = pi_pBuffer->SearchFor(sEndMark, sEndMarkSize)) == (size_t)-1)
        throw HFSHIBPInvalidResponseException(m_CurrentRequest.str().c_str());

    *(const_cast<Byte*>(pi_pBuffer->GetData()) + TokenPos) = 0;

    WString tempoStr;
    BeStringUtilities::Utf8ToWChar(tempoStr,(const char*)pi_pBuffer->GetData());

    throw HFSHIBPErrorException(tempoStr.c_str());
    }



bool HFSHIBPHandler::ConnectToServer(const HFCVersion& pi_rVersion)
    {
    bool Result = false;
    ostringstream Request;
    bool UseUTF8Protocol = false;
    Request << ("HIBP=");
    Request << pi_rVersion.GetNumber(0);
    Request << ".";
    Request << pi_rVersion.GetNumber(1);
    if (pi_rVersion.GetNumber(2) == HIBP_UTF8_PROTOCOL)
        {
        Request << ",utf8";
        UseUTF8Protocol = true;
        }
    Request << "\r\n";

    m_pConnection->Send((Byte*)Request.str().c_str(), Request.str().size());

    // Wait and analyze data until either an error an ending marker
    size_t DataAvailableSize;
    bool Complete = false;
    HPRECONDITION(m_pBuffer->GetDataSize() == 0);
    bool TimeOut = false;
    time_t StartTime = time(NULL);
    while (!Complete && m_pConnection->IsConnected() && !TimeOut)
        {
        if ((DataAvailableSize = m_pConnection->WaitDataAvailable(m_ServerTimeOut)) > 0)
            {
            m_pConnection->Receive(m_pBuffer->PrepareForNewData(DataAvailableSize),
                                   DataAvailableSize);
            m_pBuffer->SetNewDataSize(DataAvailableSize);
            Complete = m_pBuffer->GetDataSize() >= sEndMarkSize &&
                       *(m_pBuffer->GetData() + m_pBuffer->GetDataSize() - 2) == '\r' &&
                       *(m_pBuffer->GetData() + m_pBuffer->GetDataSize() - 1) == '\n';
            }
        else
            {
            TimeOut = ((time_t)(difftime(time(NULL), StartTime) * 1000) > (time_t)m_ServerTimeOut);
            HFCThread::Sleep(100);
            }
        }

    // the buffer must be contain hibp:'version'[,utf8]
    string Response(Request.str());
    Response[4] = ':';  // replace '=' by ':'

    if (m_pBuffer->GetDataSize() >= Response.length() &&
        BeStringUtilities::Strnicmp((char*)m_pBuffer->GetData(), Response.c_str(), Response.length()) == 0)
        {
        HPRECONDITION(m_pBuffer->GetDataSize() == Response.length() + 2);

        // extract the protocol version
        wostringstream Version;
        Version << pi_rVersion.GetNumber(0) << L"." << pi_rVersion.GetNumber(1);

        if (UseUTF8Protocol)
            Version << L",utf8";

        m_Version = Version.str().c_str();
        m_UseUTF8Protocol = UseUTF8Protocol;

        Result = true;
        }

    m_pBuffer->Clear();

    return Result;
    }