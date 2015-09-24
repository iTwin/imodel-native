//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HPUArray.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


template <typename T>
Array<T>::Array(const T*    pi_pArray,
                size_t      pi_Size)
    :   m_Packet(reinterpret_cast<const Byte*>(pi_pArray), pi_Size* sizeof(T))
    {

    }

template <typename T>
Array<T>::Array(size_t pi_BufferSize)
    :   m_Packet(pi_BufferSize* sizeof(T))
    {

    }



template <typename T>
Array<T>::Array (const Array<T>& pi_rRight)
    :   m_Packet(pi_rRight.m_Packet)
    {


    }

template <typename T>
Array<T>& Array<T>::operator= (const Array<T>& pi_rRight)
    {
    // Support self assignment
    if (this == &pi_rRight)
        return *this;

    m_Packet = pi_rRight.m_Packet;

    return *this;
    }


template <typename T>
Array<T>::~Array ()
    {

    }

template <typename T>
bool Array<T>::IsValidIterator (const_iterator pi_Iterator) const
    {
    return Begin() <= pi_Iterator && pi_Iterator <= End();
    }

template <typename T>
typename Array<T>::const_iterator Array<T>::Begin () const
    {
    return reinterpret_cast<const T*>(m_Packet.Begin());
    }

template <typename T>
typename Array<T>::const_iterator Array<T>::End () const
    {
    HPRECONDITION(0 == (m_Packet.GetSize()%sizeof(T)));
    return reinterpret_cast<const T*>(m_Packet.End());
    }

template <typename T>
const T* Array<T>::Get () const
    {
    return Begin();
    }

template <typename T>
size_t Array<T>::GetSize () const
    {
    return distance(Begin(), End());
    }

template <typename T>
bool Array<T>::IsEmpty () const
    {
    return m_Packet.IsEmpty();
    }

template <typename T>
size_t Array<T>::GetCapacity () const
    {
    HPRECONDITION(0 == (m_Packet.GetCapacity()%sizeof(T)));
    return m_Packet.GetCapacity()/sizeof(T);
    }

template <typename T>
typename Array<T>::iterator Array<T>::BeginEdit ()
    {
    return reinterpret_cast<T*>(m_Packet.BeginEdit());
    }

template <typename T>
typename Array<T>::iterator Array<T>::EndEdit ()
    {
    HPRECONDITION(0 == (m_Packet.GetSize()%sizeof(T)));
    return reinterpret_cast<T*>(m_Packet.EndEdit());
    }

template <typename T>
T* Array<T>::Edit ()
    {
    return reinterpret_cast<T*>(m_Packet.Edit());
    }

template <typename T>
void Array<T>::Wrap (const Array& pi_rRight)
    {
    Wrap(pi_rRight.GetPacket());
    }

template <typename T>
void Array<T>::Wrap (const T*    pi_pArray,
                     size_t      pi_Size)
    {
    m_Packet.Wrap(reinterpret_cast<const Byte*>(pi_pArray), pi_Size*sizeof(T));
    }

template <typename T>
void Array<T>::WrapEditable (T*     pi_pArray,
                             size_t  pi_Size)
    {
    m_Packet.WrapEditable(reinterpret_cast<Byte*>(pi_pArray), pi_Size*sizeof(T));
    }

template <typename T>
void Array<T>::WrapEditable    (T*     pi_pArray,
                                size_t  pi_Size,
                                size_t  pi_Capacity)
    {
    m_Packet.WrapEditable(reinterpret_cast<Byte*>(pi_pArray), pi_Size*sizeof(T), pi_Capacity*sizeof(T));
    }

template <typename T>
bool Array<T>::StealOwnership (Array&   pio_rRight)
    {
    return m_Packet.StealOwnership(*pio_rRight.m_Packet);
    }

template <typename T>
typename Array<T>::iterator Array<T>::Resize (size_t pi_Size)
    {
    return reinterpret_cast<iterator>(m_Packet.Resize(pi_Size*sizeof(T)));
    }

