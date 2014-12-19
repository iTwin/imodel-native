// Copyright 2005 and onwards Google Inc. 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are 
// met: 
// 
// * Redistributions of source code must retain the above copyright 
// notice, this list of conditions and the following disclaimer. 
// * Redistributions in binary form must reproduce the above 
// copyright notice, this list of conditions and the following disclaimer 
// in the documentation and/or other materials provided with the 
// distribution. 
// * Neither the name of Google Inc. nor the names of its 
// contributors may be used to endorse or promote products derived from 
// this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// A light-weight compression algorithm.  It is designed for speed of
// compression and decompression, rather than for the utmost in space
// savings.
//
// For getting better compression ratios when you are compressing data
// with long repeated sequences or compressing data that is similar to
// other data, while still compressing fast, you might look at first
// using BMDiff and then compressing the output of BMDiff with
// Snappy.

#ifndef UTIL_SNAPPY_SNAPPY_H__
#define UTIL_SNAPPY_SNAPPY_H__

#include <stddef.h>
#include <string.h>
#include <string>

#if defined(_M_X64)
#define __x86_64__ 1
#endif

#include "snappy-stubs-public.h"

namespace snappy {
// A Sink is an interface that consumes a sequence of bytes.
class Sink {
 public:
  Sink() { }
  virtual ~Sink() {}

  // Append "bytes[0,n-1]" to this.
  virtual void Append(const char* bytes, size_t n) = 0;

  // Returns a writable buffer of the specified length for appending.
  // May return a pointer to the caller-owned scratch buffer which
  // must have at least the indicated length.  The returned buffer is
  // only valid until the next operation on this Sink.
  //
  // After writing at most "length" bytes, call Append() with the
  // pointer returned from this function and the number of bytes
  // written.  Many Append() implementations will avoid copying
  // bytes if this function returned an internal buffer.
  //
  // If a non-scratch buffer is returned, the caller may only pass a
  // prefix of it to Append().  That is, it is not correct to pass an
  // interior pointer of the returned array to Append().
  //
  // The default implementation always returns the scratch buffer.
  virtual char* GetAppendBuffer(size_t length, char* scratch) {return scratch;}

 private:
  // No copying
  Sink(const Sink&);
  void operator=(const Sink&);
};

// A Source is an interface that yields a sequence of bytes
class Source {
 public:
  Source() { }
  virtual ~Source() {}

  // Return the number of bytes left to read from the source
  virtual size_t Available() const = 0;

  // Peek at the next flat region of the source.  Does not reposition
  // the source.  The returned region is empty iff Available()==0.
  //
  // Returns a pointer to the beginning of the region and store its
  // length in *len.
  //
  // The returned region is valid until the next call to Skip() or
  // until this object is destroyed, whichever occurs first.
  //
  // The returned region may be larger than Available() (for example
  // if this ByteSource is a view on a substring of a larger source).
  // The caller is responsible for ensuring that it only reads the
  // Available() bytes.
  virtual const char* Peek(size_t* len) = 0;

  // Skip the next n bytes.  Invalidates any buffer returned by
  // a previous call to Peek().
  // REQUIRES: Available() >= n
  virtual void Skip(size_t n) = 0;

 private:
  // No copying
  Source(const Source&);
  void operator=(const Source&);
};

// A Source implementation that yields the contents of a flat array
class ByteArraySource : public Source {
 public:
  ByteArraySource(const char* p, size_t n) : ptr_(p), left_(n) { }
  virtual ~ByteArraySource() {}
  virtual size_t Available() const { return left_; }
  virtual const char* Peek(size_t* len) {  *len = left_;  return ptr_;}
  virtual void Skip(size_t n) {  left_ -= n;  ptr_ += n;}
 private:
  const char* ptr_;
  size_t left_;
};

// A Sink implementation that writes to a flat array without any bound checks.
class UncheckedByteArraySink : public Sink {
 public:
  explicit UncheckedByteArraySink(char* dest) : dest_(dest) { }
  virtual ~UncheckedByteArraySink(){}
  virtual void Append(const char* data, size_t n){  // Do no copying if the caller filled in the result of GetAppendBuffer()
    if (data != dest_) {
        memcpy(dest_, data, n);
    }
    dest_ += n;
  }
  virtual char* GetAppendBuffer(size_t len, char* scratch) {return dest_;}

