/*--------------------------------------------------------------------------------------+
|
|     $Source: Crypt/sha1.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// This file was adapted from :

// //////////////////////////////////////////////////////////
// sha1.cpp
// Copyright (c) 2014,2015 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

// Under the "ZLIB license". See http://create.stephan-brumme.com/disclaimer.html
// no attribution is necessary.


// big endian architectures need #define __BYTE_ORDER __BIG_ENDIAN
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/SHA1.h>

//#ifndef _MSC_VER
//#include <endian.h>
//#endif

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

/// same as reset()
SHA1::SHA1()
{
  Reset();
}


/// restart
void SHA1::Reset()
{
  m_numBytes   = 0;
  m_bufferSize = 0;

  // according to RFC 1321
  m_hash[0] = 0x67452301;
  m_hash[1] = 0xefcdab89;
  m_hash[2] = 0x98badcfe;
  m_hash[3] = 0x10325476;
  m_hash[4] = 0xc3d2e1f0;
}


BEGIN_BENTLEY_SQLITE_NAMESPACE 

  // mix functions for processBlock()
  inline uint32_t f1(uint32_t b, uint32_t c, uint32_t d)
  {
    return d ^ (b & (c ^ d)); // original: f = (b & c) | ((~b) & d);
  }

  inline uint32_t f2(uint32_t b, uint32_t c, uint32_t d)
  {
    return b ^ c ^ d;
  }

  inline uint32_t f3(uint32_t b, uint32_t c, uint32_t d)
  {
    return (b & c) | (b & d) | (c & d);
  }

  inline uint32_t rotate(uint32_t a, uint32_t c)
  {
    return (a << c) | (a >> (32 - c));
  }

  inline uint32_t swap(uint32_t x)
  {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap32(x);
#endif
#ifdef MSC_VER
    return _byteswap_ulong(x);
#endif

    return (x >> 24) |
          ((x >>  8) & 0x0000FF00) |
          ((x <<  8) & 0x00FF0000) |
           (x << 24);
  }

END_BENTLEY_SQLITE_NAMESPACE 


/// process 64 bytes
void SHA1::ProcessBlock(const void* data)
{
  // get last hash
  uint32_t a = m_hash[0];
  uint32_t b = m_hash[1];
  uint32_t c = m_hash[2];
  uint32_t d = m_hash[3];
  uint32_t e = m_hash[4];

  // data represented as 16x 32-bit words
  const uint32_t* input = (uint32_t*) data;
  // convert to big endian
  uint32_t words[80];
  for (int i = 0; i < 16; i++)
#if defined(__BYTE_ORDER) && (__BYTE_ORDER != 0) && (__BYTE_ORDER == __BIG_ENDIAN)
    words[i] = input[i];
#else
    words[i] = swap(input[i]);
#endif

  // extend to 80 words
  for (int i = 16; i < 80; i++)
    words[i] = rotate(words[i-3] ^ words[i-8] ^ words[i-14] ^ words[i-16], 1);

  // first round
  for (int i = 0; i < 4; i++)
  {
    int offset = 5*i;
    e += rotate(a,5) + f1(b,c,d) + words[offset  ] + 0x5a827999; b = rotate(b,30);
    d += rotate(e,5) + f1(a,b,c) + words[offset+1] + 0x5a827999; a = rotate(a,30);
    c += rotate(d,5) + f1(e,a,b) + words[offset+2] + 0x5a827999; e = rotate(e,30);
    b += rotate(c,5) + f1(d,e,a) + words[offset+3] + 0x5a827999; d = rotate(d,30);
    a += rotate(b,5) + f1(c,d,e) + words[offset+4] + 0x5a827999; c = rotate(c,30);
  }

  // second round
  for (int i = 4; i < 8; i++)
  {
    int offset = 5*i;
    e += rotate(a,5) + f2(b,c,d) + words[offset  ] + 0x6ed9eba1; b = rotate(b,30);
    d += rotate(e,5) + f2(a,b,c) + words[offset+1] + 0x6ed9eba1; a = rotate(a,30);
    c += rotate(d,5) + f2(e,a,b) + words[offset+2] + 0x6ed9eba1; e = rotate(e,30);
    b += rotate(c,5) + f2(d,e,a) + words[offset+3] + 0x6ed9eba1; d = rotate(d,30);
    a += rotate(b,5) + f2(c,d,e) + words[offset+4] + 0x6ed9eba1; c = rotate(c,30);
  }

  // third round
  for (int i = 8; i < 12; i++)
  {
    int offset = 5*i;
    e += rotate(a,5) + f3(b,c,d) + words[offset  ] + 0x8f1bbcdc; b = rotate(b,30);
    d += rotate(e,5) + f3(a,b,c) + words[offset+1] + 0x8f1bbcdc; a = rotate(a,30);
    c += rotate(d,5) + f3(e,a,b) + words[offset+2] + 0x8f1bbcdc; e = rotate(e,30);
    b += rotate(c,5) + f3(d,e,a) + words[offset+3] + 0x8f1bbcdc; d = rotate(d,30);
    a += rotate(b,5) + f3(c,d,e) + words[offset+4] + 0x8f1bbcdc; c = rotate(c,30);
  }

  // fourth round
  for (int i = 12; i < 16; i++)
  {
    int offset = 5*i;
    e += rotate(a,5) + f2(b,c,d) + words[offset  ] + 0xca62c1d6; b = rotate(b,30);
    d += rotate(e,5) + f2(a,b,c) + words[offset+1] + 0xca62c1d6; a = rotate(a,30);
    c += rotate(d,5) + f2(e,a,b) + words[offset+2] + 0xca62c1d6; e = rotate(e,30);
    b += rotate(c,5) + f2(d,e,a) + words[offset+3] + 0xca62c1d6; d = rotate(d,30);
    a += rotate(b,5) + f2(c,d,e) + words[offset+4] + 0xca62c1d6; c = rotate(c,30);
  }

  // update hash
  m_hash[0] += a;
  m_hash[1] += b;
  m_hash[2] += c;
  m_hash[3] += d;
  m_hash[4] += e;
}


/// add arbitrary number of bytes
void SHA1::Add(const void* data, size_t numBytes)
{
  const uint8_t* current = (const uint8_t*) data;

  if (m_bufferSize > 0)
  {
    while (numBytes > 0 && m_bufferSize < BlockSize)
    {
      m_buffer[m_bufferSize++] = *current++;
      numBytes--;
    }
  }

  // full buffer
  if (m_bufferSize == BlockSize)
  {
    ProcessBlock((void*)m_buffer);
    m_numBytes  += BlockSize;
    m_bufferSize = 0;
  }

  // no more data ?
  if (numBytes == 0)
    return;

  // process full blocks
  while (numBytes >= BlockSize)
  {
    ProcessBlock(current);
    current    += BlockSize;
    m_numBytes += BlockSize;
    numBytes   -= BlockSize;
  }

  // keep remaining bytes in buffer
  while (numBytes > 0)
  {
    m_buffer[m_bufferSize++] = *current++;
    numBytes--;
  }
}


/// process final block, less than 64 bytes
void SHA1::ProcessBuffer()
{
  // the input bytes are considered as bits strings, where the first bit is the most significant bit of the byte

  // - append "1" bit to message
  // - append "0" bits until message length in bit mod 512 is 448
  // - append length as 64 bit integer

  // number of bits
  size_t paddedLength = m_bufferSize * 8;

  // plus one bit set to 1 (always appended)
  paddedLength++;

  // number of bits must be (numBits % 512) = 448
  size_t lower11Bits = paddedLength & 511;
  if (lower11Bits <= 448)
    paddedLength +=       448 - lower11Bits;
  else
    paddedLength += 512 + 448 - lower11Bits;
  // convert from bits to bytes
  paddedLength /= 8;

  // only needed if additional data flows over into a second block
  unsigned char extra[BlockSize];

  // append a "1" bit, 128 => binary 10000000
  if (m_bufferSize < BlockSize)
    m_buffer[m_bufferSize] = 128;
  else
    extra[0] = 128;

  size_t i;
  for (i = m_bufferSize + 1; i < BlockSize; i++)
    m_buffer[i] = 0;
  for (; i < paddedLength; i++)
    extra[i - BlockSize] = 0;

  // add message length in bits as 64 bit number
  uint64_t msgBits = 8 * (m_numBytes + m_bufferSize);
  // find right position
  unsigned char* addLength;
  if (paddedLength < BlockSize)
    addLength = m_buffer + paddedLength;
  else
    addLength = extra + paddedLength - BlockSize;

  // must be big endian
  *addLength++ = (unsigned char)((msgBits >> 56) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >> 48) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >> 40) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >> 32) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >> 24) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >> 16) & 0xFF);
  *addLength++ = (unsigned char)((msgBits >>  8) & 0xFF);
  *addLength   = (unsigned char)( msgBits        & 0xFF);

  // process blocks
  ProcessBlock(m_buffer);
  // flowed over into a second block ?
  if (paddedLength > BlockSize)
    ProcessBlock(extra);
}


/// return latest hash as 40 hex characters
Utf8String SHA1::GetHashString()
{
  // compute hash (as raw bytes)
  HashVal rawHash = GetHashVal();

  // convert to hex string
  Utf8String result;
  result.reserve(2 * HashBytes);
  for (int i = 0; i < HashBytes; i++)
  {
    static const char dec2hex[16+1] = "0123456789abcdef";
    result += dec2hex[(rawHash.m_buffer[i] >> 4) & 15];
    result += dec2hex[ rawHash.m_buffer[i]       & 15];
  }

  return result;
}


/// return latest hash as bytes
SHA1::HashVal SHA1::GetHashVal()
{
 
  // save old hash if buffer is partially filled
  uint32_t oldHash[HashValues];
  for (int i = 0; i < HashValues; i++)
    oldHash[i] = m_hash[i];

  // process remaining bytes
  ProcessBuffer();

  HashVal raw;
  unsigned char* current = raw.m_buffer;
  for (int i = 0; i < HashValues; i++)
  {
    *current++ = (m_hash[i] >> 24) & 0xFF;
    *current++ = (m_hash[i] >> 16) & 0xFF;
    *current++ = (m_hash[i] >>  8) & 0xFF;
    *current++ =  m_hash[i]        & 0xFF;

    // restore old hash
    m_hash[i] = oldHash[i];
  }
    return raw;
}


/// compute SHA1 of a memory block
Utf8String SHA1::operator()(const void* data, size_t numBytes)
{
  Reset();
  Add(data, numBytes);
  return GetHashString();
}


/// compute SHA1 of a string, excluding final zero
Utf8String SHA1::operator()(const Utf8String& text)
{
  Reset();
  Add(text.c_str(), text.size());
  return GetHashString();
}