template <typename T>
void Array<T>::Reserve (size_t  pi_NewCapacity)
    {
    m_Packet.Reserve(pi_NewCapacity*sizeof(T));
    }

template <typename T>
void Array<T>::Clear ()
    {
    m_Packet.Clear();
    }

template <typename T>
typename Array<T>::iterator Array<T>::Erase    (const_iterator  pi_Begin,
                                                const_iterator  pi_End)
    {
    return reinterpret_cast<iterator>(m_Packet.Erase(reinterpret_cast<Packet::const_iterator>(pi_Begin),
                                                     reinterpret_cast<Packet::const_iterator>(pi_End)));
    }

template <typename T>
typename Array<T>::iterator Array<T>::Erase    (const_iterator  pi_Position,
                                                size_t          pi_Size)
    {
    return reinterpret_cast<iterator>(Erase(pi_Position, pi_Position + pi_Size));
    }

template <typename T>
typename Array<T>::iterator Array<T>::Insert   (const_iterator  pi_Position,
                                                size_t          pi_Size)
    {
    return reinterpret_cast<iterator>(m_Packet.Insert(reinterpret_cast<Packet::const_iterator>(pi_Position), pi_Size*sizeof(T)));
    }

template <typename T>
template <typename InputIter>
void Array<T>::Insert  (const_iterator  pi_Position,
                        InputIter       pi_Begin,
                        InputIter       pi_End)
    {
    HPRECONDITION(pi_Begin <= pi_End);

    const size_t InsertedSize = distance(pi_Begin, pi_End);
    iterator NewInsertionPos = Insert(pi_Position, InsertedSize);

    if ((NewInsertionPos == pi_Position) &&
        (pi_Position <= &*pi_Begin) && (&*pi_End <= End()))
        {
        // Buffer was not reallocated, inserted data is part
        // of the buffer and located at or after insertion
        // position. Shift inserted range to match internal
        // data shift after the insertion.
        HASSERT(&typeid(pi_Position) == &typeid(pi_Begin));

        advance(pi_Begin, InsertedSize);
        advance(pi_End, InsertedSize);
        }

    copy(pi_Begin, pi_End, NewInsertionPos);
    }


template <typename T>
typename Array<T>::iterator Array<T>::Append   (size_t pi_Size)
    {
    return reinterpret_cast<iterator>(m_Packet.Append(pi_Size*sizeof(T)));
    }

template <typename T>
template <typename InputIter>
void Array<T>::Append  (InputIter   pi_Begin,
                        InputIter   pi_End)
    {
    HPRECONDITION(pi_Begin <= pi_End);
    // TDORAY: May be optimized...
    copy(pi_Begin, pi_End, Append(distance(pi_Begin, pi_End)));
    }

template <typename T>
void Array<T>::push_back (const value_type& pi_rValue)
    {
    if (m_Packet.GetSize() >= m_Packet.GetCapacity())
        {
        size_t NewCapacity = GetCapacity() + 1;
        NewCapacity += NewCapacity >> 1;
        Reserve(NewCapacity);
        }

    *Append() = pi_rValue;
    }

template <typename T>
void Array<T>::Swap (Array& pio_rRight)
    {
    m_Packet.Swap(pio_rRight.m_Packet);
    }

template <typename T>
bool Array<T>::IsReadOnly () const
    {
    return m_Packet.IsReadOnly();
    }

template <typename T>
bool Array<T>::IsBufferOwner () const
    {
    return m_Packet.IsBufferOwner();
    }

template <typename T>
const Packet& Array<T>::GetPacket () const
    {
    return m_Packet;
    }

template <typename T>
Packet& Array<T>::EditPacket ()
    {
    return m_Packet;
    }

template <typename T>
void swap  (Array<T>& pio_rLeft,
            Array<T>& pio_rRight)
    {
    pio_rLeft.Swap(pio_rRight);
    }