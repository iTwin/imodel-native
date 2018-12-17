//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCBuffer.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCBuffer.hpp
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Constructor for this class.

 Example:
 @code
 HFCBuffer MyBuffer(1024, 8192); // growth size = 1024, Max Size = 8192
 @end

 @param pi_GrowSize The size in bytes by which the buffer is to be grown
                    when the free space is insufficient for new data.

 @param pi_MaxSize  The maximum size in bytes the buffer can reach.
-----------------------------------------------------------------------------*/
inline HFCBuffer::HFCBuffer(size_t pi_GrowSize, size_t pi_MaxSize)
    {
    HPRECONDITION(pi_GrowSize > 0);
    HPRECONDITION(pi_MaxSize > 0);

    m_MaxSize    = pi_MaxSize;
    m_GrowSize   = pi_GrowSize;
    m_pBuffer    = 0;
    m_BufferSize = 0;
    Clear();
    }


/**----------------------------------------------------------------------------
 Constructor for this class.

 Example:
 @code
 HFCBuffer MyBuffer(1024, 8192); // growth size = 1024, Max Size = 8192
 @end

 @param pi_pDataBuffer The data buffer.

 @param pi_BufferSize  The Size of the data buffer.

 @param pi_GrowSize    The size in bytes by which the buffer is to be grown
                       when the free space is insufficient for new data.

 @param pi_MaxSize     The maximum size in bytes the buffer can reach.
-----------------------------------------------------------------------------*/
inline HFCBuffer::HFCBuffer(Byte* pi_pDataBuffer,
                            size_t pi_BufferSize,
                            size_t pi_GrowSize,
                            size_t pi_MaxSize)
    {
    HPRECONDITION(pi_BufferSize > 0);
    HPRECONDITION(pi_GrowSize > 0);
    HPRECONDITION(pi_MaxSize > 0);

    m_MaxSize    = pi_MaxSize;
    m_GrowSize   = pi_GrowSize;
    m_pBuffer    = pi_pDataBuffer;
    m_BufferSize = pi_BufferSize;
    m_pData      = m_pBuffer;
    m_DataSize   = m_BufferSize;
    }


/**----------------------------------------------------------------------------
 Copy constructor for this class.  This creates a new buffer of same size and
 having same content.

 @param pi_rObj Buffer to duplicate.
-----------------------------------------------------------------------------*/
inline HFCBuffer::HFCBuffer(const HFCBuffer& pi_rObj)
    {
    // copy the max size and the grow size
    m_MaxSize  = pi_rObj.m_MaxSize;
    m_GrowSize = pi_rObj.m_GrowSize;

    // Initialize the buffer to nothing
    m_pBuffer    = 0;
    m_BufferSize = 0;
    Clear();

    // Add the data from the entry buffer
    if (pi_rObj.GetDataSize() > 0)
        AddData(pi_rObj.GetData(), pi_rObj.GetDataSize());
    }


/**----------------------------------------------------------------------------
Destructor for this class.
-----------------------------------------------------------------------------*/
inline HFCBuffer::~HFCBuffer()
    {
    delete[] m_pBuffer;
    }


/**----------------------------------------------------------------------------
 Returns a constant pointer to the current data in the buffer.

 Example:

 @code
| If (MyBuffer.GetDataSize() > 0)
| {
|     Object.DoSomething(MyBuffer.GetData());
| }
 @end

 @return A const pointer to the data in the buffer

 @see GetDataSize
-----------------------------------------------------------------------------*/
inline const Byte* HFCBuffer::GetData() const
    {
    HPRECONDITION(m_DataSize > 0);

    return (m_pData);
    }


/**----------------------------------------------------------------------------
  Returns a constant pointer to the current data in the buffer.
  Also, the buffer will be cleared without deleting the internal data
  pointer, so this becomes the responsibility of the caller.

 @return A const pointer to the data in the buffer

 @see GetData
 @see GetDataSize
-----------------------------------------------------------------------------*/
inline const Byte* HFCBuffer::GetDataAndRelease()
    {
    const Byte* pData = m_pData;

    m_pBuffer = 0;
    m_BufferSize = 0;

    Clear();

    return pData;
    }


/**----------------------------------------------------------------------------
 Returns the size in bytes of data currently in the buffer.  The returned
 value is not the size of the buffer.

 Example:

 @code
| If (MyBuffer.GetDataSize() > 0)
| {
|     Object.DoSomething(MyBuffer.GetData());
| }
 @end

 @return An integer containing the amount of data in bytes currently in
         the buffer.

 @see GetData
-----------------------------------------------------------------------------*/
inline size_t HFCBuffer::GetDataSize() const
    {
    return (m_DataSize);
    }

