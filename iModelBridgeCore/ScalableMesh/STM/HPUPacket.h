//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/HPUPacket.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : IDTMPacket
//-----------------------------------------------------------------------------

#pragma once
#ifndef VANCOUVER_API
#include <ImagePP/all/h/HFCPtr.h>
#include <ImagePP/all/h/HCDPacket.h>

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
     explicit        Packet                             (const char*             pi_pStr);

    // Raw data constructor
    explicit        Packet                             (const byte*             pi_pData,
                                                                    size_t                  pi_Size);

    explicit        Packet                             (size_t                  pi_Capacity = 0);

    Packet                             (const Packet&           pi_rRight);
    Packet&         operator=                          (const Packet&           pi_rRight);

    virtual         ~Packet                             ();




    // String data accessors
    const char*     GetAsStr                           () const;
    void            WrapStr                            (const char*             pi_pStr);

    // Raw data accessors
    const_iterator  Begin                              () const;
    const_iterator  End                                () const;

    const byte*     Get                                () const;
    size_t          GetSize                            () const;
    size_t          GetCapacity                        () const;

    bool            IsEmpty                            () const;

    iterator        BeginEdit                          ();
    iterator        EndEdit                            ();

    byte*           Edit                               ();

    void            Wrap                               (const Packet&           pio_rRight);

    void            Wrap                               (const byte*             pi_pData,
                                                                    size_t                  pi_Size);

    void            WrapEditable                       (byte*                   pi_pData,
                                                                    size_t                  pi_Size,
                                                                    size_t                  pi_Capacity = 0);

    bool            StealOwnership                     (Packet&                 pio_rRight);

    void            SetSize                            (size_t                  pi_Size);

    iterator        Resize                             (size_t                  pi_Size);

    void            Reserve                            (size_t                  pi_NewCapacity);

    void            Clear                              ();

    iterator        Erase                              (const_iterator          pi_Begin,
                                                                    const_iterator          pi_End);

    iterator        Erase                              (const_iterator          pi_Position,
                                                        size_t                  pi_Size = 1);

    iterator        Insert                             (const_iterator          pi_Position,
                                                                    size_t                  pi_Size = 1);

    iterator        Append                             (size_t                  pi_Size = 1);


    void            Swap                               (Packet&                 pio_rRight);

    bool            IsReadOnly                         () const;
    bool            IsBufferOwner                      () const;

    // Internal methods
    HCDPacket&                  EditPacket                         ();
    const HCDPacket&            GetPacket                          () const;

private:

    bool                 IsSharedPacket                     () const;

    iterator                    BufferBegin                        () const;
    iterator                    BufferEnd                          () const;

    void                        Copy                               (const Packet&           pi_rRight);
    bool                        CanBeReadAsStr                     () const;

    void                        AllocateNewBuffer                  (size_t                  pi_Capacity);

    HFCPtr<HCDPacket>           m_pDataPacket;
    bool                        m_ReadOnly;
    };


void                            swap                               (Packet&             pio_rLeft,
                                                                    Packet&             pio_rRight);


#include "HPUPacket.hpp"
#endif
} // End namespace HPU