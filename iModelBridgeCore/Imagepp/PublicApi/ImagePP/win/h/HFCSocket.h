//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCSocket.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCSocket
//-----------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFCSemaphore.h>
#include <ImagePP/all/h/HFCEvent.h>

class HFCSocket
    {
public:
    // Constructor - Destructor
    HFCSocket(int32_t pi_Af = PF_INET, int32_t pi_Type = SOCK_STREAM, int32_t pi_Protocol = 0);
    HFCSocket(SOCKET pi_Socket);
    virtual ~HFCSocket();

    // Socket Methods
    int32_t         Create(unsigned short pi_Port);
    HFCSocket*      Accept(uint32_t pi_TimeOut = 0);
    int32_t         Connect(const WString& pi_rIPAddress, unsigned short pi_Port);
    int32_t         Close();
    int32_t         Listen(int32_t pi_Backlog = SOMAXCONN);
    int32_t         Ioctl(int32_t pi_Cmd, uint32_t* pi_Argp);
    bool           IsConnected();

    // Data Transfert
    size_t          Send(const Byte* buffer, size_t BufferSize);
    bool           SendWithMarkers(const Byte* pi_pBuffer, size_t pi_Size);
    bool           Receive(Byte* pi_Buffer, size_t pi_ReadSize, int32_t pi_Flag = 0);

    // Socket Query Methods
    int32_t         GetLastError();
    int32_t         GetProtocol();
    int32_t         GetType();
    int32_t         GetAf();
    const WString&  GetPeerName();
    unsigned short GetPeerPort();
    const WString&  GetSockName();
    unsigned short GetSockPort();

    // Local Address Method
    static WString  GetHostAddress();

    // IN Buffer Methods
    uint32_t        GetInBytesAvailable();
    uint32_t        WaitForInBytesAvailable(uint32_t pi_TimeOut = 0);
    Byte*          GetInBytes(uint32_t& pi_Count);
    Byte*          PeekInBytes(uint32_t& pi_Count);
    void           SetInBufferSize(uint32_t pi_Size);
    uint32_t         GetInBufferSize();

    // OUT Buffer Methods
    void           SetOutBufferSize(uint32_t pi_Size);
    uint32_t         GetOutBufferSize();

    // Marker Methods
    static uint32_t FindMarker(const Byte*  pi_pBuffer,
                               uint32_t       pi_SearchLength,
                               const Byte** pi_ppMarker,
                               const uint32_t*  pi_pMarkerSize,
                               uint32_t       pi_MarkerCount);
    char*      CheckCommand();
    void       SetBeginMarker(char* pi_Marker);
    void       SetEndMarker(char* pi_Marker);
    char*      GetBeginMarker();
    char*      GetEndMarker();

    // operator
    operator SOCKET();
    operator SOCKET()const;


protected:

    // Constructor
    HFCSocket(HFCSocket& pi_Creator, SOCKET pi_Socket);


private:

    // Not implemented
    HFCSocket(const HFCSocket&);
    HFCSocket& operator=(const HFCSocket&);

    // Socket handle
    SOCKET   m_hSocket;

    // Last error value
    int32_t     m_LastError;

    // Socket attributes
    int32_t     m_Type;
    int32_t     m_Protocol;
    int32_t     m_Af;

    // Connection Flag
    bool       m_IsConnected;

    // Buffer Sizes
    uint32_t     m_InBufferSize;
    uint32_t     m_OutBufferSize;

    // Socket and Peer names
    WString     m_SockName;
    WString     m_PeerName;

    // Marker attributes
    char*      m_pBeginMarker;
    char*      m_pEndMarker;
    };

#include "HFCSocket.hpp"