/**----------------------------------------------------------------------------
 Returns the offset, into the data, where a given block is found.   It
 uses a binary comparison, byte-by-byte, until it finds, in the buffer, a
 byte sequence identical to the one specified by @r{pi_Search}.

 @param pi_Search     Bytes containing the data to find in the buffer.

 @return Offset from the start of the buffer (as returned by GetData) where
         the byte was found, or -1 if the byte has not been found.

 @see GetData
-----------------------------------------------------------------------------*/
inline size_t HFCBuffer::SearchFor(Byte pi_Search) const
    {
    size_t Result = -1;

    if (m_DataSize != 0)
        {
        Byte* pSearch = (Byte*)memchr(m_pData, pi_Search, m_DataSize);

        Result = pSearch == 0? -1 : (size_t)pSearch - (size_t)m_pData;
        }

    return (Result);
    }

/**----------------------------------------------------------------------------
 Returns the offset, into the data, where a given block is found.   It
 uses a binary comparison, byte-by-byte, until it finds, in the buffer, a
 byte sequence identical to the one specified by @r{pi_pSearch} whose length
 is specified by @r{pi_SearchSize}.

 @param pi_pSearch    Pointer to a block of bytes containing the data to find
                      in the buffer.
 @param pi_SearchSize Size of the block to find.

 @return Offset from the start of the buffer (as returned by GetData) where
         the block was found, or -1 if the block has not been found.

 @see GetData
-----------------------------------------------------------------------------*/
inline size_t HFCBuffer::SearchFor(const Byte* pi_pSearch, size_t pi_SearchSize) const
    {
    HPRECONDITION(pi_pSearch != 0);
    HPRECONDITION(pi_SearchSize > 0);
    size_t Result = -1;

    size_t Pos = 0;
    while ((Pos < m_DataSize) &&
           (Result == -1) )
        {
        // try to find the first byte of the search string in the buffer
        Byte* pSearch = (Byte*)memchr(m_pData + Pos, pi_pSearch[0], m_DataSize - Pos);
        if (pSearch)
            {
            // Verify if data at the current found location matches the search string
            if ( (pi_SearchSize <= (size_t)(m_pData + m_DataSize - pSearch)) &&
                 (memcmp(pSearch, pi_pSearch, pi_SearchSize) == 0))
                {
                // Found, set the result at the start of the search string
                Result = (size_t)pSearch - (size_t)m_pData;
                }
            else
                {
                // Not Found, set the current position after the character found
                Pos = (size_t)pSearch - (size_t)m_pData + 1;
                }
            }
        else
            {
            // not found, tough luck
            Pos = m_DataSize;
            }

        }

    return (Result);
    }


/**----------------------------------------------------------------------------
 Indicate to the buffer that the pi_DataRead first bytes of the current data
 have been read and can be re-used.

 @param pi_DataRead The amount of data that is not needed anymore and can be
                    re-used. The value of this parameter must be lower or equal
                    to the actual data size.

 @see AddData, GetData, PrepareForNewData, SetNewDataSize
-----------------------------------------------------------------------------*/
inline void HFCBuffer::MarkReadData(size_t pi_DataRead)
    {
    HPRECONDITION(pi_DataRead <= m_DataSize);

    m_DataRead += pi_DataRead;
    m_DataSize -= pi_DataRead;
    m_pData    += pi_DataRead;

    // if the buffer size is higher that the maximum allowable size,
    // resize it
    if (m_BufferSize > m_MaxSize)
        Refit();

    // if the data size reaches 0,  Reset everything to the start of the buffer
    if (m_DataSize == 0)
        {
        m_pData    = m_pBuffer;
        m_DataSize = 0;
        m_DataRead = 0;
        }
    }


/**----------------------------------------------------------------------------
 Adds data from an already known data source.  The buffer size is
 adjusted to accommodate the new data and the data is then copied in the
 buffer.  The current data and its size is then adjusted.

 Example:

 @code
 Byte* pData = MyObject.GetSomething();
 size_t DataSize = MyObject.GetSomethingSize();
 MyBuffer.AddData(pData, DataSize);
 @end

 @param pi_pData    A const pointer to the data to be copied.
 @param pi_DataSize The size in bytes of the data to be copied.

 @see PrepareForNewData, SetNewDataSize
-----------------------------------------------------------------------------*/
inline void HFCBuffer::AddData(const Byte* pi_pData, size_t pi_DataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0);

    // Prepare the buffer for the new data
    Byte* pNew = PrepareForNewData(pi_DataSize);

    // Copy the new data & set the new data size
    memcpy(pNew, pi_pData, pi_DataSize);
    SetNewDataSize(pi_DataSize);
    }