  // Return the current output pointer so that a caller can see how
  // many bytes were produced.
  // Note: this is not a Sink method.
  char* CurrentDestination() const { return dest_; }
 private:
  char* dest_;
};

  // ------------------------------------------------------------------------
  // Generic compression/decompression routines.
  // ------------------------------------------------------------------------

  // Compress the bytes read from "*source" and append to "*sink". Return the
  // number of bytes written.
  size_t Compress(Source* source, Sink* sink);

  bool GetUncompressedLength(Source* source, uint32* result);

  // ------------------------------------------------------------------------
  // Higher-level string based routines (should be sufficient for most users)
  // ------------------------------------------------------------------------

  // Sets "*output" to the compressed version of "input[0,input_length-1]".
  // Original contents of *output are lost.
  //
  // REQUIRES: "input[]" is not an alias of "*output".
  size_t Compress(const char* input, size_t input_length, string* output);

  // Decompresses "compressed[0,compressed_length-1]" to "*uncompressed".
  // Original contents of "*uncompressed" are lost.
  //
  // REQUIRES: "compressed[]" is not an alias of "*uncompressed".
  //
  // returns false if the message is corrupted and could not be decompressed
  bool Uncompress(const char* compressed, size_t compressed_length,
                  string* uncompressed);


  // ------------------------------------------------------------------------
  // Lower-level character array based routines.  May be useful for
  // efficiency reasons in certain circumstances.
  // ------------------------------------------------------------------------

  // REQUIRES: "compressed" must point to an area of memory that is at
  // least "MaxCompressedLength(input_length)" bytes in length.
  //
  // Takes the data stored in "input[0..input_length]" and stores
  // it in the array pointed to by "compressed".
  //
  // "*compressed_length" is set to the length of the compressed output.
  //
  // Example:
  //    char* output = new char[snappy::MaxCompressedLength(input_length)];
  //    size_t output_length;
  //    RawCompress(input, input_length, output, &output_length);
  //    ... Process(output, output_length) ...
  //    delete [] output;
  void RawCompress(const char* input,
                   size_t input_length,
                   char* compressed,
                   unsigned int* compressed_length);

  // Given data in "compressed[0..compressed_length-1]" generated by
  // calling the Snappy::Compress routine, this routine
  // stores the uncompressed data to
  //    uncompressed[0..GetUncompressedLength(compressed)-1]
  // returns false if the message is corrupted and could not be decrypted
  bool RawUncompress(const char* compressed, size_t compressed_length,
                     char* uncompressed);

  // Given data from the byte source 'compressed' generated by calling
  // the Snappy::Compress routine, this routine stores the uncompressed
  // data to
  //    uncompressed[0..GetUncompressedLength(compressed,compressed_length)-1]
  // returns false if the message is corrupted and could not be decrypted
  bool RawUncompress(Source* compressed, char* uncompressed);

  // Returns the maximal size of the compressed representation of
  // input data that is "source_bytes" bytes in length;
  size_t MaxCompressedLength(size_t source_bytes);

  // REQUIRES: "compressed[]" was produced by RawCompress() or Compress()
  // Returns true and stores the length of the uncompressed data in
  // *result normally.  Returns false on parsing error.
  // This operation takes O(1) time.
  bool GetUncompressedLength(const char* compressed, size_t compressed_length,
                             size_t* result);

  // Returns true iff the contents of "compressed[]" can be uncompressed
  // successfully.  Does not return the uncompressed data.  Takes
  // time proportional to compressed_length, but is usually at least
  // a factor of four faster than actual decompression.
  bool IsValidCompressedBuffer(const char* compressed,
                               size_t compressed_length);

  // *** DO NOT CHANGE THE VALUE OF kBlockSize ***
  //
  // New Compression code chops up the input into blocks of at most
  // the following size.  This ensures that back-references in the
  // output never cross kBlockSize block boundaries.  This can be
  // helpful in implementing blocked decompression.  However the
  // decompression code should not rely on this guarantee since older
  // compression code may not obey it.
  static const int kBlockLog = 15;
  static const int kBlockSize = 1 << kBlockLog;

  static const int kMaxHashTableBits = 14;
  static const int kMaxHashTableSize = 1 << kMaxHashTableBits;

}  // end namespace snappy


#endif  // UTIL_SNAPPY_SNAPPY_H__
