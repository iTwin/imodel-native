//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCBuffer.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCBuffer.h
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCPtr.h>

BEGIN_IMAGEPP_NAMESPACE
/**

    The HFCBuffer class encapsulates the characteristics of a reusable,
    resizable and growing buffer of bytes.  An object of this class is
    particularly useful when a buffer of undetermined size is needed for
    some operations.  It allows specifying what size is needed for an
    operation and will adjust its size if there is no place in the buffer.

    Internally, there are three parts in the buffer organisation: the @t{read
    data}, the @t{current data} and @t{free space} that use the entire allocated
    memory space.  The @t{read data} is the data that was explicitly specified
    by the user as not relevant anymore.  Marking data as read changes the
    pointer to the actual data and the data size.  The @t{current data} is the
    place in the buffer where there is relevant unread data to be used.  The
    @t{free space} is the space left at the end of the buffer.  The free space
    is only known internally.

    @picture{..\..\images\HFCBuffer1.gif}

    When the free space is insufficient for new data, the buffer manager
    will resize the buffer to the next multiple of growth size.  If the data
    to add is already known, the user should call AddData that will adjust
    the buffer if needed, copy the data and adjust its data size.  If the
    data is not already known, but the size or possible size is, the user
    should call PrepareForNewData, which receives the possible size of the
    new data, adjust the size of the buffer and returns a non-const pointer
    at the buffer location were the new data may be written.  After the data
    is written in the buffer, the user MUST call SetNewDataSize to indicate
    the actual amount of data written in.  The superfluous data size that
    was specified becomes more free space.

    The PrepareForNewData and SetNewDataSize methods are implemented for
    performance reason, to avoid an useless memory copy of data.

    When data is read and the data is actually used and not needed anymore,
    the user can call the MarkReadData that will mark the specified number
    of contiguous bytes at the start of the current data as read and will
    update the current data values.

    The maximum buffer size is checked whenever data is marked as read.  If
    the buffer size has grown beyond the maximum size specified at
    construction time, the buffer will then attempt to resize the buffer.
    If there is data marked as read at the start of the buffer, the buffer
    will copy the data from its actual position to the start of the buffer,
    thus creating more free space.  The buffer will then be resized to a
    multiple of the growth size that fits the current data.  Of course, the
    buffer is non-destructive, if the data size is greater than the maximum
    size, it will not be destroyed.

    When the buffer needs to be cleared, the user can call the Clear method
    which will reset the buffer to an empty buffer and then will resize the
    buffer to make sure that the maximum size is respected.

    @h3{Temporary Buffer Utilisation}

    The HFCBuffer class can also be used as a temporary buffer for
    optimization reason in any class, especially when the buffer size is
    unknown at construction time or may change during execution.

    @code{
    MyClass::MyClass
      : MyBuffer(1, ULONG_MAX) // grows by 1 and max value of size_t as limit
    {
        ...
    }

    MyClass::OptimizedMethod()
    {
      // Compute the size needed for the buffer
      size_t SizeNeeded = ComputeSize();

      // Prepare the buffer.
      MyBuffer.Clear();    // to insure that the buffer is cleared
      Byte* pBuffer = MyBuffer.PrepareForNewData(SizeNeeded);

      // Use the data
      DoSomething(pBuffer, SizeNeeded);

      // we do not need to call MyBuffer.SetNewDataSize() because the data was
      // already used and the data now in it is of no relevance.  The data
      // (possibly) allocated by Prepare becomes free space for future calls
      // to this methods.
    }
    }

*/

class HNOVTABLEINIT HFCBuffer : public HFCShareableObject<HFCBuffer >
    {
public:

    //:> Construction/destruction

    HFCBuffer(size_t pi_GrowSize,
              size_t pi_MaximumSize = INT_MAX);

    HFCBuffer(Byte* pi_pDataBuffer,
              size_t pi_BufferSize,
              size_t pi_GrowSize    = 1024,
              size_t pi_MaximumSize = INT_MAX);

    HFCBuffer(const HFCBuffer& pi_rObj);
    ~HFCBuffer();


    //:> methods

    //:> Return the data in the buffer and its size
    const Byte*        GetData() const;
    size_t              GetDataSize() const;

    //:> Return the data and clear without deleting the buffer.
    const Byte*        GetDataAndRelease();

    //:> Search for something in the buffer
    size_t              SearchFor(Byte pi_Search) const;
    size_t              SearchFor(const Byte* pi_pSearch, size_t pi_SearchSize) const;

    //:> Marks a part of the data as read.
    void                MarkReadData(size_t pi_DataRead);

    //:> Adds data to the buffer when data is known
    void                AddData(const Byte* pi_pData, size_t pi_DataSize);

    //:> Prepare place in the buffer of a given size.  Returns
    //:> a non-const pointer to write directly.  Not "entirely"safe,
    //:> but faster.
    Byte*              PrepareForNewData(size_t pi_WriteSize);

    //:> Updates the data size after the previous methods buffer
    //:> pointer was written in
    void                SetNewDataSize(size_t pi_DataSize);

    //:> Clears the buffer
    void                Clear();

private:

    //:> Resizes to a specific size
    void                Resize(size_t pi_Size);

    //:> Resizes the buffer to fit (if possible) the maximum size
    void                Refit();

    //:> Deactivated
    HFCBuffer&          operator=(const HFCBuffer&);

    //--------------------------------------
    //:> Attributes

    // Buffer data
    Byte*          m_pBuffer;
    size_t          m_BufferSize;

    // Current data
    Byte*          m_pData;
    size_t          m_DataSize;

    // Miscellaneous information
    size_t          m_DataRead;

    // Size members
    size_t          m_GrowSize;
    size_t          m_MaxSize;

#ifdef __HMR_DEBUG_MEMBER
    // Possible data size for PrepareForNewData and SetNewDataSize
    size_t          m_PossibleNewDataSize;
#endif
    };

END_IMAGEPP_NAMESPACE

#include "HFCBuffer.hpp"