/**----------------------------------------------------------------------------
 This method insure that there is enough free space for new data to be
 directly read in the buffer and returns a non-const pointer to were the
 data is to be read. The SetNewDataSize() method MUST be called after the
 data is read.

 This method exists for performance reason, when the data
 can be written directly from a data inquiry method, to avoid an
 unnecessary memcpy.

 Example:

 @code
| size_t ReadSize = 128 * 1024;  // try to read a max of 128Kb
| Byte* pBuffer = MyBuffer.PrepareForNewData(ReadSize);
| if (MySocket.Receive(pBuffer, &ReadSize) > 0)
|     MyBuffer.SetNewDataSize(ReadSize); // May be less than 128K
 @end

 @param pi_DataSize The possible (maximum) size of data in bytes that will be
                    written directly in the buffer.

 @return A non-const pointer to where the data can be written in the buffer.

 @see SetNewDataSize
-----------------------------------------------------------------------------*/
inline Byte* HFCBuffer::PrepareForNewData(size_t pi_DataSize)
    {
    HPRECONDITION(pi_DataSize > 0);

    // Set the possible data size
#ifdef __HMR_DEBUG_MEMBER
    HDEBUGCODE(m_PossibleNewDataSize = pi_DataSize;);
#endif

    // Compute the amount of free space in the buffer
    size_t FreeSpace = m_BufferSize - m_DataSize - m_DataRead;

    // Verify if we need to allocate new space in the buffer
    if (pi_DataSize > FreeSpace)
        {
        // the new buffer size is the size of the read data, the data and
        // the new data size minus the already available free space
        size_t NewSize = m_DataRead + m_DataSize + pi_DataSize;

        // all the data does not fit in the buffer, grow it
        Resize(NewSize);
        }

    // The resulting pointer is at the end of the actual data
    return (m_pData + m_DataSize);
    }


/**----------------------------------------------------------------------------
 Specifies the actual amount of data in bytes that was actually read into the
 non-const pointer returned by PrepareForNewData.

 Example:

 @code
| size_t ReadSize = 128 * 1024;  // try to read a max of 128Kb
| Byte* pBuffer = MyBuffer.PrepareForNewData(ReadSize);
| if (MySocket.Receive(pBuffer, &ReadSize) > 0)
|     MyBuffer.SetNewDataSize(ReadSize); // May be less than 128K
 @end

 @param pi_DataSize The size in bytes of the data read.

 @see PrepareForNewData
-----------------------------------------------------------------------------*/
inline void HFCBuffer::SetNewDataSize(size_t pi_DataSize)
    {
#ifdef __HMR_DEBUG_MEMBER
    HPRECONDITION(pi_DataSize <= m_PossibleNewDataSize);
#endif

    // Grow the data size and then return the pointer where to write
    m_DataSize  += pi_DataSize;
    }


/**----------------------------------------------------------------------------
 Resets the buffer to an empty state and resize to a size lower than the
 maximum size if needed.

 Example:

 @code
 // the buffer is used for optimization in a class and it must be
 // cleared when starting a method
 MyBuffer.Clear();
 // Prepare the buffer to be used as a temporary buffer
 Byte* pTemp = MyBuffer.PrepareForNewData(KnownSize);
 HASSERT(pTemp);
 // Use the buffer
 MyObject.DoSomething(pBuffer);
 @end
-----------------------------------------------------------------------------*/
inline void HFCBuffer::Clear()
    {
    // Place everything at the start of the buffer with nothig in it
    m_pData    = m_pBuffer;
    m_DataSize = 0;
    m_DataRead = 0;

    // if the buffer size is higher that the maximum allowable size,
    // resize it
    if (m_BufferSize > m_MaxSize)
        Refit();
    }


/**----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
inline void HFCBuffer::Resize(size_t pi_Size)
    {
    // Grow the buffer by a multiple of m_GrowSize  of at least 1
    size_t CurrentBufferSize = m_BufferSize;
    m_BufferSize = (pi_Size/m_GrowSize + (pi_Size%m_GrowSize ? 1 : 0)) * m_GrowSize;
    m_BufferSize = (m_BufferSize > 0 ? m_BufferSize : m_GrowSize);

    // Resize the buffer
    m_pBuffer = (Byte*)renew(m_pBuffer, CurrentBufferSize, m_BufferSize);

    // In case the buffer changed position
    m_pData = m_pBuffer + m_DataRead;
    }


/**----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
inline void HFCBuffer::Refit()
    {
    HPRECONDITION(m_BufferSize > m_MaxSize);

    // if there is a data read offset, copy the data to the start
    // of the buffer
    if (m_DataRead)
        {
        memcpy(m_pBuffer, m_pData, m_DataSize); // do not try this at home...
        m_DataRead = 0;
        m_pData    = m_pBuffer;
        }

    // Resize to fit the data size
    Resize(m_DataSize);
    }

END_IMAGEPP_NAMESPACE