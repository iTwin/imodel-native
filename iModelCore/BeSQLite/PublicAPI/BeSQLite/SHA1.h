/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/SHA1.h $
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

/// compute SHA1 hash
/** Usage:
    SHA1 sha1;
    Utf8String myHash  = sha1("Hello World");     // Utf8String
    Utf8String myHash2 = sha1("How are you", 11); // arbitrary data, 11 bytes

    // or in a streaming fashion:

    SHA1 sha1;
    while (more data available)
      sha1.Add(pointer to fresh data, number of new bytes);
    Utf8String myHash3 = sha1.GetHashString();
  */
class BE_SQLITE_EXPORT SHA1 //: public Hash
{
public:
  /// split into 64 byte blocks (=> 512 bits), hash is 20 bytes long
  enum { BlockSize = 512 / 8, HashBytes = 20 };

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
  SHA1();

  /// compute SHA1 of a memory block
  Utf8String operator()(const void* data, size_t numBytes);
  /// compute SHA1 of a string, excluding final zero
  Utf8String operator()(Utf8StringCR text);

  /// add arbitrary number of bytes
  void Add(const void* data, size_t numBytes);

  /// return latest hash as 40 hex characters
  Utf8String GetHashString();

  /// return latest hash as 20 bytes
  HashVal GetHashVal();

  /// restart
  void Reset();
};

END_BENTLEY_SQLITE_NAMESPACE 
