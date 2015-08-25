/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshStream.hpp $
|    $RCSfile: IScalableMeshStream.hpp,v $
|   $Revision: 1.6 $
|       $Date: 2011/11/18 15:51:11 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/

// Do not call directly. Persisted data should be explicitly typed. Call ReadStringW or ReadStringA
template <class String_T>
bool __ReadString_T (BinaryIStream& stream, String_T& myString)
    {
    typedef String_T::value_type CharT;

    static const CharT endOfStr = CharT(); // TDORAY: Find where this can be found

    CharT oneChar;

    bool success = true;
    do 
        {
        stream.read(reinterpret_cast<byte*>(&oneChar), sizeof(CharT));
        if (endOfStr == oneChar)
            break;

        if (!stream.good())
            {
            success = false;
            break;
            }

        myString.push_back(oneChar);
        } 
    while (true);

    return success;
    }

inline bool ReadStringA (BinaryIStream& stream, AStringR myString)
    {
    return __ReadString_T(stream, myString);
    }

inline bool ReadStringW (BinaryIStream& stream, WStringR myString)
    {
    return __ReadString_T(stream, myString);
    }

// Do not call directly. Persisted data should be explicitly typed. Call WriteStringW or WriteStringA
template <class String_T>
bool __WriteString_T (BinaryOStream& stream, String_T const& myString)
    {
    stream.write(reinterpret_cast<const byte*>(myString.c_str()), (UInt)(sizeof(String_T::value_type)*(myString.size() + 1)));
    return stream.good();
    }

inline bool WriteStringA (BinaryOStream& stream, AStringCR cstring)
    {
    return __WriteString_T(stream, cstring);
    }

inline bool WriteStringW (BinaryOStream& stream, WStringCR cstring)
    {
    return __WriteString_T(stream, cstring);
    }


template <typename ValueT>
bool ReadValue (BinaryIStream& stream, ValueT& value)
    {
    stream.read(reinterpret_cast<byte*>(&value), sizeof(value));
    return stream.good();
    }

template <typename ValueT>
bool WriteValue (BinaryOStream& stream, const ValueT& value)
    {
    stream.write(reinterpret_cast<const byte*>(&value), sizeof(value));
    return stream.good();
    }

template <typename ValueT>
bool WriteArray (BinaryOStream& stream, const ValueT* array, size_t size)
    {
    const size_t sizeInBytes = sizeof(array[0])*size;
    assert(sizeInBytes <= (numeric_limits<uint32_t>::max)());

    const uint32_t sizeInBytesField = static_cast<uint32_t>(sizeInBytes);

    stream.write(reinterpret_cast<const byte*>(&sizeInBytesField), sizeof(sizeInBytesField));
    stream.write(reinterpret_cast<const byte*>(array), sizeInBytesField);

    return stream.good();
    }

template <typename ValueT, typename AllocT>
bool WriteArray (BinaryOStream& stream, std::vector<ValueT, AllocT>& array)
    {
    return WriteArray(stream, &array[0], array.size());
    }

template <typename ValueT, typename AllocT>
bool ReadArray (BinaryIStream& stream, std::vector<ValueT, AllocT>& array)
    {
    uint32_t sizeInBytesField = 0;
    stream.read(reinterpret_cast<byte*>(&sizeInBytesField), sizeof(sizeInBytesField));

    assert(0 == sizeInBytesField % sizeof(array[0]));
    const size_t size = sizeInBytesField / sizeof(array[0]);

    array.resize(size);
    stream.read(reinterpret_cast<byte*>(&array[0]), sizeInBytesField);

    return stream.good();
    }

