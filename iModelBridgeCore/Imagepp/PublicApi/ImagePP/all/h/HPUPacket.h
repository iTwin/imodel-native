//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPUPacket.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : IDTMPacket
//-----------------------------------------------------------------------------

#pragma once

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

    typedef Byte value_type;
    typedef const value_type*   const_iterator;
    typedef value_type*         iterator;

    // String data constructor
    _HDLLu      explicit        Packet                             (const char*             pi_pStr);

    // Raw data constructor
    _HDLLu      explicit        Packet                             (const Byte*             pi_pData,
                                                                    size_t                  pi_Size);

    _HDLLu      explicit        Packet                             (size_t                  pi_Capacity = 0);

    _HDLLu                      Packet                             (const Packet&           pi_rRight);
    _HDLLu      Packet&         operator=                          (const Packet&           pi_rRight);

    _HDLLu      virtual         ~Packet                             ();




    // String data accessors
    _HDLLu      const char*     GetAsStr                           () const;
    _HDLLu      void            WrapStr                            (const char*             pi_pStr);

    // Raw data accessors
    _HDLLu      const_iterator  Begin                              () const;
    _HDLLu      const_iterator  End                                () const;

    _HDLLu      const Byte*     Get                                () const;
    _HDLLu      size_t          GetSize                            () const;
    _HDLLu      size_t          GetCapacity                        () const;

    _HDLLu      bool            IsEmpty                            () const;

    iterator        BeginEdit                          ();
    iterator        EndEdit                            ();

    _HDLLu      Byte*           Edit                               ();

    _HDLLu      void            Wrap                               (const Packet&           pio_rRight);

    _HDLLu      void            Wrap                               (const Byte*             pi_pData,
                                                                    size_t                  pi_Size);

    _HDLLu      void            WrapEditable                       (Byte*                   pi_pData,
                                                                    size_t                  pi_Size,
                                                                    size_t                  pi_Capacity = 0);

    _HDLLu      bool            StealOwnership                     (Packet&                 pio_rRight);

    _HDLLu      void            SetSize                            (size_t                  pi_Size);

    _HDLLu      iterator        Resize                             (size_t                  pi_Size);

    _HDLLu      void            Reserve                            (size_t                  pi_NewCapacity);

    _HDLLu      void            Clear                              ();

    _HDLLu      iterator        Erase                              (const_iterator          pi_Begin,
                                                                    const_iterator          pi_End);

    iterator        Erase                              (const_iterator          pi_Position,
                                                        size_t                  pi_Size = 1);

    _HDLLu      iterator        Insert                             (const_iterator          pi_Position,
                                                                    size_t                  pi_Size = 1);

    _HDLLu      iterator        Append                             (size_t                  pi_Size = 1);


    _HDLLu      void            Swap                               (Packet&                 pio_rRight);

    _HDLLu      bool            IsReadOnly                         () const;
    _HDLLu      bool            IsBufferOwner                      () const;

    // Internal methods
    HCDPacket&                  EditPacket                         ();
    const HCDPacket&            GetPacket                          () const;

private:

    _HDLLu bool                 IsSharedPacket                     () const;

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


#include <ImagePP/all/h/HPUPacket.hpp>

} // End namespace HPU