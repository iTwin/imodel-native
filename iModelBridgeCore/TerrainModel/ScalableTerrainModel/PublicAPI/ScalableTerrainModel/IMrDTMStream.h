/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/IMrDTMStream.h $
|    $RCSfile: IMrDTMStream.h,v $
|   $Revision: 1.6 $
|       $Date: 2011/11/18 15:51:10 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


#include <TerrainModel/TerrainModel.h>
#include <iosfwd>

BEGIN_BENTLEY_MRDTM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct BinaryIOS
    {
private:
    typedef basic_ios<byte>             bios;
    bios&                               m_impl;

    // Uncopiable
                                        BinaryIOS                              (BinaryIOS&);
    BinaryIOS&                          operator=                              (BinaryIOS&);

protected:
    explicit                            BinaryIOS                              (bios&                   stream);
    virtual                             ~BinaryIOS                             ();
public:
    BENTLEYSTM_EXPORT bool                    good                                   () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct BinaryOStream : virtual public BinaryIOS
    {
    typedef basic_ostream<byte>         bostream;
    typedef UInt                        streamsize;
private:
    bostream&                           m_impl;

public:
    // NOTE: Constructor meant only to be used inside this module. Do not export.
    explicit                            BinaryOStream                          (bostream&               stream);
    virtual                             ~BinaryOStream                         ();

    BENTLEYSTM_EXPORT BinaryOStream&          write                                  (const byte*             elements,
                                                                                streamsize              count);

    BENTLEYSTM_EXPORT BinaryOStream&          put                                    (byte                    element);

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct BinaryIStream : virtual public BinaryIOS
    {
    typedef basic_istream<byte>         bistream;
    typedef int                         int_type;

private:
    bistream&                           m_impl;

public:
    // NOTE: Constructor meant only to be used inside this module. Do not export.
    explicit                            BinaryIStream                          (bistream&               stream);
    virtual                             ~BinaryIStream                         ();

    BENTLEYSTM_EXPORT BinaryIStream&          read                                   (byte*                   elements,
                                                                                streamsize              count);

    BENTLEYSTM_EXPORT int_type                get                                    ();
    BENTLEYSTM_EXPORT int_type                peek                                   ();

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct BinaryIOStream : public BinaryOStream, public BinaryIStream
    {
    typedef basic_iostream<byte>        biostream;
public:
    // NOTE: Constructor meant only to be used inside this module. Do not export.
    explicit                            BinaryIOStream                         (biostream&              stream);
    virtual                             ~BinaryIOStream                        ();
    };

bool ReadStringA (BinaryIStream& stream, AStringR myString);
bool ReadStringW (BinaryIStream& stream, WStringR myString);

bool WriteStringA (BinaryOStream& stream, AStringCR cstring);
bool WriteStringW (BinaryOStream& stream, WStringCR cstring);
inline bool WriteStringA (BinaryOStream& stream, CharCP cstring)  {return WriteStringA(stream, AString(cstring));}
inline bool WriteStringW (BinaryOStream& stream, WCharCP cstring) {return WriteStringW(stream, WString(cstring));}

template <typename ValueT>
bool ReadValue (BinaryIStream& stream, ValueT& value);

template <typename ValueT>
bool WriteValue (BinaryOStream& stream, const ValueT& value);

template <typename ValueT>
bool WriteArray (BinaryOStream& stream, const ValueT* array, size_t size);

template <typename ValueT, typename AllocT>
bool WriteArray (BinaryOStream& stream, std::vector<ValueT, AllocT>& array);

template <typename ValueT, typename AllocT>
bool ReadArray (BinaryIStream& stream, std::vector<ValueT, AllocT>& array);

#include <ScalableTerrainModel/IMrDTMStream.hpp>

END_BENTLEY_MRDTM_NAMESPACE