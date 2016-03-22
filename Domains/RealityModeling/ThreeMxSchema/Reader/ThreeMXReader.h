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

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct BaseMeshNode : RefCountedBase, NonCopyableClass
{
public:
    // read .3mxb file
    // failure: returns false, error message in err
    BentleyStatus Read3MXB(MxStreamBuffer&);
    BentleyStatus Read3MXB(BeFileNameCR filename);

private:
    // Set node directory (useful to access children)
    virtual void _SetDirectory(BeFileNameCR dir) = 0;
    // clear any data
    virtual void _Clear() = 0;
    // Add a child node to some list
    virtual void  _PushNode(const S3NodeInfo& node) = 0;
    // Push the texture back to some texture vector
    virtual void _PushJpegTexture(ByteCP data, uint32_t dataSize) = 0;
    // Add a geometry to some list
    virtual void _AddGeometry (int nodeId, int nbVertices, float* positions, float* normals, int nbTriangles, int* indices, float* textureCoordinates, int textureId) = 0;
};

END_BENTLEY_THREEMX_SCHEMA_NAMESPACE
