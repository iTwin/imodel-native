/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshTexture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_THREEMX_SCHEMA


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt readRGBFromJPEGData (bvector<Byte>& rgb, Point2dR size, Byte const* jpegData, size_t jpegDataSize)
    {
    ImageUtilities::RgbImageInfo        outInfo, inInfo;

    memset (&inInfo, 0, sizeof (inInfo));       // Not really used.
    inInfo.hasAlpha = true;
    inInfo.isBGR = false;
    inInfo.isTopDown = true;


    if (SUCCESS != ImageUtilities::ReadImageFromJpgBuffer (rgb, outInfo, jpegData, jpegDataSize, inInfo))
        return ERROR;

    size.x = outInfo.width;
    size.y = outInfo.height;

    return SUCCESS;
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshTexture::MRMeshTexture (Byte const* pData, size_t dataSize)
    {
    // This constructor is run in the "reality data" threads.   We'll decompress the data here and then free it
    // after initial use so that we dont have to do it in the main thread..
    // But save the uncompressed data as well which we'll keep around just in case the texture
    // image is required again (by an export application).

    m_compressedData.resize (dataSize);
    memcpy (&m_compressedData.front(), pData, dataSize);
    readRGBFromJPEGData (m_data, m_size, pData, dataSize);
    }


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     09/2015
+===============+===============+===============+===============+===============+======*/
struct MRMeshRenderMaterial : RenderMaterial
{

protected:
    RenderMaterialMapPtr    m_patternMap;
    mutable uintptr_t       m_qvMaterialId;

    MRMeshRenderMaterial (RenderMaterialMapPtr& patternMap) : m_patternMap (patternMap), m_qvMaterialId (0) { }

public:
       virtual     RenderMaterialPtr       _Clone () const override { return new MRMeshRenderMaterial (*this); }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static RenderMaterialPtr Create (Byte const* imageData, Point2dCR imageSize)
    {
    RenderMaterialMapPtr        patternMap = SimpleBufferPatternMap::Create (imageData, imageSize);
    
    return new MRMeshRenderMaterial (patternMap);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
virtual     uintptr_t  _GetQvMaterialId (DgnDbR dgnDb, bool createIfNotFound) const override
    {
    if (createIfNotFound)
        return m_qvMaterialId = (uintptr_t) this;        // These qvMaterials will not be shared -- just use own memory address as qvMaterialId.

    return m_qvMaterialId;

    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
virtual     RenderMaterialMapPtr    _GetMap (char const* key) const override 
    {
    if (0 != strcmp (key, RENDER_MATERIAL_MAP_Pattern))
        {
        BeAssert (false);
        return nullptr;
        }
        
    return m_patternMap;
    }

};  // MRMeshRenderMaterial


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshTexture::Initialize (MRMeshNodeCR node, MRMeshContextCR host, ViewContextR viewContext)
    {
    if (!m_material.IsValid())
        m_material = MRMeshRenderMaterial::Create (&m_data.front(), m_size);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshTexture::Activate (ViewContextR viewContext)
    {
    if (!m_material.IsValid())
        {
        BeAssert (false);
        return;
        }

    ElemMatSymbP    elemMatSymb = viewContext.GetElemMatSymb();

    elemMatSymb->SetMaterial (m_material.get());
    viewContext.GetIDrawGeom ().ActivateMatSymb (elemMatSymb);
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MRMeshTexture::IsInitialized() const
    {
    return m_material.IsValid();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshTexture::GetMemorySize() const
    {
    size_t      size = m_data.size();

    if (m_material.IsValid())
        size += m_size.x * m_size.y * 8;     // Approximate memory for QVision and embedded material.

    return size;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshTexturePtr MRMeshTexture::Create (Byte const* pData, size_t dataSize)
    {
    return new MRMeshTexture (pData, dataSize);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshTexture::~MRMeshTexture ()    { ReleaseQVisionCache(); }

void MRMeshTexture::ReleaseQVisionCache ()
    {
#ifdef WIP
    if (m_material.IsValid())
        {
        MaterialManager::GetManagerR().ClearQvTexture (m_material->GetName().c_str(), m_material.get());
        m_material = NULL;
        }
#endif
    }





























