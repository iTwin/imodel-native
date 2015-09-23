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

    memset (&inInfo, 0, sizeof (inInfo));       // Not reall used.

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

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     05/2015
//=======================================================================================
struct MRMeshTextureImage 
{
protected:
    Point2d             m_size;
    bvector<Byte>       m_compressedData;
    bvector<Byte>       m_data;
    
/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshTextureImage (Point2dCR size, bvector<Byte>& data, bvector<Byte>& compressedData) : m_size (size)
    { 
    // store both the compressed and uncompressed -- the uncompressed will be used for the initial
    // display and then freed (to optimize performance and memory usage).
    m_data.swap (data);
    m_compressedData.swap (compressedData);
    }

#ifdef WIP
/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _GetData (Byte* data) const override
    {
    if (!m_data.empty())
        {
        // First time through -- this will be for screen display.  After
        // copying the uncompressed data, free it to reduce memory size.
        memcpy (data, &m_data.front(), m_data.size());

        (const_cast <bvector<Byte>*> (&m_data))->clear();

        return SUCCESS;
        }

    Point2d         size;
    bvector<Byte>   tempData;
    readRGBFromJPEGData (tempData, size, &m_compressedData.front(), m_compressedData.size());

    memcpy (data, &tempData.front(), tempData.size());

    return SUCCESS;
    }
#endif


public:
    static EmbeddedMaterialLayerImagePtr Create (Point2dCR size, bvector<Byte>& data, bvector<Byte>& compressedData) { return new MRMeshTextureImage (size, data, compressedData); }

};  // MRMeshTextureImage


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshTexture::Initialize (MRMeshNodeCR node, MRMeshContextCR host, ViewContextR viewContext)
    {
#ifdef WIP
    if (m_material.IsValid())
        return;
    m_material = Material::Create (viewContext.GetCurrentModel()->GetDgnFileP()->GetDictionaryModel());            // use dictionary model rather than the modelRef directly - else the textures are cleared when the modelRef is closed (TFS# 217354).
    m_material->GetSettingsR().InitDefaults();
    m_material->GetSettingsR().SetHasBaseColor (true);
    m_material->GetSettingsR().SetSpecularIntensity (0.0);          // Per Jerry Flynn.
    m_material->GetPaletteR().SetName (L"Acute3D");

    if (NULL != viewContext.GetCurrentDisplayParams() &&
        0.0 != viewContext.GetCurrentDisplayParams()->GetTransparency())
        m_material->GetSettingsR().SetTransmitIntensity(viewContext.GetCurrentDisplayParams()->GetTransparency()); 

    MaterialMapP                    materialMap   = m_material->GetSettingsR().GetMapsR().AddMap (MaterialMap::MAPTYPE_Pattern);
    MaterialMapLayerR               materialLayer = materialMap->GetLayersR().GetTopLayerR ();
    EmbeddedMaterialLayerImagePtr   embeddedImage = MRMeshTextureImage::Create (m_size, m_data, m_compressedData);
    WString                         name;

    BeFileName::ParseName (NULL, NULL, &name, NULL, node.GetFileName().c_str());

    m_material->SetNameNoLimit (name.c_str());

    materialLayer.SetFileName ((node.GetFileName() + WPrintfString (L"%I64d", host.GetElement().IsValid() ? host.GetElement().GetElementId() : 0)).c_str());
    materialLayer.SetEmbeddedImage (embeddedImage);
#endif
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshTexture::Activate (ViewContextR viewContext)
    {
#ifdef WIP
    ElemMatSymbP elemMatSymb = viewContext.GetElemMatSymb();

    elemMatSymb->SetMaterial (m_material.get());
    viewContext.GetIDrawGeom().ActivateMatSymb (elemMatSymb);
#endif
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MRMeshTexture::IsInitialized() const
    {
    return false;       // WIP
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshTexture::GetMemorySize() const
    {
    size_t      size = m_data.size();

#ifdef WIP
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





























