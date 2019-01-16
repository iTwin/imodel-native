/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/DgnV8/LightWeightConverter.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "converter.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

struct LightWeightLineStyleConverter;
typedef RefCountedPtr<LightWeightLineStyleConverter> LightWeightLineStyleConverterPtr;

//=================================================================================
//! The LightWeight version of the Converter does not rely on the Sync Info
//! database for mappings. The mappings should be made by the hosts application. This should be
//! used in context of a Power Product. This converter also is setup where the application using
//! it is reponsible for the creation of the element and any EC conversions. This class has
//! method to convert the V8i Graphics to a builder. This was created to support the OpenPlant
//! integration to the iModelHub. 
//! @bsiclass                                                    Vern.Francisco   11/17
//! +===============+===============+===============+===============+===============+======

struct LightWeightConverter 
    {

    typedef bpair<DgnFontType, Utf8String> T_WorkspaceFontKey;
    typedef bmap<T_WorkspaceFontKey, DgnFontPtr> T_WorkspaceFonts;
    typedef bmap<uint32_t, DgnFontPtr> T_V8EmbeddedRscFontMap;
    typedef bpair<DgnV8Api::DgnFile*, uint32_t> T_FontRemapKey;
    typedef bmap<T_FontRemapKey, DgnFontPtr> T_FontRemap;
    typedef bmap<void*, RenderMaterialId> T_MaterialRemap;
    typedef bpair<Utf8String, Utf8String> T_MaterialNameKey;
    typedef bmap<T_MaterialNameKey, RenderMaterialId> T_MaterialNameRemap;
    typedef bmap<WString, DgnTextureId> T_TextureFileNameRemap;
    typedef std::multimap<PartRangeKey, DgnGeometryPartId> RangePartIdMap;


    public:

        DGNDBSYNC_EXPORT virtual BentleyStatus ConvertElement(DgnV8EhCR v8childEh, GeometryBuilderPtr builder, DgnCategoryId targetCategoryId);
        DGNDBSYNC_EXPORT void EmbedFonts();
        DGNDBSYNC_EXPORT LightWeightConverter(DgnDbPtr dgndb, DgnV8FileP dgnV8File, DgnModelPtr model, DgnV8ModelP v8Model, Utf8String codePrefix, DgnCategoryId defaultCategoryId, DgnSubCategoryId defaultSubCategoryId, Int32 converterId);
        DGNDBSYNC_EXPORT ~LightWeightConverter();
        DGNDBSYNC_EXPORT static void Initialize();
        DGNDBSYNC_EXPORT static void ConvertTextString(TextStringPtr& clone, Bentley::TextStringCR v8Text, DgnFileR dgnFile, LightWeightConverter& converter);

        //! Return a stable pointer to a font object that can be put into the database. This implies that the converter should own the font, and hand out a pointer to it. This also implies that it can return nullptr, meaning that we could not find the original font, and to create a missing font entry. The default implementation calls _EnsureWorkspaceFontsAreLoaded to ensure m_workspaceFonts is populated, and then searches it for the font in question, returning a pointer.
        DGNDBSYNC_EXPORT DgnFont const* _ImportV8Font(DgnV8Api::DgnFont const&);

        DGNDBSYNC_EXPORT DgnFont const& _RemapV8Font(DgnV8Api::DgnFile&, uint32_t v8FontId);

        Int32  GetConverterId() { return m_converterId; }
        DefinitionModelPtr GetJobDefinitionModel();

        //! Query sync info for a v8 linestyle in the current v8 file.

        virtual DgnStyleId FindLineStyle(double& unitsScale, bool& foundStyle, Int32 v8Id);
        virtual BeSQLite::DbResult InsertLineStyle(DgnStyleId, double unitsScale, Int32 v8Id);
        virtual DgnSubCategoryId GetSubCategory(uint32_t v8levelid, SyncInfo::Level::Type ltype);

        void InitGeometryParams(Render::GeometryParams& params, DgnV8Api::ElemDisplayParams& paramsV8, DgnV8Api::ViewContext& context, bool is3d);
        bool InitPatternParams(PatternParamsR pattern, DgnV8Api::PatternParams const& patternV8, Bentley::bvector<DgnV8Api::DwgHatchDefLine> const& defLinesV8, Bentley::DPoint3d& origin, DgnV8Api::ViewContext& context);

        virtual DgnSubCategoryId GetSubCategory(uint32_t v8levelid, DgnV8ModelCR, SyncInfo::Level::Type ltype) 
            {
            return GetSubCategory(v8levelid, ltype);
            };

        void SetDefaultDgnCategory(DgnCategoryId  defaultCategoryId)
            {
            m_defaultCategoryId = defaultCategoryId;
            }

        void SetDefaultDgnSubCategory(DgnSubCategoryId  defaultSubCategoryId)
            {
            m_defaultSubCategoryId = defaultSubCategoryId;
            }

        virtual DgnStyleId _RemapLineStyle(double& unitsScale, DgnV8Api::DgnFile&, int32_t v8LineStyleId, bool required);
        void InitGeometryParams(Render::GeometryParams& params, DgnV8Api::ElemDisplayParams& paramsV8, DgnV8Api::ViewContext& context, bool is3d, DgnV8ModelCR);
        void InitLineStyle(Render::GeometryParams& params, DgnModelRefR styleModelRef, int32_t srcLineStyleNum, DgnV8Api::LineStyleParams const* v8lsParams);
        DGNDBSYNC_EXPORT static void ConvertLineStyleParams(Render::LineStyleParams& lsParams, DgnV8Api::LineStyleParams const* v8lsParams, double uorPerMeter, double componentScale, double modelLsScale);

        void SetRootModel(DgnV8ModelP r);
        DgnCode CreateCode(Utf8StringCR value) const;

        //! @name Codes
        //! @{
        void InitBusinessKeyCodeSpec();
        //! Get the ID of the CodeSpec which determines element codes based on v8 BusinessKeySpecification custom attributes.
        CodeSpecId GetBusinessKeyCodeSpec() const { return m_businessKeyCodeSpecId; }
        //! @}


        //! @name The target DgnDb
        //! @{
        bool IsDgnDbValid() const { return m_dgndb.IsValid(); }
        void SetDgnDb(DgnDbR db) { m_dgndb = &db; }
        DgnDb& GetDgnDb() const { return *m_dgndb; }
        //! @}

        //! @name  Converting Materials
        //! @{
        void SetMaterialUsed(RenderMaterialId id);
        bool GetMaterialUsed(RenderMaterialId id) const;
        RenderMaterialId GetRemappedMaterial(DgnV8Api::Material const* material);
        //! @}


    protected:

        struct MappedLineStyle
            {
            DgnStyleId      m_id;
            double          m_unitsScale;   //  may be needed to adjust LineStyleParams since they expressed in Line Style units.

            MappedLineStyle() : m_unitsScale(1) {}
            MappedLineStyle(DgnStyleId id, double unitsScale) : m_id(id), m_unitsScale(unitsScale) {}
            };


        DgnFilePtr                          m_rootFile;
        DgnV8ModelP                         m_v8Model;
        DgnModelPtr                         m_model;
        DgnCategoryId                       m_defaultCategoryId;
        DgnSubCategoryId                    m_defaultSubCategoryId;
        Int32                               m_converterId; // ID for the Converter to use.
        LightWeightLineStyleConverterPtr    m_lineStyleConverter;
        CodeSpecId                          m_businessKeyCodeSpecId;
        T_MaterialRemap                     m_materialRemap;
        T_MaterialNameRemap                 m_materialNameRemap;
        bset<RenderMaterialId>              m_materialUsed;
        mutable bool                        m_hadError = false;
        bool                                m_hasLoadedWorkspaceFonts = false;
        T_WorkspaceFonts                    m_workspaceFonts;
        T_V8EmbeddedRscFontMap              m_v8EmbeddedRscFontMap;
        bset<BeFileName>                    m_tempFontFiles;
        T_FontRemap                         m_fontRemap;
        DgnDbPtr                            m_dgndb;
        bmap<int32_t, MappedLineStyle>      m_lineStyle;
        DgnModelId          m_jobDefinitionModelId;


        BentleyStatus _CreateElementGeom(DgnCategoryId targetCategoryId, DgnV8EhCR v8eh, GeometryBuilderPtr builder);
        DgnSubCategoryId ConvertLevelToSubCategory(DgnV8Api::LevelHandle const& level, DgnV8ModelCR v8Model, DgnCategoryId catid);
        void ComputeSubCategoryAppearanceFromLevel(DgnSubCategory::Appearance& appear, DgnV8Api::LevelHandle const& level);

        //! @name Converting fonts
        //! The idea behind converting fonts from DgnV8 is to replicate the DgnV8 workspace by creating a collection of "known" fonts from lose files. As fonts are encountered in a DgnV8 file, we look for them by-type and -name in our "workspace", and make entries for them in the database. Then, as a second pass, we embed any used fonts, as dictated by the stubs in the database.
        //! This has a critical side effect: Until the second pass when fonts are embedded, if you query the database directly for a font, it will be unresolved. This is why _RemapFont returns a DgnFont object, and not an ID (though it can be used to get an ID). The DgnFont object returned by _RemapFont is owned and resolved by this converter, and is valid for direct use. Currently, all known users will use this returned font object for all intents and purposes, thus it is acceptable to do embedding as a second pass. But be forewarned, as-is, you cannot query a database for a font by-ID and use it until the embedding pass has been done.
        //! @{
        DgnV8FileR _GetFontRootV8File() { return *(m_rootFile.get()); }
        DgnModelR  _GetModel() { return *(m_model); }


        //! Fills in m_workspaceFonts with "workspace" fonts, which are searched by-type and -name when remapping from DgnV8. "Workspace" in this sense attempts to replicate the global font registry that DgnV8 maintained from the system and its workspace configuration. The default implementation will iterate all font files in _GetWorkspaceFontSearchPaths, and make all found fonts available for remapping.
        virtual void _EnsureWorkspaceFontsAreLoaded();

        //! Attempt to embed font data for all font entries in the database so that fonts can be resolved without the converter and its "workspace" being present. The default implementation takes the union of the database's font entries and any "always" embed fonts from the import configuration, and attempts to embed data by-type and -name from m_workspaceFonts.
        virtual void _EmbedFonts() ;

        //! @}
		




		

    };



    //! =================================================================================
    //! The LightWeight version of the LineStyleConverter does not rely on the Sync Info
    //! database for mappings. The mappings should be made by the hosts application. This should be
    //! used in context of a Power Product.
    //! @bsiclass
    //! +===============+===============+===============+===============+===============+======

    struct LightWeightLineStyleConverter : RefCountedBase
        {
        private:
            struct V8Location
                {
                private:
                    bool                         m_isElement;
                    SyncInfo::V8FileSyncInfoId   m_v8fileId;             //  Foreign file ID or RSC handle
                    DgnV8Api::ElementId          m_v8componentKey;       //  Element ID or RSC ID
                    uint32_t                     m_v8componentType;      //  LsResourceType or LsElementType
                public:
                    V8Location(DgnV8Api::LsCacheComponent const& component, DgnDbPtr);
                    V8Location() : m_isElement(false), m_v8componentKey(0), m_v8componentType(0) {}
                    V8Location(V8Location const& l) : m_isElement(l.m_isElement), m_v8fileId(l.m_v8fileId), m_v8componentKey(l.m_v8componentKey), m_v8componentType(l.m_v8componentType) {}
                    bool operator< (V8Location const &o) const;
                };

            bmap<V8Location, uint32_t>      m_v8ComponentToV10Id;
            bmap<Utf8String, DgnStyleId>    m_lsDefNameToIdMap;
            DgnDbPtr                        m_dgnDb;
            Utf8String                      m_codePrefix;
            DgnCategoryId                   m_defaultCategoryId;
            DgnSubCategoryId                m_defaultSubCategoryId;
            LightWeightConverter&           m_converter;
            DgnV8ModelP                     m_rootModel {};

            DgnDbR GetDgnDb() { return *m_dgnDb; }

            LineStyleStatus ConvertPointSymbol(Dgn::LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheSymbolComponent const& symbolComponent, double lsScale);
            LineStyleStatus ConvertElementLineCode(DgnStyleId& newId, uint32_t lineCode);
            LineStyleStatus ConvertLinePoint(LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCachePointComponent const& linePointComponent, double lsScale);
            LineStyleStatus ConvertCompoundComponent(LsComponentId&v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheCompoundComponent const& compoundComponent, double lsScale);
            LineStyleStatus ConvertLineCode(Dgn::LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheStrokePatternComponent const& strokePattern, double lsScale);
            LineStyleStatus ConvertRasterImageComponent(Dgn::LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsRasterImageComponent const& rasterComponent);

            LightWeightLineStyleConverter(LightWeightConverter& converter, DgnDbPtr dgnDb, Utf8String codePrefix, DgnCategoryId defaultCategory, DgnSubCategoryId defaultSubCategory) : m_converter (converter) { m_dgnDb = dgnDb; m_codePrefix = codePrefix; m_defaultCategoryId = defaultCategory; m_defaultSubCategoryId = defaultSubCategory; }

            LineStyleStatus ConvertLsComponent(Dgn::LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheComponent const& component, double lsScale);

            static void SetDescription(Dgn::V10ComponentBase*v10, DgnV8Api::LsCacheComponent const& component);
            static void SetDescription(JsonValueR json, DgnV8Api::LsCacheComponent const& component);


        public:
            DGNDBSYNC_EXPORT static RefCountedPtr<LightWeightLineStyleConverter> Create(LightWeightConverter& converter, DgnDbPtr dgnDb, Utf8String codePrefix, DgnCategoryId defaultCategory, DgnSubCategoryId defaultSubCategory);
            DGNDBSYNC_EXPORT LineStyleStatus ConvertLineStyle(DgnStyleId& newId, double& componentScale, DgnV8Api::LsDefinition*v8ls, DgnV8Api::DgnFile&v8File);
                             LineStyleStatus GetOrConvertElementLineCode(DgnStyleId& newId, uint32_t lineCode);

            LightWeightLineStyleConverter(LightWeightConverter& converter) : m_converter(converter) {};
            void SetRootModel(DgnV8ModelP r) { m_rootModel = r; }
            DgnV8ModelP GetRootModel() { return m_rootModel; }

        };

END_DGNDBSYNC_DGNV8_NAMESPACE
