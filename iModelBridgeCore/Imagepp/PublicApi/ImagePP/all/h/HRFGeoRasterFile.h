//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFGeoRasterFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HFCURLFile.h"


BEGIN_IMAGEPP_NAMESPACE
class SDOGeoRasterWrapper;
struct SDOObjectInfo;
struct SDORasterInfo;
struct SDOLayerInfo;
struct SDOSpatialReferenceInfo;

namespace Bentley
    {
    struct BeXmlNode;
    }

class HRFGeoRasterCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFGeoRasterCapabilities();

    };


class HRFGeoRasterFile : public HRFRasterFile
    {
public:
    friend class HRFGeoRasterEditor;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_GeoRaster, HRFRasterFile)

    // allow to Open an image file
    HRFGeoRasterFile        (const HFCPtr<HFCURL>&          pi_rpURL,
                             HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                             uint64_t                      pi_Offset = 0);

    virtual                                 ~HRFGeoRasterFile       ();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities         () const;

    // File information
    virtual const HGF2DWorldIdentificator   GetWorldIdentificator   () const;

    // File manipulation
    virtual bool                           AddPage                 (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*            CreateResolutionEditor  (uint32_t                   pi_Page,
                                                                     unsigned short            pi_Resolution,
                                                                     HFCAccessMode              pi_AccessMode);


    virtual void                            Save();

protected:

    // Methods

    // Constructor use only to create a child
    //
    HRFGeoRasterFile        (const HFCPtr<HFCURL>&      pi_rpURL,
                             HFCAccessMode              pi_AccessMode,
                             uint64_t                  pi_Offset,
                             bool                      pi_DontOpenFile);
    virtual void                            CreateDescriptors       ();

private:
    friend struct HRFGeoRasterCreator;

    // members
    bool                         m_IsBigEndian;
    HAutoPtr<SDOGeoRasterWrapper> m_pSDOGeoRasterWrapper;
    
    bool ConnectToOracle         (WStringCR pi_ConnectionString);

    void  ReadXORA_1_0 (Bentley::BeXmlNode* pi_pNode, bool pi_Connected);
    void  ReadXORA_1_1 (Bentley::BeXmlNode* pi_pNode, bool pi_Connected);

    void  ReadObjectInfo (Bentley::BeXmlNode* pi_pObjectInfoNode, SDOObjectInfo* po_pSDOObjectInfo);
    void  ReadRasterInfo (Bentley::BeXmlNode* pi_pRasterInfoNode, SDORasterInfo* po_pSDORasterInfo);

    void  ReadSpatialReferenceInfo(Bentley::BeXmlNode* pi_pLayerInfoNode, SDOSpatialReferenceInfo* po_pSDOSpatialReferenceInfo);
    void  ReadLayerInfo           (Bentley::BeXmlNode* pi_pRasterInfoNode, SDOLayerInfo* po_pSDOLayerInfo);

    RasterFileGeocodingPtr  ExtractGeocodingInformation  (SDOSpatialReferenceInfo const& pi_rSpatialRefInfo);

    // Methods Disabled
    HRFGeoRasterFile(const HRFGeoRasterFile& pi_rObj);
    HRFGeoRasterFile& operator=(const HRFGeoRasterFile& pi_rObj);
    };

// GeoRaster Creator.
struct HRFGeoRasterCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                       IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                     uint64_t                pi_Offset = 0) const;
    virtual bool                       CanRegister() const;

    // Identification information
    virtual WString                     GetLabel() const;
    virtual WString                     GetSchemes() const;
    virtual WString                     GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();


    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>       Create(const HFCPtr<HFCURL>& pi_rpURL,
                                               HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                               uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFGeoRasterCreator)

    // Disabled methodes
    HRFGeoRasterCreator();
    };
END_IMAGEPP_NAMESPACE

