/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMMeshEnumerator.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include "DTMMeshEnumerator.h"
#using <mscorlib.dll>

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

DTMMeshEnumerator::DTMMeshEnumerator (DTM^ dtm) : m_dtm (dtm)
    {
    DTMMeshEnumeratorPtr native = Bentley::TerrainModel::DTMMeshEnumerator::Create (*m_dtm->Handle);
    m_native = native.get ();
    m_native->AddRef ();
    }

void DTMMeshEnumerator::MaxTriangles::set(int value)
    {
    m_native->SetMaxTriangles (value);
    }

BGEO::PolyfaceHeader^ DTMMeshEnumerator::Enumerator::Current::get()
    {
    return BGEO::PolyfaceHeader::CreateFromNative(System::IntPtr(*m_impl->m_current));
    }

END_BENTLEY_TERRAINMODELNET_NAMESPACE