//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : IDTMPacket
//-----------------------------------------------------------------------------

#pragma once
#ifndef VANCOUVER_API
#include <ImagePP/all/h/HFCPtr.h>
#include <ImagePP/all/h/HCDPacket.h>

USING_NAMESPACE_IMAGEPP

namespace HPU {

/*---------------------------------------------------------------------------------**//**
* @description  Helper class used to facilitate buffer management. This class can
*               wrap around an existing buffer without copying it. The referred buffer
*               is only copied when user tries to edit or resize the buffer (COW).
*
*               TDORAY: Consider making the underlying packet shareable. Need work
*               on every instance of HPRECONDITION(!IsSharedPacket()). This may
*               cause performances issues though...
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class Packet : public HFCShareableObject<Packet>
    {
public:
    typedef HFCPtr<Packet>      Ptr;
    typedef HFCPtr<Packet>      CPtr;

    typedef uint8_t                value_type;
    typedef const value_type*   const_iterator;
    typedef value_type*         iterator;

    // String data constructor
    BENTLEY_SM_EXPORT explicit        Packet                             (const char*             pi_pStr);

    // Raw data constructor
    BENTLEY_SM_EXPORT explicit        Packet                             (const byte*             pi_pData,
                                                                          size_t                  pi_Size);

    BENTLEY_SM_EXPORT explicit        Packet                             (size_t                  pi_Capacity = 0);

    Packet                             (const Packet&           pi_rRight);
    Packet&         operator=                          (const Packet&           pi_rRight);

    BENTLEY_SM_EXPORT virtual         ~Packet                             ();




    // String data accessors
    const char*     GetAsStr                           () const;
    void            WrapStr                            (const char*             pi_pStr);

    // Raw data accessors
    inline const_iterator  Begin                              () const
    {
        return BufferBegin();
    }
    inline const_iterator  End                                () const
    {
        return BufferEnd();
    }

    const byte*     Get                                () const;
    BENTLEY_SM_EXPORT size_t          GetSize                            () const;
    size_t          GetCapacity                        () const;

    BENTLEY_SM_EXPORT bool            IsEmpty                            () const;

    iterator        BeginEdit                          ();
    iterator        EndEdit                            ();

    byte*           Edit                               ();

    BENTLEY_SM_EXPORT void            Wrap                               (const Packet&           pio_rRight);

    BENTLEY_SM_EXPORT void            Wrap                               (const byte*             pi_pData,
                                                                    size_t                  pi_Size);

    BENTLEY_SM_EXPORT void            WrapEditable                       (byte*                   pi_pData,
                                                                        size_t                  pi_Size,
                                                                        size_t                  pi_Capacity = 0);

    bool            StealOwnership                     (Packet&                 pio_rRight);

    void            SetSize                            (size_t                  pi_Size);

    iterator        Resize                             (size_t                  pi_Size);

    void            Reserve                            (size_t                  pi_NewCapacity);

    BENTLEY_SM_EXPORT void            Clear                              ();

    iterator        Erase                              (const_iterator          pi_Begin,
                                                                    const_iterator          pi_End);

    iterator        Erase                              (const_iterator          pi_Position,
                                                        size_t                  pi_Size = 1);

    iterator        Insert                             (const_iterator          pi_Position,
                                                                    size_t                  pi_Size = 1);

    BENTLEY_SM_EXPORT iterator        Append                             (size_t                  pi_Size = 1);


    void            Swap                               (Packet&                 pio_rRight);

    bool            IsReadOnly                         () const;
    bool            IsBufferOwner                      () const;

    // Internal methods
    HCDPacket&                  EditPacket                         ();
    const HCDPacket&            GetPacket                          () const;

private:

    bool                 IsSharedPacket                     () const;

    inline iterator                    BufferBegin                        () const
    {
        return m_pDataPacket->GetBufferAddress();
    }
    iterator                    BufferEnd                          () const
    {
        return m_pDataPacket->GetBufferAddress() + m_pDataPacket->GetDataSize();
    }

    void                        Copy                               (const Packet&           pi_rRight);
    bool                        CanBeReadAsStr                     () const;

    void                        AllocateNewBuffer                  (size_t                  pi_Capacity);

    HFCPtr<HCDPacket>           m_pDataPacket;
    bool                        m_ReadOnly;
    };


void                            swap                               (Packet&             pio_rLeft,
                                                                    Packet&             pio_rRight);


} // End namespace HPU
#endif
