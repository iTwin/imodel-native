//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSNamedPipeConnection.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCSNamedPipeConnection
//-----------------------------------------------------------------------------

#pragma once

#include "HFCInternetConnection.h"
#include "HFCEvent.h"
#include "HFCHandle.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Sebastien Tardif

    This class implements a named pipe internet connection. The class behaves exactly
    like any other internet connection except that construction is performed
    using a named pipe operating system communication channel.

    ???
    -----------------------------------------------------------------------------
*/
class HCSNamedPipeConnection : public HFCInternetConnection
    {
    friend class HCSNamedPipeServerConnection;
    friend class HCSNamedPipeConnectionGroup;

public:
    //-----------------------------------------------------------------------------
    // Types
    //-----------------------------------------------------------------------------

    // Override HFCEvent in order to make GetHandle public
    class NamedPipeEvent : public HFCEvent
        {
    public:
        NamedPipeEvent(bool         pi_ManualReset = true,
                       bool         pi_Signaled    = true)
            : HFCEvent(pi_ManualReset, pi_Signaled)
            {
            };

        virtual HFCHandle GetHandle() const
            {
            return HFCEvent::GetHandle();
            }
        };


    HDECLARE_CLASS_ID(1824, HFCInternetConnection)


    //--------------------------------------
    // Construction/Desctruction
    //--------------------------------------

    HCSNamedPipeConnection(const WString& pi_rServer);
    virtual         ~HCSNamedPipeConnection();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // I/O methods
    virtual void    Send   (const Byte* pi_pData, size_t pi_DataSize) override;
    virtual void    Receive(Byte* po_pData, size_t po_DataSize);
    virtual void    Receive(Byte* po_pData, size_t* pio_pDataSize);

    // Connection/disconnection
    virtual bool   Connect        (const WString& pi_rUserName,
                                    const WString& pi_rPassword,
                                    time_t         pi_TimeOut = 30000);
    virtual bool   Connect        (time_t pi_TimeOut = 30000);
    virtual bool   ValidateConnect(uint32_t pi_TimeOut = 30000);
    virtual void    Disconnect     ();

    // Data Query
    virtual size_t  WaitDataAvailable();
    virtual size_t  WaitDataAvailable(uint32_t pi_TimeOut);


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

#ifdef _WIN32
    // Events that are use to verify the completion of IO operation
    NamedPipeEvent  m_ReadEvent;
    OVERLAPPED      m_ReadOverlapped;
    NamedPipeEvent  m_WriteEvent;
    OVERLAPPED      m_WriteOverlapped;
    HFCHandle       m_hPipe;
#endif


    //--------------------------------------
    // Methods
    //--------------------------------------

    void            SetupReadOverlapped();

#ifdef _WIN32
    // Used only by the Server Named Pipe
    HCSNamedPipeConnection(HFCHandle pi_Pipe);
#endif
    };


