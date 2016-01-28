/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshTexture.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_THREEMX_SCHEMA


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt readBGRAFromJPEGData (ByteStream& bgra, Point2dR size, Byte const* jpegData, size_t jpegDataSize)
    {
    ImageUtilities::RgbImageInfo        outInfo, inInfo;

    memset (&inInfo, 0, sizeof (inInfo));       // Not really used.
    inInfo.hasAlpha = true;
    inInfo.isBGR = true;
    inInfo.isTopDown = true;

    if (SUCCESS != ImageUtilities::ReadImageFromJpgBuffer (bgra, outInfo, jpegData, jpegDataSize, inInfo))
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

    m_compressedData.SaveData(pData, (uint32_t)dataSize);
    readBGRAFromJPEGData(m_data, m_size, pData, dataSize);
    }


#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
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
static RenderMaterialPtr Create (ImageBufferR imageBuffer)
    {
    RenderMaterialMapPtr        patternMap = SimpleBufferPatternMap::Create (imageBuffer);
    
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
#endif

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshTexture::Initialize (MRMeshNodeCR node, MRMeshContextCR host, ViewContextR viewContext)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (!m_material.IsValid())
        {
        ImageBufferPtr imageBuffer = ImageBuffer::Create(m_size.x, m_size.y, ImageBuffer::Format::Bgra);
        memcpy(imageBuffer->GetDataP(), m_data.data(), imageBuffer->GetDataSize());

        m_material = MRMeshRenderMaterial::Create (*imageBuffer);
        }
#endif
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshTexture::Activate (ViewContextR viewContext)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (!m_material.IsValid())
        {
        BeAssert (false);
        return;
        }

    ElemMatSymbP    elemMatSymb = viewContext.GetElemMatSymb();

    elemMatSymb->SetMaterial (m_material.get());
    viewContext.GetIDrawGeom ().ActivateMatSymb (elemMatSymb);
#endif
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MRMeshTexture::IsInitialized() const
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    return m_material.IsValid();
#else
    return false;
#endif
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshTexture::GetMemorySize() const
    {
    size_t      size = m_data.GetSize();

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (m_material.IsValid())
        size += m_size.x * m_size.y * 8;     // Approximate memory for QVision and embedded material.
#endif

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





























