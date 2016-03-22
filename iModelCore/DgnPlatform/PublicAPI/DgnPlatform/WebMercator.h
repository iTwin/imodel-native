/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/WebMercator.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnViewport.h>
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/ImageUtilities.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// Obtains and displays multi-resolution tiled raster reality data that is organized
// according to the WebMercator tiling system. 
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WebMercatorModel : SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS("WebMercatorModel", SpatialModel);

public:
    struct Properties
        {
        AxisAlignedBox3d m_range;       //! The range covered by this map -- typically the project's extents -- could be the whole world
        Utf8String m_mapService;        //! Identifies the source of the tiled map data. This is a token that is supplied by the 
                                        //! subclass of WebMercatorModel and stored in the DgnDb to associate it with a map server. 
                                        //! The WebMercatorModel subclass uses this string when constructing URLs at runtime for requesting tiles.
        Utf8String m_mapType;           //! Identifies the type of map data to request and display.
        bool m_finerResolution;         //! true => download and display more and smaller tiles, if necessary, in order to get the best resolution.
                                        //! false => download and display fewer and larger tiles, resulting in sometimes slightly fuzzy resolution.

        double m_groundBias=-1.0;       //! by default, draw map 1 meter below ground level.
                                                                            
        void ToJson(Json::Value&) const;
        void FromJson(Json::Value const&);
        };

    //=======================================================================================
    //! Identifies a tile in the fixed WebMercator tiling system.
    // @bsiclass                                                    Sam.Wilson      10/2014
    //=======================================================================================
    struct TileId
    {
        uint8_t  m_zoomLevel;
        uint32_t m_column;
        uint32_t m_row;
        TileId() {}
        TileId(uint8_t zoomLevel, uint32_t col, uint32_t row){m_zoomLevel=zoomLevel; m_column=col; m_row=row;}
        bool operator<(TileId const& rhs) const;
    };

protected:
    struct TextureCache
    {
        struct Entry
        {
            static uint64_t Next();
            mutable uint64_t m_accessed;
            Render::TexturePtr  m_texture;
            RgbImageInfo m_imageInfo;
            void Accessed() const {m_accessed = Next();}
        };

    private:
        int m_maxSize = 200;
        bmap<TileId, Entry> m_map;
        void Trim();
    public:
        void Insert(TileId const& id, RgbImageInfo const& info, Render::TextureR texture);
        Entry const* Get(TileId const& id) {auto ifound = m_map.find(id); return (ifound == m_map.end()) ? nullptr : &ifound->second;}
    };

    Properties m_properties;
    mutable RealityDataCachePtr m_realityDataCache;
    mutable TextureCache m_textureCache;
        
public:
    RealityDataCache& GetRealityDataCache() const;
    TextureCache& GetTextureCache() const {return m_textureCache;}

    //! Create a new WebMercatorModel object, in preparation for loading it from the DgnDb.
    WebMercatorModel(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT void _AddTerrainGraphics(TerrainContextR) const override;
    DGNPLATFORM_EXPORT void _WriteJsonProperties(Json::Value&) const override;
    DGNPLATFORM_EXPORT void _ReadJsonProperties(Json::Value const&) override;
    AxisAlignedBox3d _QueryModelRange() const override {return m_properties.m_range;}
    double GetGroundBias() const {return m_properties.m_groundBias;}

    //! Create the URL to request the specified tile from a map service.
    //! @param[out] url The returned URL
    //! @param[out] imageInfo Expected image format
    //! @param[in] tileid  The location of the tile, according to the WebMercator tiling system
    //! @return SUCCESS if URL was computed and is valid
    virtual BentleyStatus _CreateUrl(Utf8StringR url, RgbImageInfo& imageInfo, TileId const& tileid) const {return BSIERROR;}

    virtual bool _ShouldRejectTile(TileId const& tileid, Utf8StringCR url, ByteStream const& data) const {return false;}

    //! Call this after creating a new model, to set up properties.
    void SetProperties(Properties const& props) {m_properties=props;}
};

//=======================================================================================
// A street map model
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StreetMapModel : WebMercatorModel
{
    DGNMODEL_DECLARE_MEMBERS("StreetMapModel", WebMercatorModel);

    Utf8CP _GetCopyrightMessage() const override;
    BentleyStatus _CreateUrl(Utf8StringR url, RgbImageInfo& imageInfo, TileId const&) const override;
    bool _ShouldRejectTile(TileId const& tileid, Utf8StringCR url, ByteStream const& realityData) const override;

    Utf8String CreateOsmUrl(TileId const&) const;
    Utf8String CreateMapBoxUrl(TileId const&) const;
public:
    StreetMapModel(CreateParams const& params) : T_Super(params) {}
};

namespace dgn_ModelHandler
{
    //=======================================================================================
    //! Base class for model handlers that create models derived from WebMercatorModel.
    // @bsiclass                                                    Sam.Wilson      10/2014
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE WebMercator : Spatial
    {
        MODELHANDLER_DECLARE_MEMBERS ("WebMercatorModel", WebMercatorModel, WebMercator, Spatial, DGNPLATFORM_EXPORT)
    };

    //=======================================================================================
    // A handler for models that communicate with one of the well known street map services
    // to obtain and display street maps and satellite imagery based on the WebMercator tiling system.
    // @bsiclass                                                    Sam.Wilson      10/2014
    //=======================================================================================
    struct StreetMap : WebMercator
    {
        MODELHANDLER_DECLARE_MEMBERS ("StreetMapModel", StreetMapModel, StreetMap, WebMercator, DGNPLATFORM_EXPORT)

        //! Identifies a well known street map tile service
        enum class MapService
            {
#ifndef NDEBUG
            OpenStreetMaps = 999,         //!< OpenStreetMaps (developer builds only!)
#endif
            MapBox = 0
            };

        //! The kind of map to display
        enum class MapType
            {
            Map,                    //!< Show a map
            SatelliteImage,         //!< Show a satellite image (if available)
            };

        //! Create a new street map model in the DgnDb.
        //! @param[in] db           The DgnDb
        //! @param[in] mapService   Identifies the map service that will supply the maps or imagery
        //! @param[in] mapType      Identifies the kind of map data to display
        //! @param[in] finerResolution If true, the external data model will download and display more and smaller tiles, if necessary, in order to get the best resolution.
        //!                            If false, fewer and larger tiles are obtained and displayed. That saves time but may sometimes result in slightly fuzzy resolution.
        //! @return the Id of the new external data model.
        DGNPLATFORM_EXPORT static DgnModelId CreateStreetMapModel(DgnDbR db, MapService mapService, MapType mapType, bool finerResolution);
    };
};

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
