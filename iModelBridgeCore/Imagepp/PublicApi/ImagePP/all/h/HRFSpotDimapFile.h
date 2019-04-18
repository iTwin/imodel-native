//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCAccessMode.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFGeoTiffFile.h"



BEGIN_IMAGEPP_NAMESPACE
typedef struct SpotDimapXmlHeader
    {
    Utf8String  DataFileFormat;
    Utf8String  DataFilePath;
    } SpotDimapXmlHeader;


/** ---------------------------------------------------------------------------
    General capabilities of the SpotDimap file format.
    ---------------------------------------------------------------------------
 */
class HRFSpotDimapCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFSpotDimapCapabilities();
    };


/** ---------------------------------------------------------------------------
    This class handle SpotDimap raster file operations.

    @see HRFRasterFile

    ---------------------------------------------------------------------------
 */
class HRFSpotDimapFile : public HRFGeoTiffFile
    {
public:
    //:> Class ID for this class
    HDECLARE_CLASS_ID(HRFFileId_SpotDimap, HRFGeoTiffFile)



    IMAGEPP_EXPORT                         HRFSpotDimapFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                    HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                                    uint64_t             pi_Offset = 0);



    IMAGEPP_EXPORT virtual                 ~HRFSpotDimapFile               ();

    //:> File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities        () const override;

    //:> File information
    const HGF2DWorldIdentificator
    GetWorldIdentificator  () const override;

    //:> File manipulation
    HRFResolutionEditor*
    CreateResolutionEditor (uint32_t      pi_Page,
                            uint16_t pi_Resolution,
                            HFCAccessMode pi_AccessMode) override;
protected:
    //:> Open main file (xml)
    HRFSpotDimapFile       (const HFCPtr<HFCURL>&  pi_rpURL,
                            HFCAccessMode          pi_AccessMode,
                            uint64_t              pi_Offset,
                            bool                  pi_DontOpenFile);

    //:> Open  files
    bool                         Open                  (bool pi_CreateBigTifFormat=false) override;
    bool                         Open                  (const HFCPtr<HFCURL>&  pi_rpURL) override{
        return T_Super::Open(pi_rpURL);
        }

    //:> Initialization
    void            CreateDescriptors       () override;




    HFCPtr<HFCURL>          ComposeImageryURL       ();
    bool                   ReadHeaderFromXMLFile     ();
#ifdef __HMR_DEBUG_MEMBER
    bvector<double>        m_pTiePoints;
    bool                   XMLGeoRefSimilarToGeoTiff;
    bool                   IsXMLGeoRefSimilarToGeoTiff(double* pi_TiePointsMatrix, uint16_t pi_NbVal_GeoTiePoint);
#endif





private:
    //:> Close   raster file
    void                    Initialize              ();

    // Methods disabled
    HRFSpotDimapFile(const HRFSpotDimapFile& pi_rObj);
    HRFSpotDimapFile& operator=(const HRFSpotDimapFile& pi_rObj);


    SpotDimapXmlHeader          m_Header;


    };


/** ---------------------------------------------------------------------------
    This struct handle SpotDimap raster file creations.
    ---------------------------------------------------------------------------
 */
struct HRFSpotDimapCreator : public HRFGeoTiffCreator
    {
    //:> Opens the file and verifies if it is the right type
    virtual bool                 IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                               uint64_t                pi_Offset = 0) const;

    //:> Identification information
    virtual Utf8String               GetLabel() const;
    virtual Utf8String               GetSchemes() const;
    virtual Utf8String               GetExtensions() const;


    //:> Capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    //:> Allows to open an image file READONLY
    virtual HFCPtr<HRFRasterFile> Create(const HFCPtr<HFCURL>& pi_rpURL,
                                         HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                         uint64_t             pi_Offset = 0) const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFSpotDimapCreator)

    //:> Disabled methods
    HRFSpotDimapCreator();
    };
END_IMAGEPP_NAMESPACE


