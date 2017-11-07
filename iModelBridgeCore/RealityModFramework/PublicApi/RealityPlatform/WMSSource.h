/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/WMSSource.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! Type used to contain a list of WMS settings including layers, styles, coordinate 
//! system and so on.
//! @bsiclass                                   Jean-Francois.Cote               5/2015
//=====================================================================================
struct WmsMapSettings : public RefCountedBase
    {
    public:
        //! Create WMS map request with all the required information. 
        REALITYDATAPLATFORM_EXPORT static WmsMapSettingsPtr Create(Utf8CP uri, DRange2dCR bbox, Utf8CP version, Utf8CP layers, Utf8CP csType, Utf8CP csLabel);
        REALITYDATAPLATFORM_EXPORT static WmsMapSettingsPtr CreateFromXml(Utf8CP xmlFragment);

        //! Get/Set the main URI of the server. This URI is the seed to which WMS parameters must be added to obtain a fully 
        //! qualified WMS request. Up to but excluding the query char '?'
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUri() const;
        REALITYDATAPLATFORM_EXPORT void         SetUri(Utf8CP uri);

        //! Get/Set the bounding box.
        REALITYDATAPLATFORM_EXPORT DRange2dCR   GetBBox() const;
        REALITYDATAPLATFORM_EXPORT void         SetBBox(DRange2dCP bbox);

        //! Get/Set the width of the window in pixels.
        //! The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distorsion.
        //! Sub-Resolutions will be generated from this size.
        REALITYDATAPLATFORM_EXPORT uint32_t GetMetaWidth() const;
        REALITYDATAPLATFORM_EXPORT void     SetMetaWidth(uint32_t width);

        //! Get/Set the height of the window in pixels.
        //! The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distorsion.
        //! Sub-Resolutions will be generated from this size.
        REALITYDATAPLATFORM_EXPORT uint32_t GetMetaHeight() const;
        REALITYDATAPLATFORM_EXPORT void     SetMetaHeight(uint32_t height);

        //! Get/Set the WMS protocol version supported by the WMS server.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetVersion() const;
        REALITYDATAPLATFORM_EXPORT void         SetVersion(Utf8CP version);

        //! Get/Set the list of comma-separated layers. Note that layers here are the layers names not their display name.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLayers() const;
        REALITYDATAPLATFORM_EXPORT void         SetLayers(Utf8CP layers);

        //! Get/Set the list of comma-separated styles applicable to layers. There should be an equal number of styles and layers, 
        //! each style applying to corresponding layer. If the style element is absent or empty then default style will be used for every layer.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetStyles() const;
        REALITYDATAPLATFORM_EXPORT void         SetStyles(Utf8CP styles);

        //! Get/Set the coordinate system type attribute. 
        //! CRS for version 1.3, SRS for 1.1.1 and below.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetCoordSysType() const;
        REALITYDATAPLATFORM_EXPORT void         SetCoordSysType(Utf8CP csType);

        //! Get/Set the coordinate system selected and supported by the WMS server.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetCoordSysLabel() const;
        REALITYDATAPLATFORM_EXPORT void         SetCoordSysLabel(Utf8CP csLabel);

        //! Get/Set the output format. 
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFormat() const;
        REALITYDATAPLATFORM_EXPORT void         SetFormat(Utf8CP format);

        //! Get/Set the unparsed, vendor specific parameters that will be appended to the request. This is where 
        //! non-standard WMS extensions are indicated.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetVendorSpecific() const;
        REALITYDATAPLATFORM_EXPORT void         SetVendorSpecific(Utf8CP vendorSpecific);

        //! Get/Set the transparency. 
        REALITYDATAPLATFORM_EXPORT bool IsTransparent() const;
        REALITYDATAPLATFORM_EXPORT void SetTransparency(bool isTransparent);

        //! Generate a xml fragment.
        REALITYDATAPLATFORM_EXPORT void ToXml(Utf8StringR xmlFragment) const;
    
    private:
        WmsMapSettings(Utf8CP uri, DRange2dCR bbox, Utf8CP version, Utf8CP layers, Utf8CP csType, Utf8CP csLabel);
        ~WmsMapSettings();

        Utf8String  m_uri;

        // Map window    
        DRange2d    m_boundingBox;          
        uint32_t    m_metaWidth;            
        uint32_t    m_metaHeight;

        // Mandatory GetMap parameters
        Utf8String  m_version;
        Utf8String  m_layers;               
        Utf8String  m_styles;    
        Utf8String  m_csType;         
        Utf8String  m_csLabel;           
        Utf8String  m_format;

        // Optional GetMap parameters
        Utf8String  m_vendorSpecific; 
        bool        m_transparent;        
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE