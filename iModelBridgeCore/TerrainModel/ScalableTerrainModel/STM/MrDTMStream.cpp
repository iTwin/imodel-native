/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMStream.cpp $
|    $RCSfile: MrDTMStream.cpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/06/07 21:10:14 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>
#include <ScalableTerrainModel/IMrDTMStream.h>



BEGIN_BENTLEY_MRDTM_NAMESPACE



BinaryIOS::BinaryIOS   (bios& stream)
    :   m_impl(stream)
    {
    }

BinaryIOS::~BinaryIOS  ()
    {
    }


bool BinaryIOS::good () const
    {
    return m_impl.good();
    }



BinaryOStream::BinaryOStream (bostream& stream)
    :   BinaryIOS(stream),
        m_impl(stream)
    {
    }

BinaryOStream::~BinaryOStream ()
    {
    }

BinaryOStream& BinaryOStream::write(const byte* elements,
                                    streamsize  count)
    {
    m_impl.write(elements, count);
    return *this;
    }

BinaryOStream& BinaryOStream::put (byte element)
    {
    m_impl.put(element);
    return *this;
    }


BinaryIStream::BinaryIStream (bistream& stream)
    :   BinaryIOS(stream),
        m_impl(stream)
    {
    }

BinaryIStream::~BinaryIStream ()
    {
    }

BinaryIStream& BinaryIStream::read (byte*       elements,
                                    streamsize  count)
    {
    m_impl.read(elements, count);
    return *this;
    }

BinaryIStream::int_type BinaryIStream::get ()
    {
    return m_impl.get();
    }

BinaryIStream::int_type BinaryIStream::peek ()
    {
    return m_impl.peek();
    }


BinaryIOStream::BinaryIOStream (biostream& stream)
    :   BinaryIOS(stream),
        BinaryIStream(stream), BinaryOStream(stream)
    {
    }

BinaryIOStream::~BinaryIOStream  ()
    {
    }


END_BENTLEY_MRDTM_NAMESPACE