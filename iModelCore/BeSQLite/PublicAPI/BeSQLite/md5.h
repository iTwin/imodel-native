/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/md5.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

// This file was adapted from :

// //////////////////////////////////////////////////////////
// sha1.cpp
// Copyright (c) 2014,2015 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

// Under the "ZLIB license". See http://create.stephan-brumme.com/disclaimer.html

// define fixed size integer types
#ifdef _MSC_VER
// Windows
typedef unsigned __int8  uint8_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
// GCC
#include <stdint.h>
#endif

BEGIN_BENTLEY_SQLITE_NAMESPACE 

/// compute MD5 hash
/** Usage:
    MD5 md5;
    Utf8String myHash  = md5("Hello World");     // Utf8String
    Utf8String myHash2 = md5("How are you", 11); // arbitrary data, 11 bytes

    // or in a streaming fashion:

    MD5 md5;
    while (more data available)
      md5.Add(pointer to fresh data, number of new bytes);
    Uff8String myHash3 = md5.GetHashString();
  */
class MD5 //: public Hash
{
public:
  /// split into 64 byte blocks (=> 512 bits), hash is 16 bytes long
  enum { BlockSize = 512 / 8, HashBytes = 16 };

  struct HashVal {unsigned char m_buffer[HashBytes];};

private:
  /// process 64 bytes
  void ProcessBlock(const void* data);
  /// process everything left in the internal buffer
  void ProcessBuffer();

  /// size of processed data in bytes
  uint64_t m_numBytes;
  /// valid bytes in m_buffer
  size_t   m_bufferSize;
  /// bytes not processed yet
  uint8_t  m_buffer[BlockSize];

  enum { HashValues = HashBytes / 4 };
  /// hash, stored as integers
  uint32_t m_hash[HashValues];

public:
  /// same as reset()
  BE_SQLITE_EXPORT MD5();

  /// compute MD5 of a memory block
  BE_SQLITE_EXPORT Utf8String operator()(const void* data, size_t numBytes);

  /// compute MD5 of a string, excluding final zero
  BE_SQLITE_EXPORT Utf8String operator()(Utf8StringCR text);

  /// add arbitrary number of bytes
  BE_SQLITE_EXPORT void Add(const void* data, size_t numBytes);

  /// return latest hash as 32 hex characters
  BE_SQLITE_EXPORT Utf8String GetHashString();
  /// return latest hash as 16 bytes
  BE_SQLITE_EXPORT HashVal GetHashVal();

  /// restart
  BE_SQLITE_EXPORT void Reset();
};
END_BENTLEY_SQLITE_NAMESPACE 

