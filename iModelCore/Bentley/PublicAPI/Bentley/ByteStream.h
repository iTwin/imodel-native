/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/ByteStream.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Bentley.h"
#include "BeAssert.h"
#include <utility>
#include <string.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! A stream of bytes in a resizeable buffer. Released on destruction, never gets smaller.
//! This class is more efficient than bvector<byte> since it does not initialize the memory to zeros.
// @bsiclass                                                    Keith.Bentley   11/15
//=======================================================================================
struct ByteStream
{
private:
    uint32_t m_size;
    uint32_t m_allocSize;
    uint8_t* m_data;
    void swap(ByteStream& rhs) {std::swap(m_size,rhs.m_size); std::swap(m_allocSize,rhs.m_allocSize); std::swap(m_data,rhs.m_data);}

public:
    void Init() {m_size=m_allocSize=0; m_data=nullptr;}
    ByteStream() {Init();}
    explicit ByteStream(uint32_t size) {Init(); Resize(size);}
    ByteStream(uint8_t const* data, uint32_t size) {Init(); SaveData(data, size);}
    ByteStream(ByteStream const& other) {Init(); SaveData(other.m_data, other.m_size);}
    ~ByteStream() {Clear();}
    ByteStream(ByteStream&& rhs) : m_size(rhs.m_size), m_allocSize(rhs.m_allocSize), m_data(rhs.m_data) {rhs.m_size=rhs.m_allocSize=0; rhs.m_data=nullptr;}
    ByteStream& operator=(ByteStream const& other) {if (this != &other) SaveData(other.m_data, other.m_size); return *this;}
    ByteStream& operator=(ByteStream&& rhs) {ByteStream(std::move(rhs)).swap(*this); return *this;}

    //! Get the size, in bytes, of the memory allocated for this ByteStream.
    //! @note The allocated size may be larger than the currently used size returned by GetSize.
    uint32_t GetAllocSize() const {return m_allocSize;}
    uint32_t GetSize() const {return m_size;}   //!< Get the size in bytes of the current data in this ByteStream.
    uint8_t const* GetData() const {return m_data;} //!< Get a const pointer to the ByteStream.
    uint8_t* GetDataP() const {return m_data;}      //!< Get a writable pointer to the ByteStream.
    bool HasData() const {return 0 != m_size;}  //!< return false if this ByteStream is empty.
    void Clear() {FREE_AND_CLEAR(m_data); m_size = m_allocSize = 0;} //!< Return this object to an empty/uninitialized state.
    uint8_t* ExtractData() { uint8_t* data = m_data; m_data = nullptr; m_size = m_allocSize = 0; return data; }

    //! Reserve memory for this ByteStream. The stream capacity will change but not its size.
    //! @param[in] size the number of bytes to reserve
    void Reserve(uint32_t size) {if (size<=m_allocSize) return; m_data = (uint8_t*) realloc(m_data, size); m_allocSize = size;}

    //! Resize the stream. If more memory is required, the new portion won't be initialized.
    //! @param[in] newSize number of bytes
    void Resize(uint32_t newSize) { Reserve(newSize); m_size = newSize; }

    //! Save a stream of bytes into this ByteStream.
    //! @param[in] data the data to save
    //! @param[in] size number of bytes in data
    void SaveData(uint8_t const* data, uint32_t size) {m_size = 0; Append(data, size);}

    //! Append a stream of byes to the current end of this ByteStream.
    //! @param[in] data the data to save
    //! @param[in] size number of bytes in data
    void Append(uint8_t const* data, uint32_t size) 
        {
        if (data)
            {
            Reserve(m_size + size);
            memcpy(m_data + m_size, data, size);
            m_size += size;
            }
        }

    bool empty() const { return !HasData(); }
    size_t size() const { return GetSize(); }
    size_t capacity() const { return GetAllocSize(); }
    void reserve(size_t size) { Reserve(static_cast<uint32_t>(size)); }
    void resize(size_t newSize) { Resize(static_cast<uint32_t>(newSize)); }
    void clear() { Clear(); }
    uint8_t const* data() const { return GetData(); }
    uint8_t* data() { return GetDataP(); }

    typedef uint8_t* iterator;
    typedef uint8_t const* const_iterator;

    iterator begin() { return data(); }
    iterator end() { return data() + size(); }
    const_iterator begin() const { return data(); }
    const_iterator end() const { return data() + size(); }
    uint8_t const& operator[](size_t i) const { return data()[i]; }
    uint8_t& operator[](size_t i) { return data()[i]; }
};

END_BENTLEY_NAMESPACE
