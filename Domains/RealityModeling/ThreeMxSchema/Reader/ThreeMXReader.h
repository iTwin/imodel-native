/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/Reader/ThreeMXReader.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include "ThreeMxSchemaInternal.h"

BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct S3NodeInfo
{
    DPoint3d   m_center;
    double    m_radius;
    double    m_dMax;
    bvector<Utf8String> m_children;
    S3NodeInfo() 
        {
        m_center.Zero();
        m_radius = 1.0E10;
        m_dMax   = 0.0;
        }
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct MxStreamBuffer : ByteStream
    {
    uint32_t m_currPos = 0;
    ByteCP GetCurrent() const {return (m_currPos > GetSize()) ? nullptr : GetData() + m_currPos;}
    ByteCP Advance(uint32_t size) {m_currPos += size; return GetCurrent();}
    void SetPos(uint32_t pos) {m_currPos=pos;}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct S3SceneInfo
{
    Utf8String m_sceneName;
    Utf8String m_SRS;
    bvector<double> m_SRSOrigin;
    Utf8String m_navigationMode;
    bvector<Utf8String> m_meshChildren;

    BentleyStatus Read3MX(MxStreamBuffer&);
    BentleyStatus Read3MX(BeFileNameCR filename);
};

END_BENTLEY_THREEMX_SCHEMA_NAMESPACE
