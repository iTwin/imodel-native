/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshTexture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifdef OPTION_SS3_BUILD
#include  "MstnIncludes.h"
#include    <FreeImage.h>
#include <QvOutput.h>
#define QV_NO_MSTN_TYPES
#include    <QVision.h>
#else
#include    <DgnView\DgnViewLib.h>
#include    <DgnView\QvOutput.h>
#include    <FreeImage/FreeImage.h>

#define QV_NO_MSTN_TYPES
#include    <QuickVision\QVision.h>
#include <imagelib\imagelibapi.h>
#endif

#include    "MrMesh.h"


USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_MSTNPLATFORM


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt readRGBFromJPEGData (bvector<byte>& rgb, Point2dR size, byte const* jpegData, size_t jpegDataSize)
    {
    FIMEMORY*       memory = FreeImage_OpenMemory (const_cast <byte*> (jpegData), (DWORD) jpegDataSize);
    FIBITMAP*       bitmap;
    StatusInt       status = ERROR;

    if (NULL != (bitmap = FreeImage_LoadFromMemory (FIF_JPEG, memory, JPEG_ACCURATE)))
        {
        size.x = FreeImage_GetWidth (bitmap);
        size.y = FreeImage_GetHeight (bitmap);

        byte const*     inP = FreeImage_GetBits(bitmap);
        size_t          bufferSize = 4 * size.x * size.y;

        rgb.resize (bufferSize);
        for (byte* outP = &rgb.front(), *endP = outP + bufferSize; outP < endP; outP += 4, inP += 3)
            {
            outP[0] = inP[2];
            outP[1] = inP[1];
            outP[2] = inP[0];
            outP[3] = 0xff;
            }
                                                                              
#ifndef OPTION_SS3_BUILD
        mdlImage_mirror (&rgb.front (), &size, IMAGEFORMAT_RGBA, false);  
#endif

        FreeImage_Unload (bitmap);
        status = SUCCESS;
        }

    FreeImage_CloseMemory (memory);
    return status;
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshTexture::MRMeshTexture (byte const* pData, size_t dataSize)
#ifdef OPTION_SS3_BUILD
: m_material (NULL)
#endif
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
struct MRMeshTextureImage : EmbeddedMaterialLayerImage
{
protected:
    Point2d             m_size;
    bvector<byte>       m_compressedData;
    bvector<byte>       m_data;
    
    virtual Point2d _GetSize() const override           { return m_size; };

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshTextureImage (Point2dCR size, bvector<byte>& data, bvector<byte>& compressedData) : m_size (size)
    { 
    // store both the compressed and uncompressed -- the uncompressed will be used for the initial
    // display and then freed (to optimize performance and memory usage).
    m_data.swap (data);
    m_compressedData.swap (compressedData);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _GetData (byte* data) const override
    {
    if (!m_data.empty())
        {
        // First time through -- this will be for screen display.  After
        // copying the uncompressed data, free it to reduce memory size.
        memcpy (data, &m_data.front(), m_data.size());

        (const_cast <bvector<byte>*> (&m_data))->clear();

        return SUCCESS;
        }

    Point2d         size;
    bvector<byte>   tempData;
    readRGBFromJPEGData (tempData, size, &m_compressedData.front(), m_compressedData.size());

    memcpy (data, &tempData.front(), tempData.size());

    return SUCCESS;
    }
public:
    static EmbeddedMaterialLayerImagePtr Create (Point2dCR size, bvector<byte>& data, bvector<byte>& compressedData) { return new MRMeshTextureImage (size, data, compressedData); }

};  // MRMeshTextureImage


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshTexture::Initialize (MRMeshNodeCR node, MRMeshContextCR host, ViewContextR viewContext)
    {

#ifdef OPTION_SS3_BUILD
    if (m_material != 0)
#else
    if (m_material.IsValid())
#endif
        return;

#ifdef OPTION_SS3_BUILD
    m_material->flags.color = 1;
    MaterialMap  *map = mdlMaterial_createMap (m_material, MAPTYPE_Pattern, 0);

    mdlMaterial_setMapOn (m_material, MAPTYPE_Pattern, TRUE);

    WaitForQvThread ();
    qv_defineTexture ((QvTextureID)m_material, NULL, m_size.x, m_size.y, false, 0 /* QV_RGBA_FORMAT */, &data.front ());
    qv_defineTextureMapping ((QvRendMatID)m_material, (QvTextureID)m_material, 1.0, NULL, QV_MAP_PARAMETRIC, QV_SURFACE_MAPPING, NULL, NULL, NULL, NULL, NULL);
    BSIMaterialProperties   *bsiMatP = BSIMaterialProperties::FromMaterial (m_material);
    bsiMatP->flags.sentToQv = TRUE;

#else
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
    ElemMatSymbP elemMatSymb = viewContext.GetElemMatSymb();

#ifdef OPTION_SS3_BUILD
    elemMatSymb->SetRendMatID ((QvRendMatID)m_material);
    viewContext.GetIDrawGeom ()->ActivateMatSymb (elemMatSymb, 0);
#else
    elemMatSymb->SetMaterial (m_material.get());
    viewContext.GetIDrawGeom().ActivateMatSymb (elemMatSymb);
#endif
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MRMeshTexture::IsInitialized() const
    {
#ifdef OPTION_SS3_BUILD
    return  0 != m_material;
#else
    return m_material.IsValid();
#endif
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
MRMeshTexturePtr MRMeshTexture::Create (byte const* pData, size_t dataSize)
    {
    return new MRMeshTexture (pData, dataSize);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshTexture::~MRMeshTexture ()    { ReleaseQVisionCache(); }

void MRMeshTexture::ReleaseQVisionCache ()
    {
#ifdef OPTION_SS3_BUILD
    if (NULL != m_material)
        mdlMaterial_free (m_material);
#else

    if (m_material.IsValid())
        {
        MaterialManager::GetManagerR().ClearQvTexture (m_material->GetName().c_str(), m_material.get());
        m_material = NULL;
        }
#endif
    }





























