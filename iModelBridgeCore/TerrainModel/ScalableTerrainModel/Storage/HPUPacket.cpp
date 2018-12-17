//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HPUPacket.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HPUPacket.h>

namespace HPU {

/*---------------------------------------------------------------------------------**//**
* @description  Construct a packet around a c string. Packet will contain the string
*               + its null terminator. The packet will be read-only.
* @param        pi_pStr     A null terminated string.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::Packet (const char* pi_pStr)
    :   m_pDataPacket(new HCDPacket()),
        m_ReadOnly(true)
    {
    WrapStr(pi_pStr);
    }

/*---------------------------------------------------------------------------------**//**
* @description  Construct a packet around an existing buffer. The packet will be read
*               only.
* @param        pi_pData    A existing buffer.
* @param        pi_Size     The size of the specified buffer.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::Packet (const Byte* pi_pData,
                size_t      pi_Size)
    :   m_pDataPacket(new HCDPacket()),
        m_ReadOnly(true)
    {
    Wrap(pi_pData, pi_Size);
    }

/*---------------------------------------------------------------------------------**//**
* @description  Construct an empty packet with a specified capacity
* @param        pi_Capacity     The initial length of the internal buffer.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::Packet (size_t pi_Capacity)
    :   m_pDataPacket(new HCDPacket()),
        m_ReadOnly(false)
    {
    m_pDataPacket->SetBufferOwnership(true);
    if (0 != pi_Capacity)
        m_pDataPacket->SetBuffer(new Byte[pi_Capacity], pi_Capacity);
    }

/*---------------------------------------------------------------------------------**//**
* @description  Copy constructor.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::Packet (const Packet& pi_rRight)
    {
    // Deep copy. We do not want to get ptr to a buffer that can be modified
    m_pDataPacket = new HCDPacket(pi_rRight.GetPacket());
    m_ReadOnly = false;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Assignment operator.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Packet& Packet::operator= (const Packet& pi_rRight)
    {
    Copy(pi_rRight);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Copy in existing packet. Will only refer the right's data
* @param
* @return
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Copy (const Packet&  pi_rRight)

    {
    HPRECONDITION(!IsSharedPacket());

    // Support self assignment
    if (this == &pi_rRight)
        return;

    // Deep copy. We do not want to get ptr to a buffer that can be modified
    *m_pDataPacket = pi_rRight.GetPacket();
    m_ReadOnly = false;
    }

Packet::~Packet ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns an iterator (pointer) to the beginning of the packet data.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline Packet::iterator Packet::BufferBegin () const
    {
    return m_pDataPacket->GetBufferAddress();
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns an iterator (pointer) to the end of the packet data.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline Packet::iterator Packet::BufferEnd () const
    {
    return m_pDataPacket->GetBufferAddress() + m_pDataPacket->GetDataSize();
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns a const iterator (pointer) to the beginning of the packet data.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline Packet::const_iterator Packet::Begin () const
    {
    return BufferBegin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns a const iterator (pointer) to the end of the packet data.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline Packet::const_iterator Packet::End () const
    {
    return BufferEnd();
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns a const pointer to the packet data.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const Byte* Packet::Get () const
    {
    return m_pDataPacket->GetBufferAddress();
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns the size of the data contained by the packet
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Packet::GetSize () const
    {
    return m_pDataPacket->GetDataSize();
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns the size of the internal buffer that contains the packet data.
*               In other words, this is the limit in size over which a packet will have
*               reallocate itself.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Packet::GetCapacity () const
    {
    return m_pDataPacket->GetBufferSize();
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns whether the packet is empty or not
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Packet::IsEmpty () const
    {
    return 0 == m_pDataPacket->GetDataSize();
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns a pointer to the packet data. Will copy itself in order to become
*               buffer owner if read-only.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Byte* Packet::Edit ()
    {
    HPRECONDITION(!IsSharedPacket());
    //HPRECONDITION (0 != GetSize()); // Trying to edit empty buffer???

    if (m_ReadOnly)
        {
        HASSERT(!m_pDataPacket->HasBufferOwnership()); // If we're read only, we should not have buffer ownership
        const_iterator pOldBegin = Begin();
        const_iterator pOldEnd = End();
        AllocateNewBuffer(GetSize());
        copy(pOldBegin, pOldEnd, BufferBegin());
        }

    return m_pDataPacket->GetBufferAddress();
    }

void Packet::Wrap (const Packet& pio_rRight)
    {
    Wrap(pio_rRight.Get(), pio_rRight.GetSize());
    }


/*---------------------------------------------------------------------------------**//**
* @description  Wrap around a new read only buffer. Will trigger reallocation on edit
*               and also will on operation affecting the size of the buffer.
* @param        pi_pData    The new buffer.
* @param        pi_Size     The size of the new buffer.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Wrap  (const Byte* pi_pData,
                    size_t      pi_Size)
    {
    HPRECONDITION(!IsSharedPacket());
    m_pDataPacket->SetBuffer(const_cast<Byte*>(pi_pData), pi_Size);
    m_pDataPacket->SetDataSize(pi_Size);
    m_pDataPacket->SetBufferOwnership(false);
    m_ReadOnly = true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Wrap around a new buffer that can be edited. Won't trigger reallocation
*               on edit but will on operations affecting the size of the buffer over
*               specified capacity.
* @param        pi_pData    The new buffer.
* @param        pi_Size     The size of the new buffer.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::WrapEditable  (Byte*   pi_pData,
                            size_t  pi_Size,
                            size_t  pi_Capacity)
    {
    HPRECONDITION(!IsSharedPacket());
    m_pDataPacket->SetBuffer(pi_pData, MAX(pi_Size, pi_Capacity));
    m_pDataPacket->SetDataSize(pi_Size);
    m_pDataPacket->SetBufferOwnership(false);
    m_ReadOnly = false;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Wrap around specified packet buffer and steal its ownership when
*               possible. When successful, this packet will be owner of the buffer.
*               Will be unsuccessful when:
*               - Right is not owner -> Will only wrap read-only on right.
*               - Underlying packet is shared -> Do nothing
*
* @param        pio_rRight  The packet from which we want to steal ownership.
* @return       true when successful at stealing ownership false otherwise
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Packet::StealOwnership (Packet& pio_rRight)
    {
    HPRECONDITION(!IsSharedPacket());
    if (m_pDataPacket == pio_rRight.m_pDataPacket)
        return false;

    if (!pio_rRight.IsBufferOwner())
        {
        Wrap(pio_rRight);
        return false;
        }

    WrapEditable(pio_rRight.m_pDataPacket->GetBufferAddress(), pio_rRight.GetSize(), pio_rRight.GetCapacity());
    m_pDataPacket->SetBufferOwnership(true);
    pio_rRight.m_pDataPacket->SetBufferOwnership(false);
    pio_rRight.m_ReadOnly = true;

    return true;
    }


bool Packet::CanBeReadAsStr () const
    {
    if (0 == m_pDataPacket->GetDataSize())
        return false;

    const Byte* pStringEnd = m_pDataPacket->GetBufferAddress() + m_pDataPacket->GetDataSize() - 1;
    if ('\0' != *pStringEnd) // Ensure that the null termination symbol is at the expected position
        return false;

    // String should not contain other termination symbols inside itself.
    HASSERT(find(static_cast<const Byte*>(m_pDataPacket->GetBufferAddress()),
                 pStringEnd,
                 '\0') == pStringEnd);

    return true;
    }

const char* Packet::GetAsStr () const
    {
    if (!CanBeReadAsStr())
        {
        HASSERT(!"Data was not save as a string");
        return 0;
        }

    return (char*)m_pDataPacket->GetBufferAddress();
    }

void Packet::WrapStr (const char* pi_pStr)
    {
    HPRECONDITION(!IsSharedPacket());
    const size_t DataSize = strlen(pi_pStr) + 1;
    m_pDataPacket->SetBuffer((Byte*)pi_pStr, DataSize);
    m_pDataPacket->SetDataSize(DataSize);
    m_pDataPacket->SetBufferOwnership(false);
    m_ReadOnly = true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Specify a new size for the contained data. Specified size should always
*               be less or equal than current packet capacity.
* @param        pi_Size     The new data size.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::SetSize (size_t pi_Size)
    {
    HPRECONDITION(!IsSharedPacket());
    HPRECONDITION(pi_Size <= GetCapacity());
    HPRECONDITION(!m_ReadOnly);
    m_pDataPacket->SetDataSize((std::min)(pi_Size, GetCapacity()));
    }


/*---------------------------------------------------------------------------------**//**
* @description  Resize our packet to specified size. If buffer is not owned, copy its
*               content to a new buffer using specified capacity (or size if not
*               specified). If owned, only set new size if current capacity is enough.
*               Otherwise, realloc buffer using specified capacity.
* @param        pi_Size     The new data size.
*               pi_Capacity The new capacity of the internal buffer. Will be set to
*                           pi_Size if less than pi_Size.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::iterator Packet::Resize (size_t     pi_Size)
    {
    HPRECONDITION(!IsSharedPacket());
    if (pi_Size <= GetSize())
        return Erase(Begin() + pi_Size, End());

    return Append(pi_Size - GetSize());
    }


/*---------------------------------------------------------------------------------**//**
* @description  Clear existing data. If buffer was not owned, it now is.
* @bsimethod                                                  Raymond.Gauthier   5/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Clear ()
    {
    HPRECONDITION(!IsSharedPacket());
    if (m_ReadOnly)
        {
        m_pDataPacket->SetBuffer(0, 0);
        m_pDataPacket->SetBufferOwnership(true);
        m_ReadOnly = false;
        }

    m_pDataPacket->SetDataSize(0);
    }

/*---------------------------------------------------------------------------------**//**
* @description  Erase a range of elements
* @param        pi_Begin
* @param        pi_End
* @return       And iterator the element following the erased range
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::iterator Packet::Erase (const_iterator  pi_Begin,
                                const_iterator  pi_End)
    {
    HPRECONDITION(!IsSharedPacket());
    HPRECONDITION(pi_Begin <= pi_End);
    HPRECONDITION(Begin() <= pi_Begin && pi_End <= End());

    if (!m_ReadOnly)
        {
        iterator NewEnd = copy(pi_End, End(), const_cast<iterator>(pi_Begin));
        m_pDataPacket->SetDataSize(distance(BufferBegin(), NewEnd));
        return const_cast<iterator>(pi_Begin);
        }

    // We do not own the buffer. Reallocate and copy without the erased part.
    const_iterator pOldBegin = Begin();
    const_iterator pOldEnd = End();

    const size_t NewSize = GetSize() - distance(pi_Begin, pi_End);
    AllocateNewBuffer(NewSize);
    m_pDataPacket->SetDataSize(NewSize);

    iterator IterFollowingErasedRange = copy (pOldBegin, pi_Begin, BufferBegin());
    copy (pi_End, pOldEnd, IterFollowingErasedRange);

    return IterFollowingErasedRange;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Allocate a new buffer with specified capacity. We now become buffer
*               owner.
*               NOTE: Be careful when using it when already buffer owner, because the
*               old buffer need to be deleted to avoid memory leaks.
* @param        pi_Capacity     The capacity for the new buffer.
* @return       An iterator/pointer to the old buffer.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::AllocateNewBuffer (size_t pi_Capacity)
    {
    HPRECONDITION(!m_pDataPacket->HasBufferOwnership());
    m_pDataPacket->SetBuffer(new value_type[pi_Capacity], pi_Capacity);
    m_pDataPacket->SetBufferOwnership(true);
    m_ReadOnly = false;
    HPOSTCONDITION(0 != m_pDataPacket->GetBufferAddress());
    }

/*---------------------------------------------------------------------------------**//**
* @description  Reserve a block of memory of a specified size at a specified location
*               in the packet (move existing data after position so that enough space
*               is reserved). If necessary, this method reallocate the internal
*               buffer. The size of the packet is incremented by pi_Size.
* @param        pi_Position     The position at which we want the reserve requested
*                               space size.
* @param        pi_Size         The size of the space that will be reserved at specified
*                               position.
* @return       An iterator to the reserved space (can be different from pi_Position if
*               buffer is reallocated).
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::iterator Packet::Insert(const_iterator      pi_Position,
                                size_t              pi_Size)
    {
    HPRECONDITION(!IsSharedPacket());
    HPRECONDITION(0 != pi_Size);
    HPRECONDITION(pi_Position >= Begin());
    HPRECONDITION(pi_Position <= End()); // Use AppendUnitialized to append at the end
    const size_t NewSize = GetSize() + pi_Size;

    if (!m_ReadOnly && NewSize <= GetCapacity())
        {
        // No need to reallocate. Only moves data located after
        // specified position.
        copy_backward(pi_Position, End(), BufferBegin() + NewSize);
        m_pDataPacket->SetDataSize(NewSize);
        return const_cast<iterator>(pi_Position);
        }

    if (m_pDataPacket->HasBufferOwnership())
        {
        // Reallocate buffer an move data to the new location
        const_iterator pOldEnd = End();
        HArrayAutoPtr<const value_type> pOldBuffer(Begin());
        m_pDataPacket->SetBufferOwnership(false);

        AllocateNewBuffer(NewSize);
        copy_backward(pi_Position, pOldEnd, BufferBegin() + NewSize);
        m_pDataPacket->SetDataSize(NewSize);
        return copy(pOldBuffer.get(), pi_Position, BufferBegin());
        }

    // Reallocate buffer an move data to the new location
    const_iterator pOldEnd = End();
    const_iterator pOldBegin = Begin();

    AllocateNewBuffer(NewSize);
    copy_backward(pi_Position, pOldEnd, BufferBegin() + NewSize);
    m_pDataPacket->SetDataSize(NewSize);
    return copy(pOldBegin, pi_Position, BufferBegin());
    }

/*---------------------------------------------------------------------------------**//**
* @description  Reserve a block of memory of a specified size at at the end of
*               the packet. If necessary, this method reallocate the internal buffer.
*               The size of the packet is incremented by pi_Size.
* @param        pi_Size         The size of the space that will be reserved at the end
*                               of the buffer.
* @return       An iterator to the reserved space.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::iterator Packet::Append (size_t pi_Size)
    {
    HPRECONDITION(!IsSharedPacket());
    HPRECONDITION(0 != pi_Size);

    const size_t NewSize = GetSize() + pi_Size;
    Reserve(NewSize);
    m_pDataPacket->SetDataSize(NewSize);
    return BufferEnd() - pi_Size;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Increase internal buffer size up to pi_NewCapacity. Will trigger a
*               reallocation if the new capacity is greater than the actual one.
* @param        pi_NewCapacity  New internal buffer capacity.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Reserve (size_t pi_NewCapacity)
    {
    HPRECONDITION(!IsSharedPacket());
    if (pi_NewCapacity <= GetCapacity() && !m_ReadOnly)
        return;

    if (m_pDataPacket->HasBufferOwnership())
        m_pDataPacket->ChangeBufferSize(pi_NewCapacity);
    else
        {
        const_iterator pOldBegin = Begin();
        const_iterator pOldEnd = End();
        AllocateNewBuffer(pi_NewCapacity);
        copy(pOldBegin, pOldEnd, BufferBegin());
        }
    }

void Packet::Swap (Packet& pio_rRight)
    {
    std::swap(m_pDataPacket, pio_rRight.m_pDataPacket);
    std::swap(m_ReadOnly, pio_rRight.m_ReadOnly);
    }

bool Packet::IsSharedPacket () const
    {
    return m_pDataPacket->GetRefCount() > 1;
    }

bool Packet::IsReadOnly () const
    {
    return m_ReadOnly;
    }

bool Packet::IsBufferOwner () const
    {
    return m_pDataPacket->HasBufferOwnership();
    }
} // End namespace HPU