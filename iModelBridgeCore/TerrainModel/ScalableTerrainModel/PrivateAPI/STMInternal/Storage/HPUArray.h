//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HPUArray.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : IDTMArray
//-----------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HFCPtr.h>
#include <STMInternal/Storage/HPUPacket.h>

namespace HPU {

/*---------------------------------------------------------------------------------**//**
* @description  Helper class used to facilitate typed arrays management. This class can
*               wrap around an existing array without copying it. The referred array
*               is only copied when user tries to edit or resize the array (COW). Moreover,
*               this class can also wrap around an existing non-typed buffer and type it.
*               NOTE: This template may only be used with plain old data(POD) types.
*               TDORAY: Enforce this constraint at compile time when POD detection
*                       trait is available.
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class Array : public BentleyApi::ImagePP::HFCShareableObject<Array<T>>
    {
public:
    typedef BentleyApi::ImagePP::HFCPtr<Array<T>>    Ptr;
    typedef BentleyApi::ImagePP::HFCPtr<Array<T>>    CPtr;

    typedef T                   value_type;
    typedef const value_type*   const_iterator;
    typedef value_type*         iterator;

    // Data constructor
    explicit                    Array                              (const T*                pi_pArray,
                                                                    size_t                  pi_Size);

    explicit                    Array                              (size_t                  pi_Capacity = 0);


    Array                              (const Array&            pi_rRight);
    Array&                      operator=                          (const Array&            pi_rRight);

    virtual                     ~Array                             ();


    bool                        IsValidIterator                    (const_iterator          pi_Iterator) const;

    // Data accessors
    const_iterator              Begin                              () const;
    const_iterator              End                                () const;

    const T*                    Get                                () const;
    size_t                      GetSize                            () const;
    size_t                      GetCapacity                        () const;

    bool                        IsEmpty                            () const;

    iterator                    BeginEdit                          ();
    iterator                    EndEdit                            ();

    T*                          Edit                               ();

    void                        Wrap                               (const Array&            pi_rRight);

    void                        Wrap                               (const T*                pi_pArray,
                                                                    size_t                  pi_Size);

    void                        WrapEditable                       (T*                      pi_pArray,
                                                                    size_t                  pi_Size);

    void                        WrapEditable                       (T*                      pi_pArray,
                                                                    size_t                  pi_Size,
                                                                    size_t                  pi_Capacity);

    bool                        StealOwnership                     (Array&                  pio_rRight);

    iterator                    Resize                             (size_t                  pi_Size);

    void                        Reserve                            (size_t                  pi_NewCapacity);

    void                        Clear                              ();

    iterator                    Erase                              (const_iterator          pi_Begin,
                                                                    const_iterator          pi_End);

    iterator                    Erase                              (const_iterator          pi_Position,
                                                                    size_t                  pi_Size = 1);

    iterator                    Insert                             (const_iterator          pi_Position,
                                                                    size_t                  pi_Size = 1);

    template <typename InputIter>
    void                        Insert                             (const_iterator          pi_Position,
                                                                    InputIter               pi_Begin,
                                                                    InputIter               pi_End);

    iterator                    Append                             (size_t                  pi_Size = 1);


    template <typename InputIter>
    void                        Append                             (InputIter               pi_Begin,
                                                                    InputIter               pi_End);

    void                        push_back                          (const value_type&       pi_rValue);

    void                        Swap                               (Array&                  pio_rRight);

    bool                        IsReadOnly                         () const;
    bool                        IsBufferOwner                      () const;

    // Internal methods
    const Packet&               GetPacket                          () const;
    Packet&                     EditPacket                         ();


private:
    Packet                      m_Packet;
    };

template <typename T>
void                            swap                               (Array<T>&               pio_rLeft,
                                                                    Array<T>&               pio_rRight);

#include <STMInternal/Storage/HPUArray.hpp>

} // End namespace HPU