/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPackage/WMSSource.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPackage/RealityPackage.h>

BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               5/2015
//=====================================================================================
struct WmsMapSettings : public RefCountedBase
    {
public:
    //! Create WMS MapRequest info with all the required information. 
    REALITYPACKAGE_EXPORT static WmsMapSettingsPtr Create(Utf8CP uri, DRange2dCR bbox, Utf8CP version, Utf8CP layers, Utf8CP csType, Utf8CP csLabel);
    REALITYPACKAGE_EXPORT static WmsMapSettingsPtr CreateFromXml(Utf8CP xmlFragment);

    //! Get/Set the server uri. 
    REALITYPACKAGE_EXPORT Utf8StringCR GetUri() const;
    REALITYPACKAGE_EXPORT void         SetUri(Utf8CP uri);

    //! Get/Set the bounding box.
    REALITYPACKAGE_EXPORT DRange2dCR   GetBBox() const;
    REALITYPACKAGE_EXPORT void         SetBBox(DRange2dCP bbox);

    //! Get/Set the width of the window in pixels.
    //! The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distorsion.
    //! Sub-Resolutions will be generated from this size.
    REALITYPACKAGE_EXPORT uint32_t GetMetaWidth() const;
    REALITYPACKAGE_EXPORT void     SetMetaWidth(uint32_t width);

    //! Get/Set the height of the window in pixels.
    //! The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distorsion.
    //! Sub-Resolutions will be generated from this size.
    REALITYPACKAGE_EXPORT uint32_t GetMetaHeight() const;
    REALITYPACKAGE_EXPORT void     SetMetaHeight(uint32_t height);

    //! Get/Set the WMS version. 
    REALITYPACKAGE_EXPORT Utf8StringCR GetVersion() const;
    REALITYPACKAGE_EXPORT void         SetVersion(Utf8CP version);

    //! Get/Set the layer list. 
    REALITYPACKAGE_EXPORT Utf8StringCR GetLayers() const;
    REALITYPACKAGE_EXPORT void         SetLayers(Utf8CP layers);

    //! Get/Set the style list. 
    REALITYPACKAGE_EXPORT Utf8StringCR GetStyles() const;
    REALITYPACKAGE_EXPORT void         SetStyles(Utf8CP styles);

    //! Get/Set the coordinate system type attribute. 
    //! CRS for version 1.3, SRS for 1.1.1 and below.
    REALITYPACKAGE_EXPORT Utf8StringCR GetCoordSysType() const;
    REALITYPACKAGE_EXPORT void         SetCoordSysType(Utf8CP csType);

    //! Get/Set the coordinate system. 
    REALITYPACKAGE_EXPORT Utf8StringCR GetCoordSysLabel() const;
    REALITYPACKAGE_EXPORT void         SetCoordSysLabel(Utf8CP csLabel);

    //! Get/Set the output format. 
    REALITYPACKAGE_EXPORT Utf8StringCR GetFormat() const;
    REALITYPACKAGE_EXPORT void         SetFormat(Utf8CP format);

    //! Get/Set the vendor specific parameters.
    //! Unparsed, vendor specific parameters that will be appended to the request.
    REALITYPACKAGE_EXPORT Utf8StringCR GetVendorSpecific() const;
    REALITYPACKAGE_EXPORT void         SetVendorSpecific(Utf8CP vendorSpecific);

    //! Get/Set the transparency. 
    REALITYPACKAGE_EXPORT bool IsTransparent() const;
    REALITYPACKAGE_EXPORT void SetTransparency(bool isTransparent);

    //! Xml fragment.
    REALITYPACKAGE_EXPORT void ToXml(Utf8StringR xmlFragment) const;
    
private:
    WmsMapSettings(Utf8CP uri, DRange2dCR bbox, Utf8CP version, Utf8CP layers, Utf8CP csType, Utf8CP csLabel);
    ~WmsMapSettings();

    Utf8String  m_uri;                  //! GetMap url. Up to but excluding the query char '?'

    // Map window
    //&&JFC Reproject bbox in cs unit.
    DRange2d    m_boundingBox;          //! Bounding box corners (minx,miny,maxx,maxy) in 'CoordinateSystem' unit (aka. cartesian or reprojected unit).
    uint32_t    m_metaWidth;            //! Width of the window in pixels. The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distortion. Sub-Resolutions will be generated from this width.
    uint32_t    m_metaHeight;           //! Height of the window in pixels. The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distortion. Sub-Resolutions will be generated from this height.

    // Mandatory GetMap parameters
    Utf8String  m_version;              //! Wms server version.
    Utf8String  m_layers;               //! Comma-separated list of one or more map layers.
    Utf8String  m_styles;               //! Comma-separated list of one rendering style per requested layer.
    Utf8String  m_csType;               //! Usually, 'SRS' for 1.1.1 and below. 'CRS' for 1.3 and above.
    Utf8String  m_csLabel;              //! Coordinate system label. ex: "EPSG:4326"
    Utf8String  m_format;               //! Output format of map. Default is 'image/png'.

    // Optional GetMap parameters
    Utf8String  m_vendorSpecific;       //! [optional] Unparsed, vendor specific parameters that will be appended to the request. 
    bool        m_transparent;          //! [optional] Background is transparent?     
    };

END_BENTLEY_REALITYPACKAGE_NAMESPACE