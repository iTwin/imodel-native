/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnElement.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/BeAssert.h>
#include "RepositoryManager.h"
#include <PlacementOnEarth/Placement.h>

BEGIN_BENTLEY_RENDER_NAMESPACE
struct Graphic;
DEFINE_REF_COUNTED_PTR(Graphic)
END_BENTLEY_RENDER_NAMESPACE

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler 
{
    struct Element; 
    struct InformationCarrier; 
    struct InformationContent; struct InformationRecord; struct GroupInformation; struct Subject;
    struct Document; struct Drawing; struct SectionDrawing;  
    struct DriverBundle;
    struct Definition; struct PhysicalType; struct GraphicalType2d; struct SpatialLocationType; struct TemplateRecipe2d; struct TemplateRecipe3d;
    struct InformationPartition; struct DefinitionPartition; struct DocumentPartition; struct GroupInformationPartition; struct InformationRecordPartition; struct PhysicalPartition; struct SpatialLocationPartition;
    struct Geometric2d; struct Annotation2d; struct DrawingGraphic; 
    struct Geometric3d; struct Physical; struct SpatialLocation; 
    struct Role;
};

namespace dgn_TxnTable {struct Element; struct Model;};

struct ElementAutoHandledPropertiesECInstanceAdapter;

//=======================================================================================
//! Holds Id remapping tables
//=======================================================================================
struct DgnRemapTables
{
protected:
    // *** NEEDS WORK: We may have to move these remappings into temp tables
    bmap<DgnModelId, DgnModelId> m_modelId;
    bmap<DgnGeometryPartId, DgnGeometryPartId> m_geomPartId;
    bmap<DgnElementId, DgnElementId> m_elementId;
    bmap<DgnClassId, DgnClassId> m_classId;
    bmap<CodeSpecId, CodeSpecId> m_codeSpecId;
    bmap<DgnFontId, DgnFontId> m_fontId;

    template<typename T> T Find(bmap<T,T> const& table, T sourceId) const {auto i = table.find(sourceId); return (i == table.end())? T(): i->second;}
    template<typename T> T FindElement(T sourceId) const {return T(Find<DgnElementId>(m_elementId, sourceId).GetValueUnchecked());}

public:
    DgnRemapTables& Get(DgnDbR);
    CodeSpecId Find(CodeSpecId sourceId) const {return Find<CodeSpecId>(m_codeSpecId, sourceId);}
    CodeSpecId Add(CodeSpecId sourceId, CodeSpecId targetId) {return m_codeSpecId[sourceId] = targetId;}
    DgnModelId Find(DgnModelId sourceId) const {return Find<DgnModelId>(m_modelId, sourceId);}
    DgnModelId Add(DgnModelId sourceId, DgnModelId targetId) {return m_modelId[sourceId] = targetId;}
    DgnElementId Find(DgnElementId sourceId) const {return Find<DgnElementId>(m_elementId, sourceId);}
    DgnElementId Add(DgnElementId sourceId, DgnElementId targetId) {return m_elementId[sourceId] = targetId;}
    DgnGeometryPartId Find(DgnGeometryPartId sourceId) const {return Find<DgnGeometryPartId>(m_geomPartId, sourceId);}
    DgnGeometryPartId Add(DgnGeometryPartId sourceId, DgnGeometryPartId targetId) {return m_geomPartId[sourceId] = targetId;}
    DgnCategoryId Find(DgnCategoryId sourceId) const {return FindElement<DgnCategoryId>(sourceId);}
    DgnCategoryId Add(DgnCategoryId sourceId, DgnCategoryId targetId) {return DgnCategoryId((m_elementId[sourceId] = targetId).GetValueUnchecked());}
    RenderMaterialId Find(RenderMaterialId sourceId) const {return FindElement<RenderMaterialId>(sourceId);}
    RenderMaterialId Add(RenderMaterialId sourceId, RenderMaterialId targetId) {return RenderMaterialId((m_elementId[sourceId] = targetId).GetValueUnchecked());}
    DgnTextureId Find(DgnTextureId sourceId) const {return FindElement<DgnTextureId>(sourceId);}
    DgnTextureId Add(DgnTextureId sourceId, DgnTextureId targetId) {return DgnTextureId((m_elementId[sourceId] = targetId).GetValueUnchecked());}
    DgnStyleId Find(DgnStyleId sourceId) const {return FindElement<DgnStyleId>(sourceId);}
    DgnStyleId Add(DgnStyleId sourceId, DgnStyleId targetId) {return DgnStyleId((m_elementId[sourceId] = targetId).GetValueUnchecked());}
    DgnFontId Find(DgnFontId sourceId) const {return Find<DgnFontId>(m_fontId, sourceId);}
    DgnFontId Add(DgnFontId sourceId, DgnFontId targetId) {return m_fontId[sourceId] = targetId;}
    DgnSubCategoryId Find(DgnSubCategoryId sourceId) const {return FindElement<DgnSubCategoryId>(sourceId);}
    DgnSubCategoryId Add(DgnSubCategoryId sourceId, DgnSubCategoryId targetId) {return DgnSubCategoryId((m_elementId[sourceId] = targetId).GetValueUnchecked());}
    DgnClassId Find(DgnClassId sourceId) const {return Find<DgnClassId>(m_classId, sourceId);}
    DgnClassId Add(DgnClassId sourceId, DgnClassId targetId) {return m_classId[sourceId] = targetId;}
};

//=======================================================================================
//! Context used by elements when they are cloned
//=======================================================================================
struct DgnCloneContext
{
protected:
    DgnRemapTables  m_remap;

public:
    //! Construct a DgnCloneContext object.
    DGNPLATFORM_EXPORT DgnCloneContext();
    //! Look up a copy of an element
    DgnElementId FindElementId(DgnElementId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of an element
    DgnElementId AddElementId(DgnElementId sourceId, DgnElementId targetId) {return m_remap.Add(sourceId, targetId);}
};

#if !defined (DOCUMENTATION_GENERATOR)
//=======================================================================================
// A cache of ECInstanceUpdaters
// THIS MUST NOT BE EXPORTED, AS IT DOES NOT REQUIRE THE CALLER TO SUPPLY THE ECCRUDWRITETOKEN
//=======================================================================================
struct ECInstanceUpdaterCache
    {
    private:
        bmap<DgnClassId, BeSQLite::EC::ECInstanceUpdater*> m_updaters;
    protected:
        virtual void _GetPropertiesToBind(bvector<ECN::ECPropertyCP>&, DgnDbR, ECN::ECClassCR) = 0;
    public:
        ECInstanceUpdaterCache();
        ~ECInstanceUpdaterCache();
        void Clear();
        BeSQLite::EC::ECInstanceUpdater* GetUpdater(DgnDbR, ECN::ECClassCR);
    };
#endif

//=======================================================================================
//! Helps models, elements, aspects and other data structures copy themselves between DgnDbs
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnImportContext : DgnCloneContext
{
private:
    bool            m_areCompatibleDbs;
    DPoint3d        m_xyzOffset;
    AngleInDegrees  m_yawAdj;
    DgnDbR          m_sourceDb;
    DgnDbR          m_destDb;
    bmap<LsComponentId, uint32_t> m_importedComponents;

    void ComputeGcsAdjustment();

protected:
    DGNPLATFORM_EXPORT virtual CodeSpecId _RemapCodeSpecId(CodeSpecId sourceId);
    DGNPLATFORM_EXPORT virtual DgnGeometryPartId _RemapGeometryPartId(DgnGeometryPartId sourceId);
    DGNPLATFORM_EXPORT virtual DgnCategoryId _RemapCategory(DgnCategoryId sourceId);
    DGNPLATFORM_EXPORT virtual DgnSubCategoryId _RemapSubCategory(DgnCategoryId destCategoryId, DgnSubCategoryId sourceId);
    DGNPLATFORM_EXPORT virtual DgnClassId _RemapClassId(DgnClassId sourceId);
    DGNPLATFORM_EXPORT virtual RenderMaterialId _RemapRenderMaterialId(RenderMaterialId sourceId);
    DGNPLATFORM_EXPORT virtual DgnTextureId _RemapTextureId(DgnTextureId sourceId);
    DGNPLATFORM_EXPORT virtual DgnDbStatus _RemapGeometryStreamIds(GeometryStreamR geom);
    DGNPLATFORM_EXPORT virtual DgnFontId _RemapFont(DgnFontId);
    DGNPLATFORM_EXPORT virtual DgnStyleId _RemapLineStyleId(DgnStyleId sourceId);
    DGNPLATFORM_EXPORT virtual DgnElementId _RemapAnnotationStyleId(DgnElementId sourceId);

public:
    //! Construct a DgnImportContext object.
    DGNPLATFORM_EXPORT DgnImportContext(DgnDbR source, DgnDbR dest);
    //! Destruct a DgnImportContext object.
    DGNPLATFORM_EXPORT ~DgnImportContext();

    //! @name Source and Destination Dbs
    //! @{
    DgnDbR GetSourceDb() const {return m_sourceDb;}
    DgnDbR GetDestinationDb() const {return m_destDb;}
    bool IsBetweenDbs() const {return &GetDestinationDb() != &GetSourceDb();}
    //! @}

    //! @name Id remapping
    //! @{
    //! Make sure that a CodeSpec has been imported
    CodeSpecId RemapCodeSpecId(CodeSpecId sourceId) {return _RemapCodeSpecId(sourceId);}
    //! Register a copy of a CodeSpec
    CodeSpecId AddCodeSpecId(CodeSpecId sourceId, CodeSpecId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Look up a copy of a model
    DgnModelId FindModelId(DgnModelId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a model
    DgnModelId AddModelId(DgnModelId sourceId, DgnModelId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a GeometryPart has been imported
    DgnGeometryPartId RemapGeometryPartId(DgnGeometryPartId sourceId) {return _RemapGeometryPartId(sourceId);}
    //! Look up a copy of a Category
    DgnCategoryId FindCategory(DgnCategoryId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a Category
    DgnCategoryId AddCategory(DgnCategoryId sourceId, DgnCategoryId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a Category has been imported
    DgnCategoryId RemapCategory(DgnCategoryId sourceId) {return _RemapCategory(sourceId);}
    //! Look up a copy of an subcategory
    DgnSubCategoryId FindSubCategory(DgnSubCategoryId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a SubCategory
    DgnSubCategoryId AddSubCategory(DgnSubCategoryId sourceId, DgnSubCategoryId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a SubCategory has been imported
    DgnSubCategoryId RemapSubCategory(DgnCategoryId destCategoryId, DgnSubCategoryId sourceId) {return _RemapSubCategory(destCategoryId, sourceId);}
    //! Make sure that an ECClass has been imported
    DgnClassId RemapClassId(DgnClassId sourceId) {return _RemapClassId(sourceId);}
    //! Look up a copy of a RenderMaterial
    RenderMaterialId FindRenderMaterialId(RenderMaterialId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a RenderMaterial
    RenderMaterialId AddMaterialId(RenderMaterialId sourceId, RenderMaterialId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a RenderMaterial has been imported
    RenderMaterialId RemapRenderMaterialId(RenderMaterialId sourceId) {return _RemapRenderMaterialId(sourceId);}
    //! Look up a copy of a Texture
    DgnTextureId FindTextureId(DgnTextureId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a Texture
    DgnTextureId AddTextureId(DgnTextureId sourceId, DgnTextureId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a Texture has been imported
    DgnTextureId RemapTextureId(DgnTextureId sourceId) {return _RemapTextureId(sourceId);}
    //! Look up a copy of a LineStyle
    DgnStyleId FindLineStyleId(DgnStyleId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a LineStyle
    DgnStyleId AddLineStyleId(DgnStyleId sourceId, DgnStyleId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a LineStyle has been imported
    DgnStyleId RemapLineStyleId(DgnStyleId sourceId) {return _RemapLineStyleId(sourceId);}
    //! Look up a copy of a LineStyle component
    LsComponentId FindLineStyleComponentId(LsComponentId sourceId) const;
    //! Register a copy of a LineStyle component
    void AddLineStyleComponentId(LsComponentId sourceId, LsComponentId targetId);
    //! Look up a copy of an AnnotationTextStyle
    DgnElementId FindAnnotationTextStyleId(DgnElementId sourceId) const { return m_remap.Find(sourceId); }
    //! Register a copy of an AnnotationTextStyleI
    DgnElementId AddAnnotationTextStyleId(DgnElementId sourceId, DgnElementId targetId);
    // Register a copy of an AnnotationTextStyleId
    DgnElementId RemapAnnotationStyleId(DgnElementId sourceId) { return _RemapAnnotationStyleId(sourceId); }
    //! Make sure that any ids referenced by the supplied GeometryStream have been imported
    DgnDbStatus RemapGeometryStreamIds(GeometryStreamR geom) {return _RemapGeometryStreamIds(geom);}
    //! Remap a font between databases. If it exists by-type and -name, the Id is simply remapped; if not, a deep copy is made. If a deep copy is made and the source database contained the font data, the font data is also deep copied.
    DgnFontId RemapFont(DgnFontId srcId) {return _RemapFont(srcId);}
    //! @}

    //! @name GCS coordinate system shift
    //! @{
    //! Check if the source and destination GCSs are compatible, such that elements can be copied between them.
    DgnDbStatus CheckCompatibleGCS() const {return m_areCompatibleDbs? DgnDbStatus::Success: DgnDbStatus::MismatchGcs;}
    //! When copying between different DgnDbs, X and Y coordinates may need to be offset
    DPoint3d GetOriginOffset() const {return m_xyzOffset;}
    //! When copying between different DgnDbs, the Yaw angle may need to be adjusted.
    AngleInDegrees GetYawAdjustment() const {return m_yawAdj;}
    //! @}
};

//=======================================================================================
//! The "current entry" of an ElementIterator
// @bsiclass                                                     Shaun.Sewall      11/16
//=======================================================================================
struct ElementIteratorEntry : ECSqlStatementEntry
{
    friend struct ECSqlStatementIterator<ElementIteratorEntry>;
private:
    ElementIteratorEntry(BeSQLite::EC::ECSqlStatement* statement = nullptr) : ECSqlStatementEntry(statement) {}
public:
    DGNPLATFORM_EXPORT DgnElementId GetElementId() const; //!< Get the DgnElementId of the current element
    template <class T_ElementId> T_ElementId GetId() const {return T_ElementId(GetElementId().GetValue());} //!< Get the DgnElementId of the current element
    DGNPLATFORM_EXPORT DgnClassId GetClassId() const; //!< Get the DgnClassId of the current element
    DGNPLATFORM_EXPORT BeSQLite::BeGuid GetFederationGuid() const; //!< Get the FederationGuid of the current element
    DGNPLATFORM_EXPORT DgnCode GetCode() const; //!< Get the DgnCode of the current element
    DGNPLATFORM_EXPORT Utf8CP GetCodeValue() const; //!< Get the CodeValue of the current element
    DGNPLATFORM_EXPORT DgnModelId GetModelId() const; //!< Get the DgnModelId of the current element
    DGNPLATFORM_EXPORT DgnElementId GetParentId() const; //!< Get the DgnElementId of the parent of the current element
    DGNPLATFORM_EXPORT Utf8CP GetUserLabel() const; //!< Get the user label of the current element
    DGNPLATFORM_EXPORT DateTime GetLastModifyTime() const; //!< Get the last modify time of the current element
};

//=======================================================================================
//! An iterator over a set of DgnElements, defined by a query.
// @bsiclass                                                     Shaun.Sewall      11/16
//=======================================================================================
struct ElementIterator : ECSqlStatementIterator<ElementIteratorEntry>
{
    //! Iterates all entries to build an unordered IdSet templated on DgnElementId or a subclass of DgnElementId
    template <class T_ElementId> BeSQLite::IdSet<T_ElementId> BuildIdSet()
        {
        BeSQLite::IdSet<T_ElementId> idSet;
        for (ElementIteratorEntry entry : *this)
            idSet.insert(entry.GetId<T_ElementId>());

        return idSet;
        }

    //! Iterates all entries to build an ordered bvector templated on DgnElementId or a subclass of DgnElementId
    template <class T_ElementId> bvector<T_ElementId> BuildIdList()
        {
        bvector<T_ElementId> idList;
        for (ElementIteratorEntry entry : *this)
            idList.push_back(entry.GetId<T_ElementId>());

        return idList;
        }

    //! Iterates all entries to populate an ordered bvector templated on DgnElementId or a subclass of DgnElementId
    template <class T_ElementId> void BuildIdList(bvector<T_ElementId>& idList)
        {
        for (ElementIteratorEntry entry : *this)
            idList.push_back(entry.GetId<T_ElementId>());
        }
};

//=======================================================================================
//! The "current entry" of an ElementAspectIterator
// @bsiclass                                                     Shaun.Sewall      11/16
//=======================================================================================
struct ElementAspectIteratorEntry : ECSqlStatementEntry
{
    friend struct ECSqlStatementIterator<ElementAspectIteratorEntry>;
private:
    ElementAspectIteratorEntry(BeSQLite::EC::ECSqlStatement* statement = nullptr) : ECSqlStatementEntry(statement) {}
public:
    DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceId GetECInstanceId() const; //!< Get ECInstanceId (unique instance identifier) of the current aspect
    DGNPLATFORM_EXPORT DgnClassId GetClassId() const; //!< Get the DgnClassId of the current aspect
    DGNPLATFORM_EXPORT DgnElementId GetElementId() const; //!< Get the DgnElementId of the element that owns the current aspect
};

//=======================================================================================
//! An iterator over a set of ElementAspects, defined by a query.
// @bsiclass                                                     Shaun.Sewall      07/17
//=======================================================================================
struct ElementAspectIterator : ECSqlStatementIterator<ElementAspectIteratorEntry>
{
};

//=======================================================================================
//! The basic element importer. Imports elements and their children.
// @bsiclass                                                BentleySystems
//=======================================================================================
struct ElementImporter
{
protected:
    DgnImportContext& m_context;
    bool m_copyChildren;
    bool m_copyGroups;

public:
    DGNPLATFORM_EXPORT ElementImporter(DgnImportContext&);

    DgnImportContext& GetImportContext() {return m_context;}

    //! Specify if children should be deep-copied or not. The default is yes, deep-copy children.
    void SetCopyChildren(bool b) {m_copyChildren=b;}

    //! Specify if group members should be deep-copied or not. The default is no, do not deep-copy group members.
    void SetCopyGroups(bool b) {m_copyGroups=b;}

    //! Make a persistent copy of a specified Physical element, along with all of its children.
    //! If the source element is a group, this function will optionally import all of its members (recursively). See SetCopyGroups.
    //! When importing children, if the child element's model has already been imported, this function will import the child into the copy of that model. 
    //! If the child element's model has not already been imported, then this function will import the child into its parent's model. 
    //! The same strategy is used to choose the destination model of group members.
    //! @param[out] stat        Optional. If not null, then an error code is stored here in case the copy fails.
    //! @param[in] destModel    The model where the instance is to be inserted
    //! @param[in] sourceElement The element that is to be copied
    //! @note Parent elements must be imported before child elements. The parent of the new element will 
    //!       be the element in the destination db to which the source parent has been remapped, or it will be invalid if the parent has not been remapped.
    //! @return a new element if successful
    DGNPLATFORM_EXPORT DgnElementCPtr ImportElement(DgnDbStatus* stat, DgnModelR destModel, DgnElementCR sourceElement);
};

//=======================================================================================
//! Returns all auto- or custom-handled properties on a class that are for the specified type of statements
//! @private
// @bsiclass                                                    Sam.Wilson      07/16
//=======================================================================================
struct AutoHandledPropertiesCollection
{
    ECN::ECPropertyIterable m_props;
    ECN::ECPropertyIterable::const_iterator m_end;
    ECN::ECClassCP m_customHandledProperty;
    ECN::ECClassCP m_autoHandledProperty;
    ECSqlClassParams::StatementType m_stype;
    bool m_wantCustomHandledProps;
    static bmap<ECN::ECClassCP, bvector<ECN::ECPropertyCP>> s_orphanCustomHandledProperties;

    AutoHandledPropertiesCollection(ECN::ECClassCR eclass, DgnDbR db, ECSqlClassParams::StatementType stype, bool wantCustomHandledProps);
    
    static void DetectOrphanCustomHandledProperty(DgnDbR db, ECN::ECClassCR);
    DGNPLATFORM_EXPORT static bool IsOrphanCustomHandledProperty(ECN::ECPropertyCR);     // used by imodeljs node addon

    struct Iterator : std::iterator<std::input_iterator_tag, ECN::ECPropertyCP>
        {
        private:
            friend struct AutoHandledPropertiesCollection;
            ECN::ECPropertyIterable::const_iterator m_i;
            ECSqlClassParams::StatementType m_stype;
            AutoHandledPropertiesCollection const& m_coll;
            Iterator(ECN::ECPropertyIterable::const_iterator it, AutoHandledPropertiesCollection const& coll);
            void ToNextValid();

        public:
            ECN::ECPropertyCP operator*() const {BeAssert(m_i != m_coll.m_end); return *m_i;}
            Iterator& operator++();
            bool operator!=(Iterator const& rhs) const {return !(*this == rhs);}
            bool operator==(Iterator const& rhs) const {return m_i == rhs.m_i;}
            ECSqlClassParams::StatementType GetStatementType() const {return m_stype;}
        };

    typedef Iterator const_iterator;
    const_iterator begin() const {return Iterator(m_props.begin(), *this);}
    const_iterator end() const {return Iterator(m_props.end(), *this);}
};

//=======================================================================================
//! Specifies either an invalid value or the index of an item in an array.
// @bsiclass                                                     Sam.Wilson        10/16
//=======================================================================================
struct PropertyArrayIndex
{
    bool m_hasIndex;
    uint32_t m_index;
    PropertyArrayIndex() : m_hasIndex(0) {}
    PropertyArrayIndex(uint32_t index) : m_hasIndex(true), m_index(index) {}
    PropertyArrayIndex(bool useIndex, uint32_t index) : m_hasIndex(useIndex), m_index(index) {}
    bool HasIndex() const {return m_hasIndex;}
    uint32_t GetIndex() const {return m_index;}
};

//=======================================================================================
//! Helps with access to an individual element property
// @bsiclass                                                     Sam.Wilson        10/16
//=======================================================================================
struct ElementECPropertyAccessor
{
private:
    DgnElementR m_element;
    ECSqlClassInfo* m_classInfo;
    ECN::ECClassCP m_eclass;
    ECN::ClassLayoutCP m_layout;
    bpair<ECSqlClassInfo::T_ElementPropGet,ECSqlClassInfo::T_ElementPropSet> const* m_accessors;
    uint32_t m_propIdx;
    bool m_isPropertyIndexValid;
    bool m_readOnly;

    DGNPLATFORM_EXPORT void Init(uint32_t propIdx, Utf8CP accessString);

public:
    DGNPLATFORM_EXPORT ElementECPropertyAccessor(DgnElementCR, uint32_t);
    DGNPLATFORM_EXPORT ElementECPropertyAccessor(DgnElementCR, Utf8CP accessString);
    DGNPLATFORM_EXPORT ElementECPropertyAccessor(DgnElementR, uint32_t);
    DGNPLATFORM_EXPORT ElementECPropertyAccessor(DgnElementR, Utf8CP accessString);

    bool IsValid() const {return m_isPropertyIndexValid;}

    bool IsAutoHandled() const {return nullptr == m_accessors;}

    uint32_t GetPropertyIndex() const {return m_propIdx;}

    DgnElementR GetElement() const {return m_element;}

    DGNPLATFORM_EXPORT ECN::PropertyLayoutCP GetPropertyLayout() const;

    DGNPLATFORM_EXPORT Utf8CP GetAccessString() const;

    DGNPLATFORM_EXPORT DgnDbStatus SetAutoHandledPropertyValue(ECN::ECValueCR value, PropertyArrayIndex const& arrayIndex);
    DGNPLATFORM_EXPORT DgnDbStatus GetAutoHandledPropertyValue(ECN::ECValueR value, PropertyArrayIndex const& arrayIndex) const;

    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(ECN::ECValueCR value, PropertyArrayIndex const& arrayIndex);
    DGNPLATFORM_EXPORT DgnDbStatus GetPropertyValue(ECN::ECValueR value, PropertyArrayIndex const& arrayIndex) const;
};

#define DGNELEMENT_DECLARE_MEMBERS(__ECClassName__,__superclass__) \
    private: typedef __superclass__ T_Super;\
    public: static Utf8CP MyHandlerECClassName() {return __ECClassName__;}\
    protected: Utf8CP _GetHandlerECClassName() const override {return MyHandlerECClassName();}\
               Utf8CP _GetSuperHandlerECClassName() const override {return T_Super::_GetHandlerECClassName();}

#define DGNASPECT_DECLARE_MEMBERS(__ECSchemaName__,__ECClassName__,__superclass__) \
    private:    typedef __superclass__ T_Super;\
    public:     static Utf8CP MyECSchemaName() {return __ECSchemaName__;}\
                static Utf8CP MyECClassName() {return __ECClassName__;}\
    protected:  Utf8CP _GetECSchemaName() const override {return MyECSchemaName();}\
                Utf8CP _GetECClassName() const override {return MyECClassName();}\
                Utf8CP _GetSuperECClassName() const override {return T_Super::_GetECClassName();}


/**
* @addtogroup GROUP_DgnElement DgnElement Module
* Types related to working with %DgnElements
* @see @ref PAGE_ElementOverview
* @see @ref PAGE_CustomElement
*/

/**
* @addtogroup ElementCopying DgnElement Copying and Importing
* 
* There are 3 basic reasons why you would want to make a copy of an element, and there is a function for each one:
*   1. DgnElement::Clone makes a copy of an element, suitable for inserting into the Db.
*   2. DgnElement::Import makes a copy of an element in a source Db, suitable for inserting into a different Db. It remaps any IDs stored in the element or its aspects.
*   3. DgnElement::CopyForEdit and DgnElement::MakeCopy make make a quick copy of an element, suitable for editing and then replacing in the Db.
*
* When making a copy of an element within the same DgnDb but a different model, set up an instance of DgnElement::CreateParams that specifies the target model
* and pass that when you call Clone.
*
*/

/** 
* @addtogroup ElementCopyVirtualFunctions DgnElement Copy and Importing - Virtual Functions
*
* A DgnElement subclass will normally override a few virtual functions to support @ref ElementCopying.
*
* <h2>Virtual Member Functions</h2>
* DgnElement defines several virtual functions that control copying and importing.
*   * DgnElement::_CopyFrom must copy member variables from source element. It is used in many different copying operations.
*   * DgnElement::_Clone must make a copy of an element, suitable for inserting into the DgnDb.
*   * DgnElement::_CloneForImport must make a copy of an element in a source DgnDb, suitable for inserting into a target DgnDb. 
*   * DgnElement::_RemapIds must remap any IDs stored in the element's member variables or its aspects.
* 
* If you define a new subclass of DgnElement, you may need to override one or more of these virtual methods.
*
* If subclass ...|It must override ...
* ---------------|--------------------
* Defines new member variables|DgnElement::_CopyFrom to copy them.
* Defines new member variables that stored IDs of any kind|DgnElement::_RemapIds to relocate them to the destination DgnDb.
* Stores some of its data in Aspects|DgnElement::_Clone and DgnElement::_CloneForImport, as described below.
* 
* Normally, there is no need to override _Clone, as the base class implementation will work for subclasses, as it calls _CopyFrom.
*
* If you don't use Aspects, then normally, you won't need to override _Clone and _CloneForImport.
*
* <h2>The Central role of _CopyFrom</h2>
*
* _Clone, _CloneForImport, and CopyForEdit all call _CopyFrom to do one specific part of the copying work: copying the member variables. 
* _CopyFrom must make a straight, faithful copy of the C++ element struct's member variables only. It must be quick. 
* It should not load data from the Db. 
*
* Note that the _CopyFrom method copies <em>only</em> member variables. It must not try to read from the Db. 
* The _CopyFrom method handle only custom-handled properties. It must not try to copy or modify auto-handled properties or 
* user properties. Those properties are handled by the base class.
*
* <h2>Copying and Importing Aspects</h2>
*
* A subclass of DgnElement that stores some of its data in Aspects must take care of copying and importing those Aspects. 
* Specifically, an element subclass should override _Clone and _CloneForImport. 
*   * Its _Clone method should call super and then copy its aspects. 
*   * Its _CloneForImport method should call super, then copy over its aspects, and then tell the copied Aspects to remap their IDs.
*
* @note If a DgnElement subclass overrides any of the virtual methods mentioned above, then the corresponding ECClass should also specify @ref ElementRestrictions
*
* @see ElementRestrictions
* @see @ref PAGE_ElementOverview
*/

/**
* @addtogroup ElementRestrictions DgnElement Restrictions
*
* Element restrictions specify what operations may be applied to an element when its handler is not present. 
* Restrictions are specified in the ECSchema definition of an Element subclass.
*
* <b><em>If a DgnElement subclasss needs to do validation, then the corresponding dgn.Element ECClass should specify restrictions.</em></b>
*
* As a rule of thumb, if you write a subclass of dgn_ElementHandler::Element, then you should probably restrict all actions in your Element's schema definition.
*
* See DgnElement::RestrictedAction for the permissions that may be granted.
*/

/**
* @addtogroup ElementProperties DgnElement Properties
*
* A DgnElement may have two kinds of properties: Class-defined properties and user-defined properties.
*
* <h2>User-Defined Properties</h2>
* The user or application may add properties that are not defined by the ECClass to a particular instance. 
* User-defined properties are stored together in Json format in the JsonProperties of an element. ECSQL select statements may
* query User Properties using ECSQL's JSON functions.
* 
* <h2>Class-Defined Properties</h2>
* Properties that are defined by the ECClass are defined for every instance. 
*
* Class-defined properties may be queried by name in ECSQL select statements.
* See DgnElement::_GetPropertyValue and DgnElement::_SetPropertyValue for how to get and set class-defined property values in C++.
* Note that, while all class-defined properties are defined for every instance, the actual value of a property on a particular element may be NULL, 
* if the property definition permits NULLs.
*
* When a subclass of dgn.Element defines a property, it specifies whether the handling of that property is to be done autmatically by the 
* platform or must be done using custom logic supplied by a DgnElement subclass.
*
* <h3>Auto-Handled Properties</h3>
* When you define a subclass of dgn.Element in your domain's schema and you define properties for it, you don't have to write
* any code to enable applications to work with those properties. The base class implementation of the functions that load, store, and copy properties
* will automatically detect and handle all properties defined in the schema. This is called "auto-handling" of properties, and it is the default.
* The base class implementation of DgnElement::_GetPropertyValue and DgnElement::_SetPropertyValue will provide access to all auto-handled properties.
* New or updated auto-handled properties are automatically written to the database when the element is inserted or updated.
*
* <h4>Validating Auto-Handled Properties</h4>
* The domain schema can specify some validation rules for auto-handled properties in the ECSchema, such as the IsNullable CustomAttribute.
* Beyond that, to apply custom validation rules to auto-handled properties, a domain must define an element subclass that overrides 
* _OnInsert and _OnUpdate methods to check property values. In this case, the ECSchema should <em>also</em> specify @ref ElementRestrictions.
*
* <h3>Custom Properties</h3>
* In some cases, a subclass of DgnElement may want to map a property to a C++ member variable or must provide a custom API for a property.
* That is often necessary for binary data. In such cases, the subclass can take over the job of loading and storing that one property.
* This is called "custom-handling" a property. To opt into custom handling, the property definition in the schema must include the @a CustomHandledProperty
* CustomAttribute. The subclass of DgnElement must then override DgnElement::_BindWriteParams and DgnElement::_ReadSelectParams to load and store
* the custom-handled properties. The subclass must also override DgnElement::_GetPropertyValue and DgnElement::_SetPropertyValue to provide name-based get/set support for its custom-handled properties.
* Finally, the subclass must override DgnElement::_CopyFrom and possibly other virtual methods to support copying and importing of its custom-handled properties.
* An element subclass that defines custom-handled properties <em>must</em> specify @ref ElementRestrictions.
* @note A class that has custom-handled properties must override DgnElement::_GetPropertyValue and DgnElement::_SetPropertyValue to provide access to those properties, even if it also define special custom access methods for them.
* 
* @see @ref ElementRestrictions
* @see @ref PAGE_ElementOverview
*/

//=======================================================================================
//! An instance of a DgnElement in memory. 
//!
//! For details on writing an element handler and a DgnElement subclass, see
//! - @ref PAGE_CustomElement
//! - @ref ElementProperties
//! - @ref ElementRestrictions
//! - @ref ElementCopyVirtualFunctions
//!
//! @ingroup GROUP_DgnElement
// @bsiclass                                                     KeithBentley    10/13
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnElement : RefCountedBase, NonCopyableClass
{
public:
    friend struct DgnElements;
    friend struct DgnModel;
    friend struct IModelJs;
    friend struct ElemIdTree;
    friend struct dgn_ElementHandler::Element;
    friend struct dgn_TxnTable::Element;
    friend struct MultiAspect;
    friend struct GeometrySource;
    friend struct ElementECPropertyAccessor;
    friend struct ElementAutoHandledPropertiesECInstanceAdapter;

    //! Parameters for creating a new DgnElement
    struct CreateParams
    {
    public:
        DgnDbR              m_dgndb;
        DgnModelId          m_modelId;
        DgnClassId          m_classId;
        DgnCode             m_code;
        BeSQLite::BeGuid    m_federationGuid;
        Utf8String          m_userLabel;
        DgnElementId        m_id;
        DgnElementId        m_parentId;
        DgnClassId          m_parentRelClassId;
        bool                m_isLoadingElement;

        DGNPLATFORM_EXPORT CreateParams(DgnDbR, JsonValueCR);
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCodeCR code=DgnCode(), Utf8CP label=nullptr, DgnElementId parentId=DgnElementId(), DgnClassId parentRelClassId=DgnClassId(), BeSQLite::BeGuidCR federationGuid=BeSQLite::BeGuid(), bool isLoadingElement = false)
            : m_dgndb(db), m_modelId(modelId), m_classId(classId), m_code(code), m_parentId(parentId), m_parentRelClassId(parentRelClassId), m_federationGuid(federationGuid), m_isLoadingElement(isLoadingElement) {SetUserLabel(label);}

        DGNPLATFORM_EXPORT void RelocateToDestinationDb(DgnImportContext&);
        void SetCode(DgnCode code) {m_code = code;}                 //!< Set the DgnCode for elements created with this CreateParams
        void SetIsLoadingElement(bool value) { m_isLoadingElement = value; }  //!< Set the m_isLoadingElement flag for elements that are being loaded 
        void SetUserLabel(Utf8CP label) {m_userLabel.AssignOrClear(label);} //!< Set the Label for elements created with this CreateParams
        void SetElementId(DgnElementId id) {m_id = id;}             //!< @private
        void SetModelId(DgnModelId modelId) {m_modelId = modelId;}  //!< @private
        void SetParentId(DgnElementId parent, DgnClassId parentRelClassId) {m_parentId=parent; m_parentRelClassId=parentRelClassId;}  //!< Set the ParentId for elements created with this CreateParams
        void SetFederationGuid(BeSQLite::BeGuidCR federationGuid) {m_federationGuid = federationGuid;} //!< Set the FederationGuid for the DgnElement created with this CreateParams
        bool IsValid() const {return m_modelId.IsValid() && m_classId.IsValid();}
    };

    //! Property filter to be use when comparing elements
    struct ComparePropertyFilter
    {
        enum Ignore 
        {
            None      = 0, 
            WriteOnly = 0x02,  //! Ignore properties such as LastMod
            ElementId = 0x10,  //! Ignore ElementIds
        };

        Ignore m_ignore;
        bset<Utf8String> m_ignoreList;

        ComparePropertyFilter(Ignore ignore, bset<Utf8String> const& list = bset<Utf8String>()) : m_ignore(ignore), m_ignoreList(list) {}
        ComparePropertyFilter(bset<Utf8String> const& list) : m_ignore(Ignore::None), m_ignoreList(list) {}

        virtual bool _ExcludeElementId() const {return 0 != (Ignore::ElementId & m_ignore);}
        DGNPLATFORM_EXPORT virtual bool _ExcludeProperty(ECN::ECPropertyValueCR) const;
    };

    //! Property filter to be used when setting properties
    struct SetPropertyFilter
    {
        enum Ignore 
        {
            None          = 0, 
            Bootstrapping = 0x01,  //! Don't set properties that are specified in DgnElement::CreateParams, plus ElementId
            WriteOnly     = 0x02,  //! Don't set properties such as LastMod
            Null          = 0x08,  //! Don't set the property if the supplied value is null
            ElementId     = 0x10,  //! Don't set ElementId
            WriteOnlyNullBootstrapping = WriteOnly|Null|Bootstrapping|ElementId,
        };

        Ignore m_ignore;
        bool m_ignoreErrors;
        bset<Utf8String> m_ignoreList;

        SetPropertyFilter(Ignore ignore = None, bool ignoreErrors = false, bset<Utf8String> const& ignoreProps = bset<Utf8String>()) : m_ignore(ignore), m_ignoreErrors(ignoreErrors), m_ignoreList(ignoreProps) {}
        SetPropertyFilter(bset<Utf8String> const& ignore)  : m_ignore(Ignore::None), m_ignoreErrors(false), m_ignoreList(ignore) {}

        DGNPLATFORM_EXPORT static bool IsBootStrappingProperty(Utf8StringCR);

        virtual bool _ExcludeElementId() const {return 0 != (ElementId & m_ignore);}
        DGNPLATFORM_EXPORT virtual bool _ExcludeProperty(ECN::ECPropertyValueCR) const;
        virtual bool _IgnoreErrors() const {return m_ignoreErrors;}
    };

    //! Information about an ECNavigationProperty
    struct NavigationPropertyInfo
    {
    private:
        BeInt64Id m_id;
        DgnClassId m_relClassId;

    public:
        explicit NavigationPropertyInfo(BeInt64Id id=BeInt64Id(), DgnClassId relClassId=DgnClassId()) : m_id(id), m_relClassId(relClassId) {}
        DgnClassId GetRelClassId() const {return m_relClassId;}
        template <class TBeInt64Id> TBeInt64Id GetId() const {return TBeInt64Id(m_id.GetValueUnchecked());}
    };

    //! Identifies actions which may be restricted for elements created by a handler for a missing subclass of DgnElement.
    struct RestrictedAction : DgnDomain::Handler::RestrictedAction
    {
        DEFINE_T_SUPER(DgnDomain::Handler::RestrictedAction);

        static const uint64_t Clone = T_Super::NextAvailable; //!< Create a copy of element. "Clone"
        static const uint64_t SetParent = Clone << 1; //!< Change the parent element. "SetParent"
        static const uint64_t InsertChild = SetParent << 1; //!< Insert an element with this element as its parent. "InsertChild"
        static const uint64_t UpdateChild = InsertChild << 1; //!< Modify a child of this element. "UpdateChild"
        static const uint64_t DeleteChild = UpdateChild << 1; //!< Delete a child of this element. "DeleteChild"
        static const uint64_t SetCode = DeleteChild << 1; //!< Change this element's code. "SetCode"
        static const uint64_t Move = SetCode << 1; //!< Rotate and/or translate. "Move"
        static const uint64_t SetCategory = Move << 1; //!< Change element category. "SetCategory"
        static const uint64_t SetGeometry = SetCategory << 1; //!< Change element geometry. "SetGeometry"

        static const uint64_t Reserved_1 = SetGeometry << 1; //!< Reserved for future use 
        static const uint64_t Reserved_2 = Reserved_1 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_3 = Reserved_2 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_4 = Reserved_3 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_5 = Reserved_4 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_6 = Reserved_5 << 1; //!< Reserved for future use 

        static const uint64_t NextAvailable = Reserved_6 << 1; //!< Subclasses can add new actions beginning with this value

        DGNPLATFORM_EXPORT static uint64_t Parse(Utf8CP name); //!< Parse action name from ClassHasHandler custom attribute
    };

    //! Application data attached to a DgnElement. Create a subclass of this to store non-persistent information on a DgnElement.
    struct EXPORT_VTABLE_ATTRIBUTE AppData : RefCountedBase
    {
        //! A unique identifier for this type of AppData. Use a static instance of this class to identify your AppData.
        struct Key : NonCopyableClass {};

        virtual DgnDbStatus _OnInsert(DgnElementR el) {return DgnDbStatus::Success;}
        virtual DgnDbStatus _OnUpdate(DgnElementR el, DgnElementCR original){return DgnDbStatus::Success;}
        virtual DgnDbStatus _OnDelete(DgnElementCR el) {return DgnDbStatus::Success;}

        enum class DropMe : bool {No=false, Yes=true};

        //! Called after the element was Inserted.
        //! @param[in]  el the new persistent DgnElement that was Inserted
        //! @return DropMe::Yes to drop this appData, DropMe::No to leave it attached to the DgnElement.
        //! @note el will not be the writable element onto which this AppData was attached. It will be the new persistent copy of that element.
        //! If you wish for your AppData to reside on the new element, call el.AddAppData(key,this) inside this method.
        virtual DropMe _OnInserted(DgnElementCR el){return DropMe::No;}

        //! Called after the element was Updated.
        //! @param[in] modified the modified DgnElement
        //! @param[in] original the original DgnElement
        //! @param[in] isOriginal If true, this AppData is on the original element, else it's on the modified element.
        //! @return DropMe::Yes to drop this appData, DropMe::No to leave it attached to the DgnElement.
        //! @note This method is called for @b all AppData on both the original and the modified DgnElements.
        virtual DropMe _OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal) {return isOriginal? DropMe::Yes: DropMe::No;}

        //! Called after a change set representing an update to the element was applied to the database
        //! @param[in] original the original DgnElement (after applying the change)
        //! @param[in] modified the modified DgnElement (before applying the change)
        //! @return DropMe::Yes to drop this appData, DropMe::No to leave it attached to the DgnElement.
        //! @note This method is called for @b all AppData on both the original and the modified DgnElements.
        virtual DropMe _OnAppliedUpdate(DgnElementCR original, DgnElementCR modified) {return DropMe::Yes;}

        //! Called after the element was Deleted.
        //! @param[in]  el the DgnElement that was deleted
        //! @return DropMe::Yes to drop this appData, DropMe::No to leave it attached to the DgnElement.
        virtual DropMe _OnDeleted(DgnElementCR el) {return DropMe::Yes;}
    };

    using AppDataPtr = RefCountedPtr<AppData>;

    //! Holds changes to a dgn.ElementAspect in memory and writes out the changes when the host DgnElement is inserted or updated.
    //! All aspects are actually subclasses of either dgn.ElementUniqueAspect or dgn.ElementMultiAspect.
    //! A domain that defines a subclass of one of these ECClasses in the schema should normally also define a subclass of one of the
    //! subclasses of DgnElement::Aspect to manage transactions.
    //! A domain will normally subclass one of the following more specific subclasses:
    //!     * DgnElement::UniqueAspect when the domain defines a subclass of dgn.ElementUniqueAspect for aspects that must be 1:1 with the host element.
    //!     * DgnElement::MultiAspect when the domain defines a subclass of dgn.ElementMultiAspect for cases where multiple instances of the class can be associated with a given element.
    //! The domain must also define and register a subclass of ElementAspectHandler to load instances of its aspects.
    struct EXPORT_VTABLE_ATTRIBUTE Aspect : AppData
    {
    private:
        DGNPLATFORM_EXPORT DropMe _OnInserted(DgnElementCR el) override final;
        DGNPLATFORM_EXPORT DropMe _OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal) override final;
        friend struct MultiAspectMux;

    protected:
        BeSQLite::EC::ECInstanceId m_instanceId;

        enum class ChangeType{None, Write, Delete};
        ChangeType m_changeType;

        DgnDbStatus InsertThis(DgnElementCR el);
        Utf8String  GetFullEcSqlClassName() {return Utf8String(_GetECSchemaName()).append(".").append(_GetECClassName());}
        Utf8String  GetFullEcSqlKeyClassName() {return Utf8String(_GetKeyECSchemaName()).append(".").append(_GetKeyECClassName());}

        DGNPLATFORM_EXPORT Aspect();

        //! The subclass must implement this method to return the name of the schema that defines the aspect.
        virtual Utf8CP _GetECSchemaName() const = 0;

        //! The subclass must implement this method to return the name of the class that defines the aspect.
        virtual Utf8CP _GetECClassName() const {return BIS_CLASS_ElementAspect;}

        //! The subclass must implement this method to return the name of the schema that defines the key for the aspect.
        virtual Utf8CP _GetKeyECSchemaName() const {return _GetECSchemaName();}

        //! The subclass must implement this method to return the name of the class that defines the key for the aspect.
        virtual Utf8CP _GetKeyECClassName() const {return _GetECClassName();}

        //! The subclass must implement this method to return the name of the superclass
        virtual Utf8CP _GetSuperECClassName() const {return nullptr;}

        //! The subclass must implement this method to report an existing instance on the host element that this instance will replace.
        virtual BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) = 0;

        //! The subclass must override this method to insert an empty instance into the Db and associate it with the host element.
        //! @param el   The host element
        //! @param writeToken The token for updating element-related data
        //! @note The caller will call _UpdateProperties immediately after calling this method.
        //! @note use DgnDb::GetNonSelectPreparedECSqlStatement to prepare an insert statement, and pass @a writeToken as the second argument
        virtual DgnDbStatus _InsertInstance(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) = 0;

        //! The subclass must override this method to delete an existing instance in the Db, plus any ECRelationship that associates it with the host element.
        //! @param el   The host element
        //! @param writeToken The token for updating element-related data
        //! @note use DgnDb::GetNonSelectPreparedECSqlStatement to prepare a delete statement, and pass @a writeToken as the second argument
        virtual DgnDbStatus _DeleteInstance(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) = 0;

        //! The subclass must implement this method to update the instance properties.
        //! @param el   The host element
        //! @param writeToken The token for updating element-related data
        //! @note use DgnDb::GetNonSelectPreparedECSqlStatement to prepare an update statement, and pass @a writeToken as the second argument
        virtual DgnDbStatus _UpdateProperties(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) = 0;

        //! The subclass must implement this method to load properties from the Db.
        //! @param el   The host element
        virtual DgnDbStatus _LoadProperties(DgnElementCR el) = 0;

        //! The subclass must implement this method to get the value of a property by name from this aspect
        virtual DgnDbStatus _GetPropertyValue(ECN::ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const = 0;
        //! The subclass must implement this method to set the value of a property by name for this aspect
        virtual DgnDbStatus _SetPropertyValue(Utf8CP propertyName, ECN::ECValueCR value, PropertyArrayIndex const& arrayIndex) = 0;

    public:
        //! Get the Id of this aspect
        BeSQLite::EC::ECInstanceId GetAspectInstanceId() const {return m_instanceId;}

        Utf8CP GetECClassName() const {return _GetECClassName();}
        Utf8CP GetKeyECClassName() const {return _GetKeyECClassName();}
        Utf8CP GetSuperECClassName() const {return _GetSuperECClassName();}

        //! Prepare to delete this aspect.
        //! @note The aspect will not actually be deleted in the Db until you call DgnElements::Update on the aspect's host element.
        void Delete() {m_changeType = ChangeType::Delete;}

        //! Get the Id of the ECClass for this aspect
        DGNPLATFORM_EXPORT DgnClassId GetECClassId(DgnDbR) const;

        //! Get the ECClass for this aspect
        DGNPLATFORM_EXPORT ECN::ECClassCP GetECClass(DgnDbR) const;

        DGNPLATFORM_EXPORT ECN::ECClassCP GetKeyECClass(DgnDbR) const;
        DGNPLATFORM_EXPORT ECN::ECClassId GetKeyECClassId(DgnDbR) const;

        //! Get the value of a property by name from this aspect
        DgnDbStatus GetPropertyValue(ECN::ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex()) const {return _GetPropertyValue(value, propertyName, arrayIndex);}
        //! Set the value of a property by name for this aspect
        DgnDbStatus SetPropertyValue(Utf8CP propertyName, ECN::ECValueCR value, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex()) {return _SetPropertyValue(propertyName, value, arrayIndex);}

        //! The aspect should make a copy of itself.
        DGNPLATFORM_EXPORT virtual RefCountedPtr<DgnElement::Aspect> _CloneForImport(DgnElementCR sourceEl, DgnImportContext& importer) const;

        //! The subclass should override this method if it has <em>custom-handled properties</em> that contain IDs that must be remapped when it is copied (perhaps between DgnDbs)
        virtual DgnDbStatus _RemapIds(DgnElementCR el, DgnImportContext& context) {return DgnDbStatus::Success;}
    };

    //! Represents an ElementAspect subclass for the case where the host Element can have multiple instances of the subclass.
    //! Use ECSql to query existing instances and their properties. Use GetAspectP or GetP to buffer changes to a particular instance.
    //! <p>A subclass of MultiAspect must override the following methods:
    //!     * _GetECSchemaName
    //!     * _GetECClassName
    //!     * _UpdateProperties
    //!     * _LoadProperties
    //! @see UniqueAspect
    //! @note If you override _UpdateProperties, use DgnDb::GetNonSelectPreparedECSqlStatement to prepare an update statement, and pass @a writeToken as the second argument
    //! (Note: This is not stored directly as AppData, but is held by an AppData that aggregates instances for this class.)
    //! @note A domain that defines a subclass of MultiAspect may also define a subclass of dgn_AspectHandler to load it.
    struct EXPORT_VTABLE_ATTRIBUTE MultiAspect : Aspect
    {
        DEFINE_T_SUPER(Aspect)
    protected:
        DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) override final;
        DGNPLATFORM_EXPORT DgnDbStatus _DeleteInstance(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const*) override final;
        DGNPLATFORM_EXPORT DgnDbStatus _InsertInstance(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const*) override final;

    public:
        //! Create a new, uninitialized MultiAspect of the specified ECClass
        //! @see DgnElement::Aspect::SetPropertyValue
        DGNPLATFORM_EXPORT static RefCountedPtr<MultiAspect> CreateAspect(DgnDbR, ECN::ECClassCR);

        //! Load the specified instance
        //! @param el   The host element
        //! @param ecclass The class of ElementAspect to load
        //! @param ecinstanceid The Id of the ElementAspect to load
        //! @note Call this method only if you intend to modify the aspect.
        DGNPLATFORM_EXPORT static MultiAspect* GetAspectP(DgnElementR el, ECN::ECClassCR ecclass, BeSQLite::EC::ECInstanceId ecinstanceid);

        template<typename T> static T* GetP(DgnElementR el, ECN::ECClassCR cls, BeSQLite::EC::ECInstanceId id) {return dynamic_cast<T*>(GetAspectP(el,cls,id));}

        //! Get read-only access to the Aspect for the specified element
        //! @param el   The host element
        //! @param ecclass The class of ElementAspect to load
        //! @param ecinstanceid The Id of the ElementAspect to load
        //! @return The currently cached Aspect object, or nullptr if the element has no such aspect or if DeleteAspect was called.
        //! @see GetP, GetAspectP for read-write access
        DGNPLATFORM_EXPORT static MultiAspect const* GetAspect(DgnElementCR el, ECN::ECClassCR ecclass, BeSQLite::EC::ECInstanceId ecinstanceid);

        template<typename T> static T const* Get(DgnElementCR el, ECN::ECClassCR cls, BeSQLite::EC::ECInstanceId ecinstanceid) {return dynamic_cast<T const*>(GetAspect(el,cls,ecinstanceid));}

        //! Prepare to insert an aspect for the specified element
        //! @param el The host element
        //! @param aspect The new aspect to be adopted by the host.
        //! @note \a el will add a reference to \a aspect and will hold onto it.
        //! @note The aspect will not actually be inserted into the Db until you call DgnElements::Insert or DgnElements::Update on \a el
        DGNPLATFORM_EXPORT static void AddAspect(DgnElementR el, MultiAspect& aspect);
    };

    //! Represents a multiaspect that has no handler of its own.
    struct EXPORT_VTABLE_ATTRIBUTE GenericMultiAspect : MultiAspect
        {
        DEFINE_T_SUPER(MultiAspect)
        friend struct MultiAspect;
     protected:
        ECN::IECInstancePtr m_instance;
        Utf8String m_ecclassName;
        Utf8String m_ecschemaName;

        Utf8CP _GetECSchemaName() const override {return m_ecschemaName.c_str();}
        Utf8CP _GetECClassName() const override {return m_ecclassName.c_str();}
        Utf8CP _GetSuperECClassName() const override {return T_Super::_GetECClassName();}
        DGNPLATFORM_EXPORT DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
        DGNPLATFORM_EXPORT DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const*) override;
        DGNPLATFORM_EXPORT DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, PropertyArrayIndex const&) const override;
        DGNPLATFORM_EXPORT DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, PropertyArrayIndex const&) override;

        //! Use this constructor when you want to load a multiaspect
        GenericMultiAspect(ECN::ECClassCR cls, BeSQLite::EC::ECInstanceId id);

        //! Use this constructor when you want to add to the host element or update an existing multiaspect.
        //! @param inst An instance that holds the properties
        //! @param id Optional. If valid, the ID of the particular multiaspect that should be updated. If not valid, then this multiaspect will be added.
        GenericMultiAspect(ECN::IECInstanceR inst, BeSQLite::EC::ECInstanceId id);

     public:
        //! Schedule a generic multi aspect to be inserted or updated on the specified element.
        //! @param el   The host element
        //! @param properties The instance that holds the properties of the aspect that are to be written
        //! @return non-zero error status if the specified aspect cannot be added
        DGNPLATFORM_EXPORT static DgnDbStatus AddAspect(DgnElementR el, ECN::IECInstanceR properties);

        //! Prepare to update an aspect for the specified element
        //! @param el The host element
        //! @param properties  holds the properties that are to be set on the aspect
        //! @param id  The ID of the particular multiaspect that should be updated
        //! @note The aspect will not actually be updated in the Db until you call DgnElements::Update on \a el
        //! @return non-zero error status if the specified aspect is not found or cannot be set
        DGNPLATFORM_EXPORT static DgnDbStatus SetAspect(DgnElementR el, ECN::IECInstanceR properties, BeSQLite::EC::ECInstanceId id);

        //! Get the specified type of generic multi aspect, if any, from an element, with the intention of modifying the aspect's properties.
        //! @note Call Update on the host element after modifying the properties of the instance. 
        //! @note Do not free the returned instance!
        //! @note Call this method only if you intend to modify the aspect.
        //! @param el   The host element
        //! @param ecclass The type of aspect to look for
        //! @param id  The ID of the particular multiaspect that should be loaded
        //! @return the properties of the aspect or nullptr if no such aspect is found.
        DGNPLATFORM_EXPORT static ECN::IECInstanceP GetAspectP(DgnElementR el, ECN::ECClassCR ecclass, BeSQLite::EC::ECInstanceId id);

        //! Get the specified type of generic multi aspect, if any, from an element, with the intention of looking at the aspect's properties <em>but not modifying them</em>.
        //! @note Do not free the returned instance!
        //! @note Call this method only if you <em>do not</em> intend to modify the aspect.
        //! @param el   The host element
        //! @param ecclass The type of aspect to look for
        //! @param id  The ID of the particular multiaspect that should be loaded
        //! @return the properties of the aspect or nullptr if no such aspect is found.
        DGNPLATFORM_EXPORT static ECN::IECInstanceCP GetAspect(DgnElementCR el, ECN::ECClassCR ecclass, BeSQLite::EC::ECInstanceId id);
        };

    //! Represents an ElementAspect subclass in the case where the host Element can have 0 or 1 instance of the subclass. The aspect's Id is the same as the element's Id,
    //! and the aspect class must be stored in its own table (TablePerClass).
    //! A subclass of UniqueAspect must override the following methods:
    //!     * _GetECSchemaName
    //!     * _GetECClassName
    //!     * _UpdateProperties
    //!     * _LoadProperties
    //! @see MultiAspect
    //! @note A domain that defines a subclass of UniqueAspect must also define a subclass of ElementAspectHandler to load it.
    struct EXPORT_VTABLE_ATTRIBUTE UniqueAspect : Aspect
    {
        DEFINE_T_SUPER(Aspect)
    protected:
        static Key& GetKey(ECN::ECClassCR cls) {return *(Key*)&cls;}
        Key& GetKey(DgnDbR db) {return GetKey(*GetKeyECClass(db));}
        static UniqueAspect* Find(DgnElementCR, ECN::ECClassCR);
        static RefCountedPtr<DgnElement::UniqueAspect> Load0(DgnElementCR, DgnClassId); // Loads *but does not call AddAppData*
        static UniqueAspect* Load(DgnElementCR, DgnClassId);
        DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) override;
        static void SetAspect0(DgnElementCR el, UniqueAspect& aspect);
        DGNPLATFORM_EXPORT DgnDbStatus _DeleteInstance(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const*) override;
        DGNPLATFORM_EXPORT DgnDbStatus _InsertInstance(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const*) override final;
        DgnDbStatus QueryActualClass(ECN::ECClassId& classId, Dgn::DgnElementCR el, Utf8CP schemaName, Utf8CP className);

    public:
        //! The reason why GenerateGeometricPrimitive is being called
        enum class GenerateReason
        {
            Insert,         //!< The Element is being inserted into the Db
            Update,         //!< Some aspect of the Element's content has changed.
            TempDraw,       //!< A tool wants to draw the Element temporarily (the Element may not be persistent)
            BulkInsert,     //!< An application is creating a large number of Elements 
            Other           //!< An unspecified reason
        };

        //! Create a new, uninitialized UniqueAspect of the specified ECClass
        //! @see DgnElement::Aspect::SetPropertyValue
        DGNPLATFORM_EXPORT static RefCountedPtr<UniqueAspect> CreateAspect(DgnDbR, ECN::ECClassCR);

        //! Prepare to insert or update an Aspect for the specified element
        //! @param el   The host element
        //! @param aspect The new aspect to be adopted by the host.
        //! @note \a el will add a reference to \a aspect and will hold onto it.
        //! @note The aspect will not actually be inserted into the Db until you call DgnElements::Insert or DgnElements::Update on \a el
        DGNPLATFORM_EXPORT static void SetAspect(DgnElementR el, UniqueAspect& aspect);

        //! Get read-write access to the Aspect for the specified element
        //! @param el   The host element
        //! @param ecclass The class of ElementAspect to load
        //! @return The currently cached Aspect object, or nullptr if the element has no such aspect or if DeleteAspect was called.
        //! @note call this method \em only if you plan to \em modify the aspect
        //! @see Get, GetAspect for read-only access
        //! @note The aspect will not actually be updated in the Db until you call DgnElements::Update on \a el
        DGNPLATFORM_EXPORT static UniqueAspect* GetAspectP(DgnElementR el, ECN::ECClassCR ecclass);

        template<typename T> static T* GetP(DgnElementR el, ECN::ECClassCR cls) {return dynamic_cast<T*>(GetAspectP(el,cls));}

        //! Get read-only access to the Aspect for the specified element
        //! @param el   The host element
        //! @param ecclass The class of ElementAspect to load
        //! @return The currently cached Aspect object, or nullptr if the element has no such aspect or if DeleteAspect was called.
        //! @see GetP, GetAspectP for read-write access
        DGNPLATFORM_EXPORT static UniqueAspect const* GetAspect(DgnElementCR el, ECN::ECClassCR ecclass);

        template<typename T> static T const* Get(DgnElementCR el, ECN::ECClassCR cls) {return dynamic_cast<T const*>(GetAspect(el,cls));}
    };

    //! holds the properties of an aspect in memory in the case where the aspect does not have its own handler
    struct EXPORT_VTABLE_ATTRIBUTE GenericUniqueAspect : UniqueAspect
        {
        DEFINE_T_SUPER(UniqueAspect)
        friend struct UniqueAspect;

     protected:
        ECN::IECInstancePtr m_instance;
        Utf8String m_ecclassName;
        Utf8String m_ecschemaName;
        Utf8String m_key_ecclassName;
        Utf8String m_key_ecschemaName;

        Utf8CP _GetECSchemaName() const override {return m_ecschemaName.c_str();}
        Utf8CP _GetKeyECSchemaName() const override {return m_key_ecschemaName.c_str();}
        Utf8CP _GetECClassName() const override {return m_ecclassName.c_str();}
        Utf8CP _GetKeyECClassName() const override {return m_key_ecclassName.c_str();}
        Utf8CP _GetSuperECClassName() const override {return T_Super::_GetECClassName();}
        void SetKeyClass(ECN::ECClassCP keyClass);
        DgnDbStatus QueryAndSetActualClass(DgnElementCR);
        DGNPLATFORM_EXPORT DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
        DGNPLATFORM_EXPORT DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const*) override;
        DGNPLATFORM_EXPORT DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, PropertyArrayIndex const&) const override;
        DGNPLATFORM_EXPORT DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, PropertyArrayIndex const&) override;
        GenericUniqueAspect(ECN::ECClassCR cls) : m_ecclassName(cls.GetName()), m_ecschemaName(cls.GetSchema().GetName()) { m_key_ecclassName = m_ecclassName; m_key_ecschemaName = m_ecschemaName; }
        GenericUniqueAspect(ECN::IECInstanceR inst) : m_instance(&inst),  m_ecclassName(inst.GetClass().GetName()), m_ecschemaName(inst.GetClass().GetSchema().GetName()) {}

     public:

        //! Schedule a generic unique aspect to be inserted or updated on the specified element.
        //! @param el   The host element
        //! @param instance The instance that holds the properties of the aspect that are to be written
        //! @param keyClass Optional. To support polymorphism, specify a base class as the keyClass when setting an aspect that is an instance of some class derived from it. Then, you will 
        //! be able to pass the base class to GetAspect and get the stored aspect, regardless of what particular derived class it happens to be.
        //! @return non-zero error status if the specified aspect cannot be set
        DGNPLATFORM_EXPORT static DgnDbStatus SetAspect(DgnElementR el, ECN::IECInstanceR instance, ECN::ECClassCP keyClass = nullptr);

        //! Get the specified type of generic unique aspect, if any, from an element.
        //! @param el   The host element
        //! @param ecclass The type of aspect to look for
        //! @return the properties of the aspect or nullptr if no such aspect is found.
        DGNPLATFORM_EXPORT static ECN::IECInstanceCP GetAspect(DgnElementCR el, ECN::ECClassCR ecclass);

        //! Get the specified type of generic unique aspect, if any, from an element, with the intention of modifying the aspect's properties.
        //! @note Call Update on the host element after modifying the properties of the instance. 
        //! @note Do not free the returned instance!
        //! @param el   The host element
        //! @param ecclass The type of aspect to look for
        //! @return the properties of the aspect or nullptr if no such aspect is found or cannot be modified.
        DGNPLATFORM_EXPORT static ECN::IECInstanceP GetAspectP(DgnElementR el, ECN::ECClassCR ecclass);
        };

    struct RelatedElement 
        {
        DgnElementId m_id;
        DgnClassId m_relClassId;

        RelatedElement(DgnElementId id=DgnElementId(), DgnClassId relClassId=DgnClassId()) : m_id(id), m_relClassId(relClassId) {}
        bool IsValid() const {return m_id.IsValid();}
        DGNPLATFORM_EXPORT Json::Value ToJson(DgnDbR db) const;
        DGNPLATFORM_EXPORT void FromJson(DgnDbR, JsonValueCR val);
        };

private:
    mutable bmap<AppData::Key const*, RefCountedPtr<AppData>, std::less<AppData::Key const*>, 8> m_appData;

    BeMutex& GetElementsMutex() const;
    template<class T> void CallAppData(T const& caller) const;
    DGNPLATFORM_EXPORT void AddAppDataInternal(AppData::Key const& key, AppData* data) const;
    DGNPLATFORM_EXPORT AppData* FindAppDataInternal(AppData::Key const& key) const;

    Utf8String ToJsonPropString() const;
    BE_JSON_NAME(UserProps)
    ECN::AdHocJsonValueR GetUserPropsR() {return (ECN::AdHocJsonValueR) m_jsonProperties[json_UserProps()];}

protected:
    //! @private
    struct Flags
    {
        uint32_t m_persistent:1;
        uint32_t m_preassignedId:1;
        uint32_t m_propState:2; // See PropState
        Flags() {memset(this, 0, sizeof(*this));}
    };

    enum PropState // must fit in 2 bits
    {
        Unknown = 0,
        NotFound = 1,
        InBuffer = 2,
        Dirty = 3       // (implies InBuffer)
    };

    mutable Flags m_flags;
    mutable uint32_t m_ecPropertyDataSize;
    mutable Byte* m_ecPropertyData;
    DgnDbR m_dgndb;
    DgnElementId m_elementId;
    RelatedElement m_parent;
    DgnModelId m_modelId;
    DgnClassId m_classId;
    DgnCode m_code;
    BeSQLite::BeGuid m_federationGuid;
    Utf8String m_userLabel;
    ECN::AdHocJsonValue m_jsonProperties;
    ECN::StructInstanceVector* m_structInstances;

    virtual Utf8CP _GetHandlerECClassName() const {return MyHandlerECClassName();} //!< @private
    virtual Utf8CP _GetSuperHandlerECClassName() const {return nullptr;} //!< @private

    void SetPersistent(bool val) const {m_flags.m_persistent = val;} //!< @private
    void InvalidateElementId() {m_elementId = DgnElementId();} //!< @private
    void InvalidateCode() {m_code = DgnCode();} //!< @private
    
    //! A utility function to set up CreateParams from the properties of the specified instance. The input properties must include Model, CodeAuthority, CodeNamespace, and CodeValue.
    //! The value of CodeNamespace may be the empty string. If CodeNamespace is the empty string, then the value of CodeValue may be null. CodeValue may not be the empty string.
    //! The class is taken from the class of the instance.
    //! If the instance has an ECInstanceId, then DgnElementId is taken from that.
    //! @param db The BIM that will contain the new element
    //! @param instance The properties that will be used to create the new element
    //! @param initError if not null, a non-zero error status is returned here if input properties are invalid. Possible errors include:
    //! * DgnDbStatus::BadModel in case the Model property is not missing or invalid, or
    //! * DgnDbStatus::MissingId if CodeAuthority is missing or invalid or if CodeNamespace or CodeValue is missing.
    //! @return a CreateParams object or an invalid object in case of errors.
    DGNPLATFORM_EXPORT static CreateParams InitCreateParamsFromECInstance(DgnDbR db, ECN::IECInstanceCR instance, DgnDbStatus* initError);

    //! Invokes _CopyFrom() in the context of _Clone() or _CloneForImport(), preserving this element's code as specified by the CreateParams supplied to those methods.
    void CopyForCloneFrom(DgnElementCR src);

    DGNPLATFORM_EXPORT virtual ~DgnElement();

    //! Invoked when loading an element from the database, to allow subclasses to extract their custom-handled property values
    //! from the SELECT statement. The parameters are those which are marked in the schema with the CustomHandledProperty CustomAttribute.
    //! @param[in] statement The SELECT statement which selected the data from the database
    //! @param[in] selectParams The custom-handled properties selected by the SELECT statement. Use this to obtain an index into the statement.
    //! @return DgnDbStatus::Success if the data was loaded successfully, else an error status.
    //! @note If you override this method, you @em must first call T_Super::_ReadSelectParams, forwarding its status.
    //! You should then extract your subclass custom-handled properties from the supplied ECSqlStatement, using
    //! selectParams.GetParameterIndex() to look up the index of each parameter within the statement.
    //! @see ElementProperties
    virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParamsCR selectParams) {return DgnDbStatus::Success;}

    //! Convert this DgnElement to a Json::Value.
    //! @note If you override this method, you @em must call T_Super::_ToJson()
    DGNPLATFORM_EXPORT virtual void _ToJson(JsonValueR out, JsonValueCR opts) const;

    //! Update this DgnElement from a Json::Value.
    //! @note If you override this method, you @em must call T_Super::_FromJson()
    DGNPLATFORM_EXPORT virtual void _FromJson(JsonValueR props);

    //! Override this method if your element needs to load additional data from the database when it is loaded (for example,
    //! look up related data in another table).
    //! @note If you override this method, you @em must call T_Super::_LoadFromDb() first, forwarding its status
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LoadFromDb();

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert();

    //! Called whenever the JsonProperties of this element are loaded. You can override this method if you have internal state derived from JsonProperties.
    //! @note If you override this method, you @em must call T_Super::_OnLoadedJsonProperties() 
    virtual void _OnLoadedJsonProperties() {}

    //! Called before the JsonProperties of this element are saved as a Json string. 
    //! You can override this method to store internal state into JsonProperties before they are saved.
    //! @note If you override this method, you @em must call T_Super::_OnSaveJsonProperties() 
    virtual void _OnSaveJsonProperties() {}

     //! argument for _BindWriteParams
    enum class ForInsert : bool {No=false, Yes=true};

    //! Called to bind the element's custom-handled property values to the ECSqlStatement when inserting
    //! a new element. The parameters to bind are the ones which are marked in the schema with the CustomHandledProperty CustomAttribute.
    //! @param[in] statement A statement that has been prepared for either Insert or Update of your class' CustomHandledProperties
    //! @param[in] forInsert Indicates whether the statement is an insert or update statement
    //! @note If you override this method, you should bind your subclass custom-handled properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with each custom-handled property's name.
    //! Then you @em must call T_Super::_BindWriteParams,
    //! @see ElementProperties
    DGNPLATFORM_EXPORT virtual void _BindWriteParams(BeSQLite::EC::ECSqlStatement& statement, ForInsert forInsert);

    //! Override this method if your element needs to do additional Inserts into the database (for example,
    //! insert a relationship between the element and something else).
    //! @note If you override this method, you @em must call T_Super::_InsertInDb() first.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _InsertInDb();

    //! Called after a DgnElement was successfully inserted into the database.
    //! @note If you override this method, you @em must call T_Super::_OnInserted.
    DGNPLATFORM_EXPORT virtual void _OnInserted(DgnElementP copiedFrom) const;

    //! Called after a DgnElement was successfully imported into the database.
    //! @note If you override this method, you @em must call T_Super::_OnImported.
    virtual void _OnImported(DgnElementCR original, DgnImportContext& importer) const {}
    
    //! Called after a change representing addition of a DgnElement was applied to the database
    //! @note If you override this method, you @em must call T_Super::_OnAppliedAdd.
    DGNPLATFORM_EXPORT virtual void _OnAppliedAdd() const;

    //! Called when this element is about to be replace its original element in the DgnDb.
    //! @param [in] original the original state of this element.
    //! Subclasses may override this method to control whether their instances are updated.
    //! @return DgnDbStatus::Success to allow the update, otherwise the update will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnUpdate, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR original);

    //! Called to update a DgnElement in the DgnDb with new values. Override to update subclass custom-handled properties.
    //! @note This method is called from DgnElements::Update, on the persistent element, after its values have been
    //! copied from the modified version. If the update fails, the original data will be copied back into this DgnElement. Only
    //! override this method if your element needs to do additional work when updating the element, such as updating
    //! a relationship.
    //! @note If you override this method, you @em must call T_Super::_UpdateInDb, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateInDb();

    //! Called on the replacement element, after a DgnElement was successfully updated, but before the data is 
    //! copied into the original element and before its parent is notified.
    //! The replacement element will be in its post-updated state and the original element is supplied.
    //! @note If you override this method, you @em must call T_Super::_OnUpdated.
    DGNPLATFORM_EXPORT virtual void _OnUpdated(DgnElementCR original) const;

    //! Called after a DgnElement was successfully updated from a replacement element and it now holds the data from the replacement.
    //! @note If you override this method, you @em must call T_Super::_OnUpdateFinished.
    virtual void _OnUpdateFinished() const {}

    //! Called after a change set representing an update to this DgnElement was applied to the database. In the case of an undo, the element will be in its original (pre-change, post-undo) state.
    //! @note If you override this method, you @em must call T_Super::_OnAppliedUpdate.
    DGNPLATFORM_EXPORT virtual void _OnAppliedUpdate(DgnElementCR changed) const;

    //! Called when an element is about to be deleted from the DgnDb.
    //! Subclasses may override this method to control when/if their instances are deleted.
    //! @return DgnDbStatus::Success to allow the delete, otherwise the delete will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnDelete, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDelete() const;

    //! Called to delete a DgnElement from the DgnDb. Override to do any additional processing on delete.
    //! @note If you override this method, you @em must call T_Super::_DeleteInDb, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _DeleteInDb() const;

    //! Called after a DgnElement was successfully deleted. Note that the element will not be marked as persistent when this is called.
    //! @note If you override this method, you @em must call T_Super::_OnDeleted.
    DGNPLATFORM_EXPORT virtual void _OnDeleted() const;

    //! Called after a change representing delete of a DgnElement was applied to the database
    //! @note If you override this method, you @em must call T_Super::_OnAppliedDelete.
    DGNPLATFORM_EXPORT virtual void _OnAppliedDelete() const;

    //! Called when a new element is to be inserted into a DgnDb with this element as its parent.
    //! Subclasses may override this method to control which other elements may become children.
    //! @param[in] child the new element that will become a child of this element.
    //! @return DgnDbStatus::Success to allow the child insert, otherwise it will fail with the returned status.
    //! @note implementers should not presume that returning DgnDbStatus::Success means that the element will become a child element,
    //! since the insert may fail for other reasons. Instead, rely on _OnChildInserted for that purpose.
    //! @note If you override this method, you @em must call T_Super::_OnChildInsert, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnChildInsert(DgnElementCR child) const;

    //! Called when an element that has this element as its parent is about to be updated in the DgnDb.
    //! Subclasses may override this method to control modifications to its children.
    //! @param [in] original element in its original state
    //! @param [in] replacement the child element in its modified state
    //! @return DgnDbStatus::Success to allow the child update, otherwise it will fail with the returned status.
    //! @note implementers should not presume that returning DgnDbStatus::Success means that the element was updated,
    //! since the update may fail for other reasons. Instead, rely on _OnChildUpdated for that purpose.
    //! @note If you override this method, you @em must call T_Super::_OnChildUpdate, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnChildUpdate(DgnElementCR original, DgnElementCR replacement) const;

    //! Called when an child element of this element is about to be deleted from the DgnDb.
    //! Subclasses may override this method to block deletion of their children.
    //! @param[in] child that will be deleted.
    //! @return DgnDbStatus::Success to allow the child deletion, otherwise it will fail with the returned status.
    //! @note implementers should not presume that returning DgnDbStatus::Success means that the element was deleted,
    //! since the delete may fail for other reasons. Instead, rely on _OnChildDeleted for that purpose.
    //! @note If you override this method, you @em must call T_Super::_OnChildDelete, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnChildDelete(DgnElementCR child) const;

    //! Called when an existing element is about to be added as a child to this parent.
    //! Subclasses may override this method to block children from being added.
    //! @param[in] child that will be added to this parent.
    //! @return DgnDbStatus::Success to allow the child addition, otherwise it will fail with the returned status.
    //! @note implementers should not presume that returning DgnDbStatus::Success means that the element was added,
    //! since the add may fail for other reasons. Instead, rely on _OnChildAdded for that purpose.
    //! @note If you override this method, you @em must call T_Super::_OnChildAdd, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnChildAdd(DgnElementCR child) const;

    //! Called when an existing element is about to be dropped as a child from this parent.
    //! Subclasses may override this method to block children from being dropped.
    //! @param[in] child that will be dropped from this parent.
    //! @return DgnDbStatus::Success to allow the child removal, otherwise it will fail with the returned status.
    //! @note implementers should not presume that returning DgnDbStatus::Success means that the element was dropped,
    //! since the drop may fail for other reasons. Instead, rely on _OnChildDropped for that purpose.
    //! @note If you override this method, you @em must call T_Super::_OnChildDrop, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnChildDrop(DgnElementCR child) const;

    //! Called after a new element was inserted with this element as its parent.
    //! @note If you override this method, you @em must call T_Super::_OnChildInserted.
    virtual void _OnChildInserted(DgnElementCR child) const {}

    //! Called after an element, with this element as its parent, was successfully updated.
    //! @note if the parent of an element is changed, this method will @em not be paired with a call to _OnChildUpdate
    //! @note If you override this method, you @em must call T_Super::_OnChildUpdated.
    virtual void _OnChildUpdated(DgnElementCR child) const {}

    //! Called after an element, with this element as its parent, was successfully deleted.
    //! @note If you override this method, you @em must call T_Super::_OnChildDeleted.
    virtual void _OnChildDeleted(DgnElementCR child) const {}

    //! Called after an existing element was successfully added to this parent.
    //! @note If you override this method, you @em must call T_Super::_OnChildAdded.
    virtual void _OnChildAdded(DgnElementCR child) const {}

    //! Called after an existing element was successfully dropped from this parent.
    //! @note If you override this method, you @em must call T_Super::_OnChildDropped.
    virtual void _OnChildDropped(DgnElementCR child) const {}

    //! Called when a child element of this element is about to be imported into another DgnDb or model
    //! Subclasses may override this method to block control import of their children.
    //! @param[in] child The original element which is being imported
    //! @param[in] destModel The model into which the child is being imported
    //! @param[in] importer Enables the element to copy the resources that it needs (if copying between DgnDbs) and to remap any references that it holds to things outside itself to the copies of those things.
    //! @note If you override this method, you @em must call T_Super::_OnChildImport, forwarding its status.
    virtual DgnDbStatus _OnChildImport(DgnElementCR child, DgnModelR destModel, DgnImportContext& importer) const {return DgnDbStatus::Success;}

    //! Called after an element, with this element as its parent, was successfully imported
    //! @param[in] original The original element which was cloned for import, which is @em not necessarily a child of this element.
    //! @param[in] imported The clone which was imported, which is a child of this element.
    //! @param[in] importer Enables the element to copy the resources that it needs (if copying between DgnDbs) and to remap any references that it holds to things outside itself to the copies of those things.
    //! @note If you override this method, you @em must call T_Super::_OnChildImported.
    virtual void _OnChildImported(DgnElementCR original, DgnElementCR imported, DgnImportContext& importer) const {}

    //! Called when this element is being <i>modeled</i> by a new DgnModel.
    //! Subclasses may override this method to control which DgnModel types are valid to model this element.
    //! @param[in] model the new DgnModel
    //! @return DgnDbStatus::Success to allow the DgnModel insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnSubModelInsert, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnSubModelInsert(DgnModelCR model) const;

    //! Called after this element has been <i>modeled</i> by a new DgnModel.
    //! @note If you override this method, you @em must call T_Super::_OnSubModelInserted.
    virtual void _OnSubModelInserted(DgnModelCR model) const {}

    //! Called when a delete of a DgnModel modeling this element is in progress. Subclasses may override this method to block the deletion.
    //! @param[in] model the DgnModel being deleted
    //! @return DgnDbStatus::Success to allow the DgnModel deletion, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnSubModelDelete, forwarding its status.
    virtual DgnDbStatus _OnSubModelDelete(DgnModelCR model) const {return DgnDbStatus::Success;}

    //! Called after a delete of a DgnModel modeling this element has completed.
    //! @note If you override this method, you @em must call T_Super::_OnSubModelDeleted.
    virtual void _OnSubModelDeleted(DgnModelCR model) const {}

    //! Get the size, in bytes, used by this DgnElement. This is used by the element memory management routines to gauge the "weight" of
    //! each element, so it is not necessary for the value to be 100% accurate.
    //! @note Subclasses of DgnElement that add any member variables should override this method using this template:
    //! uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() + (sizeof(*this) - sizeof(T_Super)) + myAllocedSize;}
    //! where "myAllocedSize" is the number of bytes allocated for this element, held through member variable pointers.
    virtual uint32_t _GetMemSize() const {return sizeof(*this) + m_ecPropertyDataSize;}

    //! Virtual writeable deep copy method.
    //! @remarks If no CreateParams are supplied, a new DgnCode will be generated for the cloned element - it will \em not be copied from this element's DgnCode.
    DGNPLATFORM_EXPORT DgnElementPtr virtual _Clone(DgnDbStatus* stat=nullptr, DgnElement::CreateParams const* params=nullptr) const;

    //! Virtual assignment method. If your subclass has member variables, it @b must override this method and copy those values from @a source.
    //! @param[in] source The element from which to copy
    //! @note If you override this method, you @b must call T_Super::_CopyFrom, forwarding its status (that is, only return DgnDbStatus::Success if both your
    //! implementation and your superclass succeed.)
    //! @note Implementers should be aware that your element starts in a valid state. Be careful to free existing state before overwriting it. Also note that
    //! @a source is not necessarily the same type as this DgnElement. See notes at CopyFrom.
    //! @note If this element's data holds any IDs, it must also override _RemapIds. Also see _AdjustPlacementForImport
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source);

    //! Make a (near) duplicate of yourself in memory, in preparation for copying from another element that <em>may be</em> in a different DgnDb.
    //! This base class implementation calls _CopyFrom and then _RemapIds and _AdjustPlacementForImport
    //! @note Do not do any of the following:
    //!     * Do not call Insert and do not attempt to remap your own ElementId. The caller will do those things.
    //!     * Do not deep-copy child elements. The caller must do that (or not).
    //!     * Do not copy ECRelationships or deep-copy related elements. The caller must do that (or not).
    //! @param[out] stat Optional status to describe failures, a valid DgnElementPtr will only be returned if successful.
    //! @param[in] destModel The destination model (which must be in the importer's destination Db).
    //! @param[in] importer Enables the element to copy the resources that it needs (if copying between DgnDbs) and to remap any references that it holds to things outside itself to the copies of those things.
    //! @return In-memory copy of the element
    DGNPLATFORM_EXPORT DgnElementPtr virtual _CloneForImport(DgnDbStatus* stat, DgnModelR destModel, DgnImportContext& importer) const;

    //! Remap any IDs that might refer to elements or resources in the source DgnDb.
    //! @param[in] importer Specifies source and destination DgnDbs and knows how to remap IDs
    DGNPLATFORM_EXPORT virtual void _RemapIds(DgnImportContext& importer);

    //! Apply X,Y offset and Yaw angle adjustment when importing from one DgnDb to another, in the case where source and destination GCSs are compatible but have the Cartesian coordinate system
    //! located at different geo locations and/or have different Azimuth angles.
    //! @param[in] importer Specifies source and destination DgnDbs and knows how to remap IDs
    virtual void _AdjustPlacementForImport(DgnImportContext const& importer) {}

    //! Get the display label (for use in the GUI) for this DgnElement.
    //! The default implementation returns the label if set or the code if the label is not set.
    //! Override to generate the display label in a different way.
    virtual Utf8String _GetDisplayLabel() const {return HasUserLabel() ? m_userLabel : GetCode().GetValue().GetUtf8();}

    //! Change the parent (owner) of this DgnElement. The default implementation sets the parent without doing any checking.
    //! @param[in] parentId The DgnElementId of the new parent element.
    //! @param[in] parentRelClassId The DgnClassId of the ElementOwnsChildElements subclass that relates this element to its parent element.
    //! @return DgnDbStatus::Success if the parentId was changed, error status otherwise.
    //! Override to validate the parent/child relationship and return a value other than DgnDbStatus::Success to reject proposed new parent.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetParentId(DgnElementId parentId, DgnClassId parentRelClassId);

    //! Disclose any locks which must be acquired and/or codes which must be reserved to perform the specified operation on this element.
    //! @param[in] request Request to populate
    //! @param[in] opcode The operation to be performed
    //! @param[in] original If DbOpcode::Update, the persistent state of this element; otherwise, nullptr.
    //! @return RepositoryStatus::Success, or an error code if for example a required lock or code is known to be unavailable without querying the repository manager.
    //! @note If you override this function you @b must call T_Super::_PopulateRequest(), forwarding its status.
    DGNPLATFORM_EXPORT virtual RepositoryStatus _PopulateRequest(IBriefcaseManager::Request& request, BeSQLite::DbOpcode opcode, DgnElementCP original) const;

    //! Provide a description of this element to display in the "info balloon" that appears when the element is under the cursor.
    //! @param[in] delimiter Put this string to break lines of the description.
    //! @return The information to display in the info balloon.
    //! @note If you override this method, you may decide whether to call your superclass' implementation or not (it is not required).
    //! The default implementation shows display label, category and model.
    DGNPLATFORM_EXPORT virtual Utf8String _GetInfoString(Utf8CP delimiter) const;

    virtual bool _SupportsCodeSpec(CodeSpecCR) const {return true;}
    DGNPLATFORM_EXPORT virtual DgnCode _GenerateDefaultCode() const;
    virtual GeometrySourceCP _ToGeometrySource() const {return nullptr;}
    virtual AnnotationElement2dCP _ToAnnotationElement2d() const {return nullptr;}
    virtual DrawingGraphicCP _ToDrawingGraphic() const {return nullptr;}
    virtual RoleElementCP _ToRoleElement() const {return nullptr;}
    virtual InformationContentElementCP _ToInformationContentElement() const {return nullptr;}
    virtual DefinitionElementCP _ToDefinitionElement() const {return nullptr;}
    virtual DocumentCP _ToDocumentElement() const {return nullptr;}
    virtual IElementGroupCP _ToIElementGroup() const {return nullptr;}
    virtual DgnGeometryPartCP _ToGeometryPart() const {return nullptr;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _InsertPropertyArrayItems(uint32_t propertyIndex, uint32_t index, uint32_t size);
    DGNPLATFORM_EXPORT virtual DgnDbStatus _AddPropertyArrayItems(uint32_t propertyIndex, uint32_t size);
    DGNPLATFORM_EXPORT virtual DgnDbStatus _RemovePropertyArrayItem(uint32_t propertyIndex, uint32_t index);
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ClearPropertyArray(uint32_t propertyIndex);
    virtual DgnDbStatus _SetPropertyValue(ElementECPropertyAccessor& accessor, ECN::ECValueCR value, PropertyArrayIndex const& arrayIndex) {return accessor.SetPropertyValue(value, arrayIndex);}
    virtual DgnDbStatus _GetPropertyValue(ECN::ECValueR value, ElementECPropertyAccessor& accessor, PropertyArrayIndex const& arrayIndex) const {return accessor.GetPropertyValue(value, arrayIndex);}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetPropertyValues(ECN::IECInstanceCR, SetPropertyFilter const& filter);
    DGNPLATFORM_EXPORT virtual bool _Equals(DgnElementCR rhs, ComparePropertyFilter const&) const;
    //! Test if the value of the specified property on this element is equivalent to the value of the same property on the other element
    //! @param expected The property to be compared and its expected value
    //! @param other    The other element
    DGNPLATFORM_EXPORT virtual bool _EqualProperty(ECN::ECPropertyValueCR expected, DgnElementCR other) const;

    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, BeSQLite::EC::ECInstanceId, DgnClassId relClassId);

    DGNPLATFORM_EXPORT virtual void _Dump(Utf8StringR str, ComparePropertyFilter const&) const;

    void RemapAutoHandledNavigationproperties(DgnImportContext&);

    //! Construct a DgnElement from its params
    DGNPLATFORM_EXPORT explicit DgnElement(CreateParams const& params);

    void ClearAllAppData(); //!< @private

    //! Generate the CreateParams to use for Import
    //! @param destModel Specifies the model into which the element is being cloned
    //! @param importer Specifies source and destination DgnDbs and knows how to remap IDs
    //! @return CreateParams initialized with the element's current data
    //! @remarks The m_id fields are \em not set, as it is never correct for two elements to have the same Id. The m_parentId field is not set,
    //! as it is not clear if the copy should be a child of the same parent as the original. The caller can set this if appropriate.
    //! The m_code field is copied \em only when cloning between two different DgnDbs, as it is never correct for two elements to have the same code.
    CreateParams GetCreateParamsForImport(DgnModelR destModel, DgnImportContext& importer) const;

public:
    BE_JSON_NAME(id)
    BE_JSON_NAME(classFullName)
    BE_JSON_NAME(bisBaseClass)
    BE_JSON_NAME(model)
    BE_JSON_NAME(code)
    BE_JSON_NAME(parent)
    BE_JSON_NAME(federationGuid)
    BE_JSON_NAME(userLabel)
    BE_JSON_NAME(jsonProperties)

    static void AddBisClassName(JsonValueR val, ECN::ECClassCP ecClass); //!< @private
    static Utf8CP MyHandlerECClassName() {return BIS_CLASS_Element;}                //!< @private
    Utf8CP GetHandlerECClassName() const {return _GetHandlerECClassName();}             //!< @private
    Utf8CP GetSuperHandlerECClassName() const {return _GetSuperHandlerECClassName();}   //!< @private

    bool IsCustomHandledProperty(Utf8CP) const;
    bool IsCustomHandledProperty(ECN::ECPropertyCR) const;
    Utf8String GetInfoString(Utf8CP delimiter) const {return _GetInfoString(delimiter);}

    //! @name Dynamic casting to subclasses of DgnElement 
    //! @{
    GeometrySourceCP ToGeometrySource() const {return _ToGeometrySource();} //!< more efficient substitute for dynamic_cast<GeometrySourceCP>(el)
    DGNPLATFORM_EXPORT GeometrySource2dCP ToGeometrySource2d() const;
    DGNPLATFORM_EXPORT GeometrySource3dCP ToGeometrySource3d() const;

    DgnGeometryPartCP ToGeometryPart() const {return _ToGeometryPart();}                //!< more efficient substitute for dynamic_cast<DgnGeometryPartCP>(el)
    RoleElementCP ToRoleElement() const {return _ToRoleElement();}                      //!< more efficient substitute for dynamic_cast<RoleElementCP>(el)
    InformationContentElementCP ToInformationContentElement() const {return _ToInformationContentElement();} //!< more efficient substitute for dynamic_cast<InformationContentElementCP>(el)
    DefinitionElementCP ToDefinitionElement() const {return _ToDefinitionElement();}    //!< more efficient substitute for dynamic_cast<DefinitionElementCP>(el)
    DocumentCP ToDocumentElement() const {return _ToDocumentElement();}                 //!< more efficient substitute for dynamic_cast<DocumentCP>(el)
    AnnotationElement2dCP ToAnnotationElement2d() const {return _ToAnnotationElement2d();} //!< more efficient substitute for dynamic_cast<AnnotationElement2dCP>(el)
    DrawingGraphicCP ToDrawingGraphic() const {return _ToDrawingGraphic();}             //!< more efficient substitute for dynamic_cast<DrawingGraphicCP>(el)
    IElementGroupCP ToIElementGroup() const {return _ToIElementGroup();}                //!< more efficient substitute for dynamic_cast<IElementGroup>(el)
    
    GeometrySourceP ToGeometrySourceP() {return const_cast<GeometrySourceP>(_ToGeometrySource());} //!< more efficient substitute for dynamic_cast<GeometrySourceP>(el)
    GeometrySource2dP ToGeometrySource2dP() {return const_cast<GeometrySource2dP>(ToGeometrySource2d());} //!< more efficient substitute for dynamic_cast<GeometrySource2dP>(el)
    GeometrySource3dP ToGeometrySource3dP() {return const_cast<GeometrySource3dP>(ToGeometrySource3d());} //!< more efficient substitute for dynamic_cast<GeometrySource3dP>(el)

    DgnGeometryPartP ToGeometryPartP() {return const_cast<DgnGeometryPartP>(_ToGeometryPart());} //!< more efficient substitute for dynamic_cast<DgnGeometryPartCP>(el)
    RoleElementP ToRoleElementP() {return const_cast<RoleElementP>(_ToRoleElement());} //!< more efficient substitute for dynamic_cast<RoleElementP>(el)
    InformationContentElementP ToInformationContentElementP() {return const_cast<InformationContentElementP>(_ToInformationContentElement());} //!< more efficient substitute for dynamic_cast<InformationContentElementP>(el)
    DefinitionElementP ToDefinitionElementP() {return const_cast<DefinitionElementP>(_ToDefinitionElement());}  //!< more efficient substitute for dynamic_cast<DefinitionElementP>(el)
    DocumentP ToDocumentElementP() {return const_cast<DocumentP>(_ToDocumentElement());}  //!< more efficient substitute for dynamic_cast<DocumentP>(el)
    AnnotationElement2dP ToAnnotationElement2dP() {return const_cast<AnnotationElement2dP>(_ToAnnotationElement2d());} //!< more efficient substitute for dynamic_cast<AnnotationElement2dP>(el)
    DrawingGraphicP ToDrawingGraphicP() {return const_cast<DrawingGraphicP>(_ToDrawingGraphic());} //!< more efficient substitute for dynamic_cast<DrawingGraphicP>(el)
    //! @}

    bool IsGeometricElement() const {return nullptr != ToGeometrySource();}         //!< Determine whether this element is a GeometricElement or not
    bool IsRoleElement() const {return nullptr != ToRoleElement();}                 //!< Determine whether this element is a RoleElement or not
    bool IsInformationContentElement() const {return nullptr != ToInformationContentElement();}   //!< Determine whether this element is an InformationContentElement or not
    bool IsDefinitionElement() const {return nullptr != ToDefinitionElement();}     //!< Determine whether this element is a DefinitionElement or not
    bool IsDocumentElement() const {return nullptr != ToDocumentElement();}         //!< Determine whether this element is a Document element or not
    bool IsAnnotationElement2d() const {return nullptr != ToAnnotationElement2d();} //!< Determine whether this element is an AnnotationElement2d
    bool IsDrawingGraphic() const {return nullptr != ToDrawingGraphic();}           //!< Determine whether this element is an DrawingGraphic
    bool IsSameType(DgnElementCR other) {return m_classId == other.m_classId;}      //!< Determine whether this element is the same type (has the same DgnClassId) as another element.

    //! Determine whether this is a copy of the "persistent state" (i.e. an exact copy of what is saved in the DgnDb) of a DgnElement.
    //! @note If this flag is true, this element must be readonly. To modify an element, call CopyForEdit.
    bool IsPersistent() const {return m_flags.m_persistent;}

    //! Create a writeable deep copy of a DgnElement for insert into the same or new model. Also see @ref ElementCopying.
    //! @param[out] stat Optional status to describe failures, a valid DgnElementPtr will only be returned if successful.
    //! @param[in] params Optional CreateParams. Might specify a different destination model, etc.
    //! @remarks If no CreateParams are supplied, a new DgnCode will be generated for the cloned element - it will \em not be copied from this element's DgnCode.
    DGNPLATFORM_EXPORT DgnElementPtr Clone(DgnDbStatus* stat=nullptr, DgnElement::CreateParams const* params=nullptr) const;

    //! Copy the content of another DgnElement into this DgnElement. Also see @ref ElementCopying.
    //! @param[in] source The other element whose content is copied into this element.
    //! @note This method @b does @b not change the DgnClassId, DgnModel or DgnElementId of this DgnElement. If the type of @a source is different
    //! than this element, then all of the data from subclasses in common are copied and the remaining data on this DgnElement are unchanged.
    void CopyFrom(DgnElementCR source) {_CopyFrom(source);}

    //! Make a writable copy of this DgnElement so that the copy may be edited. Also see @ref ElementCopying.
    //! @return a DgnElementPtr that holds the editable copy of this element.
    //! @note This method may only be used on a DgnElement this is the readonly persistent element returned by DgnElements::GetElement, and then
    //! only one editing copy of this element at a time may exist. If another copy is extant, this method will return an invalid DgnElementPtr.
    //! @see MakeCopy, IsPersistent
    DGNPLATFORM_EXPORT DgnElementPtr CopyForEdit() const;

    //! Make a writable copy of this DgnElement so that the copy may be edited. Also see @ref ElementCopying.
    //! This is merely a templated shortcut to dynamic_cast the return of CopyForEdit to a subclass of DgnElement.
    template<class T> RefCountedPtr<T> MakeCopy() const {return dynamic_cast<T*>(CopyForEdit().get());}

    //! Make a writable copy of the supplied DgnElement so that the copy may be edited. Also see @ref ElementCopying.
    //! This is merely a templated shortcut to dynamic_cast the return of CopyForEdit to a the supplied DgnElement's actual class.
    template<class T> static RefCountedPtr<T> MakeCopy(T const& elem) {auto clone=elem.CopyForEdit(); BeAssert(clone.IsNull() || nullptr != dynamic_cast<T*>(clone.get())); return static_cast<T*>(clone.get());}

    //! Create a copy of this DgnElement and all of its extended content in a destination model.
    //! The copied element will be persistent in the destination DgnDb. Also see @ref ElementCopying.
    //! @param[out] stat Optional status to describe failures, a valid DgnElementPtr will only be returned if successful.
    //! @param[in] destModel The destination model (which must be in the importer's destination Db).
    //! @param[in] importer Enables the element to copy the resources that it needs (if copying between DgnDbs) and to remap any references that it holds to things outside itself to the copies of those things.
    //! @remarks The element's code will \em not be copied to the copied element if the import is being performed within a single DgnDb, as it is never correct for two elements within the same DgnDb to have the same code.
    //! @return The persistent copy of the element
    //! @note This function can only be safely invoked from the client thread.
    DGNPLATFORM_EXPORT DgnElementCPtr Import(DgnDbStatus* stat, DgnModelR destModel, DgnImportContext& importer) const;

    //! Update the persistent state of a DgnElement in the DgnDb from this modified copy of it.
    //! This is merely a shortcut for el.GetDgnDb().Elements().Update(el, stat);
    //! @note This function can only be safely invoked from the client thread.
    DGNPLATFORM_EXPORT DgnElementCPtr Update(DgnDbStatus* stat=nullptr);

    //! Insert this DgnElement into the DgnDb.
    //! This is merely a shortcut for el.GetDgnDb().Elements().Insert(el, stat);
    //! @note This function can only be safely invoked from the client thread.
    DGNPLATFORM_EXPORT DgnElementCPtr Insert(DgnDbStatus* stat=nullptr);

    template<class T> RefCountedCPtr<T> InsertT(DgnDbStatus* stat=nullptr) {return dynamic_cast<T const*>(Insert(stat).get());}

    //! Delete this DgnElement from the DgnDb,
    //! This is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    //! @note This function can only be safely invoked from the client thread.
    DGNPLATFORM_EXPORT DgnDbStatus Delete() const;

    //! Get the ElementHandler for this DgnElement.
    DGNPLATFORM_EXPORT ElementHandlerR GetElementHandler() const;

    //! Check if this element is equal to source. Two elements are considered to be "equal" if they are instances of the same ECClass and if their properties have equivalent data.
    //! The element's identity and user properties may be excluded from the comparison.
    //! @param source   The element to compare with
    //! @param filter   Optional. The properties to exclude from the comparison.
    //! @return true if this element's properties are equivalent to the source element's properties.
    DGNPLATFORM_EXPORT bool Equals(DgnElementCR source, ComparePropertyFilter const& filter = ComparePropertyFilter(ComparePropertyFilter::Ignore::WriteOnly)) const;

    DGNPLATFORM_EXPORT void Dump(Utf8StringR str, ComparePropertyFilter const& filter) const;

    //! @name AppData Management
    //! @{
    //! Add Application Data to this element.
    //! @param[in] key The AppData's key. If AppData with this key already exists on this element, it is dropped and
    //! replaced with \a appData.
    //! @param[in] appData The appData object to attach to this element.
    void AddAppData(AppData::Key const& key, AppData* appData) const { BeMutexHolder lock(GetElementsMutex()); AddAppDataInternal(key, appData); }

    //! Drop Application data from this element.
    //! @param[in] key the key for the AppData to drop.
    //! @return SUCCESS if an entry with \a key is found and dropped.
    DGNPLATFORM_EXPORT StatusInt DropAppData(AppData::Key const& key) const;

    //! Find DgnElementAppData on this element by key.
    //! @param[in] key The key for the AppData of interest.
    //! @return the AppData for key \a key, or nullptr.
    AppDataPtr FindAppData(AppData::Key const& key) const { BeMutexHolder lock(GetElementsMutex()); return FindAppDataInternal(key); }

    //! Find application data on this element by key, or add it if not found.
    //! @param[in] key The key specifying the app data of interest
    //! @param[in] createAppData A callable taking no arguments and returning an AppData*
    //! @return the existing or newly-created app data corresponding to the supplied key.
    template<typename T> AppDataPtr FindOrAddAppData(AppData::Key const& key, T createAppData) const
        {
        BeMutexHolder lock(GetElementsMutex());
        AppData* data = FindAppDataInternal(key);
        if (nullptr == data)
            AddAppDataInternal(key, data = createAppData());

        return data;
        }

    //! Find application data on this element by key, or add it if not found.
    //! @param[in] key The key specifying the app data of interest
    //! @param[in] createAppData A callable taking no arguments and returning an AppData*
    //! @return the existing or newly-created app data corresponding to the supplied key, as a reference-counted pointer to the derived type.
    template<typename T> auto ObtainAppData(AppData::Key const& key, T createAppData) const -> RefCountedPtr<typename std::remove_pointer<decltype(createAppData())>::type>
        {
        using U = decltype(createAppData());
        AppDataPtr data = FindOrAddAppData(key, createAppData);
        return static_cast<U>(data.get());
        }

    //! Add app data, replacing a previous entry if it exists.
    //! @param[in] key The key specifying the app data of interest.
    //! @param[in] appdata The new app data to associate with the key.
    DGNPLATFORM_EXPORT void ReplaceAppData(AppData::Key const& key, AppData* appData) const;

    //! @private
    DGNPLATFORM_EXPORT void CopyAppDataFrom(DgnElementCR source) const;

    //! @}

    //! Get the DgnDb of this DgnElement.
    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! Get the DgnModelId of the DgnModel that contains this DgnElement.
    DgnModelId GetModelId() const {return m_modelId;}

    //! Get the DgnModel that contains this DgnElement.
    DGNPLATFORM_EXPORT DgnModelPtr GetModel() const;

    //! Get the (optional) DgnModelId of the DgnModel that is modeling this DgnElement.  That is, the DgnModel that is beneath this element in the hierarchy.
    //! @return Invalid if model does not exist
    DGNPLATFORM_EXPORT DgnModelId GetSubModelId() const;

    //! Get the (optional) DgnModel that is modeling this DgnElement. That is, the DgnModel that is beneath this element in the hierarchy.
    //! @return Invalid if model does not exist
    DGNPLATFORM_EXPORT DgnModelPtr GetSubModel() const;

    //! Get the (optional) DgnModel that is modeling this DgnElement. That is, the DgnModel that is beneath this element in the hierarchy.
    //! @return Invalid if model does not exist
    template<class T> RefCountedPtr<T> GetSub() const {return dynamic_cast<T*>(GetSubModel().get());}

    //! Get the DgnElementId of this DgnElement
    DgnElementId GetElementId() const {return m_elementId;}

    //! Only valid to be used in very specific synchronization workflows. All other workflows should allow Insert to use next available DgnElementId.
    //! @private
    DGNPLATFORM_EXPORT void ForceElementIdForInsert(DgnElementId);

    //! Get the DgnClassId of this DgnElement.
    DgnClassId GetElementClassId() const {return m_classId;}

    //! Get the ECInstanceKey (the element DgnClassId and DgnElementId) of this DgnElement
    //! @see GetElementClassId, GetElementId
    BeSQLite::EC::ECInstanceKey GetECInstanceKey() const {return BeSQLite::EC::ECInstanceKey(GetElementClassId(), BeSQLite::EC::ECInstanceId(GetElementId().GetValue()));}

    //! Get a pointer to the ECClass of this DgnElement.
    DGNPLATFORM_EXPORT ECN::ECClassCP GetElementClass() const;

    //! Get the FederationGuid of this DgnElement.
    BeSQLite::BeGuidCR GetFederationGuid() const {return m_federationGuid;}

    //! Set the FederationGuid for this DgnElement.
    //! @note To clear the FederationGuid, pass BeGuid() since an invalid BeGuid indicates a null value is desired
    void SetFederationGuid(BeSQLite::BeGuidCR federationGuid) {m_federationGuid = federationGuid;}

    //! Get the DgnElementId of the parent of this element.
    //! @see SetParentId
    //! @return Id will be invalid if this element does not have a parent element.
    DgnElementId GetParentId() const {return m_parent.m_id;}

    //! Test if \a ancestorId identifies the parent of this element or of its parent, recursively.
    DGNPLATFORM_EXPORT bool IsDescendantOf(DgnElementId ancestorId) const;

    //! Get the DgnClassId of the ElementOwnsChildElements subclass used to relate this element to its parent element.
    //! @return Will be invalid if this element does not have a parent element.
    DgnClassId GetParentRelClassId() const {return m_parent.m_id.IsValid() ? m_parent.m_relClassId : DgnClassId();}

    //! Set the parent (owner) of this DgnElement.
    //! @see GetParentId, _SetParentId
    //! @return DgnDbStatus::Success if the parent was set
    //! @note This call can fail if a DgnElement subclass overrides _SetParentId and rejects the parent.
    DgnDbStatus SetParentId(DgnElementId parentId, DgnClassId parentRelClassId) {return parentId == GetParentId() && parentRelClassId == GetParentRelClassId() ? DgnDbStatus::Success : _SetParentId(parentId, parentRelClassId);}

    //! Return the DgnCode of this DgnElement
    DgnCodeCR GetCode() const {return m_code;}

    //! Generate a default code for this DgnElement
    DgnCode GenerateDefaultCode() const {return _GenerateDefaultCode();}

    DGNPLATFORM_EXPORT DgnDbStatus SetCode(DgnCodeCR newCode);
    DGNPLATFORM_EXPORT CodeSpecCPtr GetCodeSpec() const;
    bool SupportsCodeSpec(CodeSpecCR codeSpec) const {return _SupportsCodeSpec(codeSpec);}

    //! Query the database for the last modified time of this DgnElement.
    DGNPLATFORM_EXPORT DateTime QueryLastModifyTime() const;

    //! Return true if this DgnElement has a label.
    bool HasUserLabel() const {return !m_userLabel.empty();}

    //! Get the label of this DgnElement.
    //! @note may be nullptr
    Utf8CP GetUserLabel() const {return m_userLabel.c_str();}

    //! Set the label of this DgnElement.
    void SetUserLabel(Utf8CP label) {m_userLabel.AssignOrClear(label);}

    //! Get the display label (for use in the GUI) of this DgnElement.
    //! @note The default implementation returns the label if it is set or the code if the label is not set.
    //! @see GetUserLabel, GetCode, _GetDisplayLabel
    Utf8String GetDisplayLabel() const {return _GetDisplayLabel();}

    //! Query the DgnDb for the children of this DgnElement.
    //! @return DgnElementIdSet containing the DgnElementIds of all child elements of this DgnElement. Will be empty if no children.
    DGNPLATFORM_EXPORT DgnElementIdSet QueryChildren() const;

    //! Disclose any locks which must be acquired and/or codes which must be reserved to perform the specified operation on this element.
    //! @param[in] request Request to populate
    //! @param[in] opcode The operation to be performed
    //! @return RepositoryStatus::Success, or an error code if for example a required lock or code is known to be unavailable without querying the repository manager.
    DGNPLATFORM_EXPORT RepositoryStatus PopulateRequest(IBriefcaseManager::Request& request, BeSQLite::DbOpcode opcode) const;

    //! @name JsonProperties 
    //! @{
    //! Get the current value of a set of Json Properties on this element
    ECN::AdHocJsonValueCR GetJsonProperties(Utf8CP nameSpace) const {return m_jsonProperties.GetMember(nameSpace);}

    //! Change the value of a set of Json Properties on this element
    void SetJsonProperties(Utf8CP nameSpace, JsonValueCR value) {m_jsonProperties.GetMemberR(nameSpace) = (ECN::AdHocJsonValueCR) value;}

    //! Remove a set of Json Properties on this element
    void RemoveJsonProperties(Utf8CP nameSpace) {m_jsonProperties.RemoveMember(nameSpace);}

    ECN::AdHocJsonValueCR GetUserProperties(Utf8CP nameSpace) const {return GetJsonProperties(json_UserProps()).GetMember(nameSpace);}

    void SetUserProperties(Utf8CP nameSpace, JsonValueCR value) {GetUserPropsR().GetMemberR(nameSpace) = (ECN::AdHocJsonValueCR) value;}

    void RemoveUserProperties(Utf8CP nameSpace) {GetUserPropsR().RemoveMember(nameSpace);}

    /** @} */

    //! @name Properties 
    //! @{
    //! Get the index of the property
    //! @param[out] index       The index of the property in the ECClass
    //! @param[in] accessString The access setring. For simple properties, this is the name of the property. For struct members, this is the dot-separated path to the member.
    //! @return non-zero error status if accessString does not identify a property.
    DGNPLATFORM_EXPORT DgnDbStatus GetPropertyIndex(uint32_t& index, Utf8CP accessString);

    //! Return the value of a DateTime ECProperty by name
    //! @note Returns an invalid DateTime if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT DateTime GetPropertyValueDateTime(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex()) const;

    //! Return the value of a DPoint3d ECProperty by name
    //! @note Returns DPoint3d::From(0,0,0) if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT DPoint3d GetPropertyValueDPoint3d(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex()) const;

    //! Return the value of a DPoint2d ECProperty by name
    //! @note Returns DPoint2d::From(0,0,0) if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT DPoint2d GetPropertyValueDPoint2d(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex()) const;

    //! Return the value of a boolean ECProperty by name
    //! @note Returns false if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT bool GetPropertyValueBoolean(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex()) const;

    //! Return the value of a double ECProperty by name
    //! @note Returns 0.0 if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT double GetPropertyValueDouble(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex()) const;

    //! Return the value of a integer ECProperty by name
    //! @note Returns 0 if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT int32_t GetPropertyValueInt32(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex()) const;

    //! Return the value of a UInt64 ECProperty by name
    //! @note Returns 0 if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT uint64_t GetPropertyValueUInt64(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex()) const;

    //! Return the NavigationPropertyInfo for an ECNavigationProperty of the specified name
    DGNPLATFORM_EXPORT NavigationPropertyInfo GetNavigationPropertyInfo(Utf8CP propertyName) const;

    //! Return the value of the Id of an ECNavigationProperty by name
    template <class TBeInt64Id> TBeInt64Id GetPropertyValueId(Utf8CP propertyName) const
        {
        return GetNavigationPropertyInfo(propertyName).GetId<TBeInt64Id>();
        }

    //! Return the value of a string ECProperty by name
    DGNPLATFORM_EXPORT Utf8String GetPropertyValueString(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex()) const;

    //! Return the value of a GUID ECProperty by name
    DGNPLATFORM_EXPORT BeSQLite::BeGuid GetPropertyValueGuid(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex()) const;

    //! Get the 3 property values that back a YPR
    DGNPLATFORM_EXPORT YawPitchRollAngles GetPropertyValueYpr(Utf8CP yawName, Utf8CP pitchName, Utf8CP rollName) const;

    //! Set a DateTime ECProperty by name
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, DateTimeCR value, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex());
    //! Set a DPoint3d ECProperty by name
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, DPoint3dCR value, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex());
    //! Set a DPoint2d ECProperty by name
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, DPoint2dCR value, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex());
    //! Set a boolean ECProperty by name
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, bool value, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex());
    //! Set a double ECProperty by name
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, double value, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex());
    //! Set an integer ECProperty by name
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, int32_t value, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex());
    //! Set an int64_t ECProperty by name
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, int64_t value, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex());

    //! Set an ECNavigationProperty by name
    //! @param[in] propertyName The name of the navigation property
    //! @param[in] elementId The DgnElementId that identifies the target element
    //! @param[in] relClassId Optional. The relationship class that defines the navigation property.
    //! @note Passing an invalid elementId will cause a null value to be set.
    DgnDbStatus SetPropertyValue(Utf8CP propertyName, DgnElementId elementId, DgnClassId relClassId = DgnClassId()) 
        {return SetPropertyValue(propertyName, (BeSQLite::EC::ECInstanceId)(elementId.GetValueUnchecked()), relClassId);}
    //! Set an ECNavigationProperty by name
    //! @param[in] propertyName The name of the navigation property
    //! @param[in] modelId Identifies the target model
    //! @param[in] relClassId Optional. The relationship class that defines the navigation property.
    //! @note Passing an invalid modelId will cause a null value to be set.
    DgnDbStatus SetPropertyValue(Utf8CP propertyName, DgnModelId modelId, DgnClassId relClassId = DgnClassId())
        {return SetPropertyValue(propertyName, (BeSQLite::EC::ECInstanceId)(modelId.GetValueUnchecked()), relClassId);}
    //! Set a string ECProperty by name
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, Utf8CP value, PropertyArrayIndex const& arrayIndex = PropertyArrayIndex());
    //! Set the three property values that back a YPR
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValueYpr(YawPitchRollAnglesCR angles, Utf8CP yawName, Utf8CP pitchName, Utf8CP rollName);

    //! Get the value of a property. Also see @ref ElementProperties.
    //! @param value The returned value
    //! @param accessString The access string that identifies the property. @see GetPropertyIndex
    //! @param aidx Optional. If the property is an array, you must specify the index of the item to get.
    //! @return non-zero error status if this element has no such property or if the subclass has chosen not to expose it via this function
    DgnDbStatus GetPropertyValue(ECN::ECValueR value, Utf8CP accessString, PropertyArrayIndex aidx = PropertyArrayIndex()) const
        {
        ElementECPropertyAccessor access(*this, accessString);
        return access.IsValid()? _GetPropertyValue(value, access, aidx): DgnDbStatus::WrongClass;
        }

    //! Get the value of a property. Also see @ref ElementProperties.
    //! @param value The returned value
    //! @param propIndex The index of the property. @see GetPropertyIndex
    //! @param aidx Optional. If the property is an array, you must specify the index of the item to get.
    //! @return non-zero error status if this element has no such property or if the subclass has chosen not to expose it via this function
    DgnDbStatus GetPropertyValue(ECN::ECValueR value, uint32_t propIndex, PropertyArrayIndex aidx = PropertyArrayIndex()) const
        {
        ElementECPropertyAccessor access(*this, propIndex);
        return access.IsValid()? _GetPropertyValue(value, access, aidx): DgnDbStatus::WrongClass;
        }

    //! Set the value of a property. 
    //! @note This function does not write to the bim. The caller must call Update to write the element and all of 
    //! its modified property to the DgnDb. Also see @ref ElementProperties.
    //! @param value The returned value
    //! @param accessString The access string that identifies the property. @see GetPropertyIndex
    //! @param aidx Optional. If the property is an array, you must specify the index of the item to set.
    //! @return non-zero error status if this element has no such property, if the value is illegal, or if the subclass has chosen not to expose the property via this function
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP accessString, ECN::ECValueCR value, PropertyArrayIndex aidx = PropertyArrayIndex())
        {
        ElementECPropertyAccessor access(*this, accessString);
        return access.IsValid()? _SetPropertyValue(access, value, aidx): DgnDbStatus::WrongClass;
        }

    //! Set the value of a property. 
    //! @note This function does not write to the bim. The caller must call Update to write the element and all of 
    //! its modified property to the DgnDb. Also see @ref ElementProperties.
    //! @param value The returned value
    //! @param propIndex The index of the property. @see GetPropertyIndex
    //! @param aidx Optional. If the property is an array, you must specify the index of the item to set.
    //! @return non-zero error status if this element has no such property, if the value is illegal, or if the subclass has chosen not to expose the property via this function
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(uint32_t propIndex, ECN::ECValueCR value, PropertyArrayIndex aidx = PropertyArrayIndex())
        {
        ElementECPropertyAccessor access(*this, propIndex);
        return access.IsValid()? _SetPropertyValue(access, value, aidx): DgnDbStatus::WrongClass;
        }

    //! Set the properties of this element from the specified instance. Calls _SetPropertyValue for each non-NULL property in the input instance.
    //! @param instance The source of the properties that are to be copied to this element
    //! @param filter   Optional. The properties to exclude.
    //! @return non-zero error status if any property could not be set. Note that some properties might be set while others are not in case of error.
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValues(ECN::IECInstanceCR instance, SetPropertyFilter const& filter 
                                                     = SetPropertyFilter(SetPropertyFilter::Ignore::WriteOnlyNullBootstrapping));

    //! Given a property index, will insert size number of empty array items at the given index
    //! @param[in] propertyIndex The index (into the ClassLayout) of the array property
    //! @param[in] index    The starting index of the array at which to insert the new items
    //! @param[in] size The number of empty array items to insert
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    //! @see GetPropertyIndex
    DGNPLATFORM_EXPORT DgnDbStatus InsertPropertyArrayItems(uint32_t propertyIndex, uint32_t index, uint32_t size);

    //! Given a property index and an array index, will remove a single array item
    //! @param[in] propertyIndex The index (into the ClassLayout) of the array property
    //! @param[in] index    The index of the item to remove
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    //! @see GetPropertyIndex
    DGNPLATFORM_EXPORT DgnDbStatus RemovePropertyArrayItem(uint32_t propertyIndex, uint32_t index);

    //! Given a property index, will add size number of empty array items to the end of the array
    //! @param[in] propertyIndex The index (into the ClassLayout) of the array property
    //! @param[in] size The number of empty array items to add
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    //! @see GetPropertyIndex
    DGNPLATFORM_EXPORT DgnDbStatus AddPropertyArrayItems(uint32_t propertyIndex, uint32_t size);

    //! Given a property index, removes all array items from the array
    //! @param[in] propertyIndex The index (into the ClassLayout) of the array property
    //! @returns SUCCESS if successful, otherwise an error code indicating the failure
    //! @see GetPropertyIndex
    DGNPLATFORM_EXPORT DgnDbStatus ClearPropertyArray(uint32_t propertyIndex);

    //! Create a Json::Value that represents the state of this element.
    //! @param[in] opts options for customizing the value. If opts["wantGeometry"] != true, geometry stream Json::Value is not included.
    Json::Value ToJson(JsonValueCR opts = Json::Value()) const { Json::Value val; _ToJson(val, opts); return val; }

    void FromJson(JsonValueR props) {_FromJson(props);}
    //! @}

    //! Make an iterator over all ElementAspects owned by this element
    DGNPLATFORM_EXPORT ElementAspectIterator MakeAspectIterator() const;
};

//=======================================================================================
//! A stream of geometry, stored on a DgnElement, created by a GeometryBuilder.
//! @ingroup GROUP_Geometry
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
struct GeometryStream : ByteStream
{
public:
    bool HasGeometry() const {return HasData();}  //!< return false if this GeometryStream is empty.
    DGNPLATFORM_EXPORT DgnDbStatus ReadGeometryStream(BeSQLite::SnappyFromMemory& snappy, DgnDbR dgnDb, void const* blob, int blobSize); //!< @private
    static DgnDbStatus WriteGeometryStream(BeSQLite::SnappyToBlob&, DgnDbR, DgnElementId, Utf8CP className, Utf8CP propertyName); //!< @private
    DgnDbStatus BindGeometryStream(bool& multiChunkGeometryStream, BeSQLite::SnappyToBlob&, BeSQLite::EC::ECSqlStatement&, Utf8CP parameterName) const; //!< @private
};

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  11/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometrySource
{
    friend struct GeometryBuilder;

protected:
    virtual DgnDbR _GetSourceDgnDb() const = 0;
    virtual DgnElementCP _ToElement() const = 0;
    virtual GeometrySource2dCP _GetAsGeometrySource2d() const = 0; // Either this method or _GetAsGeometrySource3d must return non-null.
    virtual GeometrySource3dCP _GetAsGeometrySource3d() const = 0; // Either this method or _GetAsGeometrySource2d must return non-null.
    virtual DgnCategoryId _GetCategoryId() const = 0;
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) = 0;
    virtual GeometryStreamCR _GetGeometryStream() const = 0;
    virtual AxisAlignedBox3d _CalculateRange3d() const = 0;
    DGNPLATFORM_EXPORT virtual Render::GraphicPtr _Stroke(ViewContextR, double pixelSize) const;
    GeometryStreamR GetGeometryStreamR() {return const_cast<GeometryStreamR>(_GetGeometryStream());} // Only GeometryBuilder should have write access to the GeometryStream...

    friend struct GeometricElement;
public:
    virtual ~GeometrySource() { }

    bool Is3d() const {return nullptr != _GetAsGeometrySource3d();}    //!< Determine whether this GeometrySource is 3d or not
    bool Is2d() const {return nullptr != _GetAsGeometrySource2d();}    //!< Determine whether this GeometrySource is 2d or not
    bool HasGeometry() const {return _GetGeometryStream().HasGeometry();} //!< return false if this geometry source currently has no geometry (is empty).
    DgnDbR GetSourceDgnDb() const {return _GetSourceDgnDb();}
    DgnElementCP ToElement() const {return _ToElement();} //! Caller must be prepared to this to return nullptr.
    DgnElementP ToElementP() {return const_cast<DgnElementP>(_ToElement());} //! Caller must be prepared to this to return nullptr.
    GeometrySource2dCP GetAsGeometrySource2d() const {return _GetAsGeometrySource2d();}
    GeometrySource2dP GetAsGeometrySource2dP() {return const_cast<GeometrySource2dP>(_GetAsGeometrySource2d());}
    GeometrySource3dCP GetAsGeometrySource3d() const {return _GetAsGeometrySource3d();}
    GeometrySource3dP GetAsGeometrySource3dP() {return const_cast<GeometrySource3dP>(_GetAsGeometrySource3d());}
    DgnCategoryId GetCategoryId() const {return _GetCategoryId();}
    DgnDbStatus SetCategoryId(DgnCategoryId categoryId) {return _SetCategoryId(categoryId);}
    GeometryStreamCR GetGeometryStream() const {return _GetGeometryStream();}
    AxisAlignedBox3d CalculateRange3d() const {return _CalculateRange3d();}
    DGNPLATFORM_EXPORT Transform GetPlacementTransform() const;

    DGNPLATFORM_EXPORT bool IsUndisplayed() const; //!< @private
    DGNPLATFORM_EXPORT void SetUndisplayed(bool yesNo) const; //!< @private

    Render::GraphicPtr Stroke(ViewContextR context, double pixelSize) const {return _Stroke(context, pixelSize);}
    DGNPLATFORM_EXPORT Render::GraphicPtr Draw(ViewContextR context, double pixelSize) const;
};

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  11/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometrySource3d : GeometrySource
{
protected:
    GeometrySource2dCP _GetAsGeometrySource2d() const override final {return nullptr;}
    AxisAlignedBox3d _CalculateRange3d() const override final {return _GetPlacement().CalculateRange();}
    virtual Placement3dCR _GetPlacement() const = 0;
    virtual DgnDbStatus _SetPlacement(Placement3dCR placement) = 0;

public:
    Placement3dCR GetPlacement() const {return _GetPlacement();} //!< Get the Placement3d of this element
    DgnDbStatus SetPlacement(Placement3dCR placement) {return _SetPlacement(placement);} //!< Change the Placement3d for this element
};

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  11/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometrySource2d : GeometrySource
{
protected:
    GeometrySource3dCP _GetAsGeometrySource3d() const override final {return nullptr;}
    AxisAlignedBox3d _CalculateRange3d() const override final {return _GetPlacement().CalculateRange();}
    virtual Placement2dCR _GetPlacement() const = 0;
    virtual DgnDbStatus _SetPlacement(Placement2dCR placement) = 0;

public:
    Placement2dCR GetPlacement() const {return _GetPlacement();} //!< Get the Placement2d of this element
    DgnDbStatus SetPlacement(Placement2dCR placement) {return _SetPlacement(placement);} //!< Change the Placement2d for this element
};

//=======================================================================================
//! Base class for elements with geometry.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Paul.Connelly   02/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricElement : DgnElement
{
    DEFINE_T_SUPER(DgnElement);

    friend struct dgn_ElementHandler::Geometric3d;
    friend struct dgn_ElementHandler::Geometric2d;

    //! Parameters used to construct a GeometricElement
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(GeometricElement::T_Super::CreateParams);

        DgnCategoryId   m_category; //!< The category to which the element belongs

        //! Construct from the supplied parameters
        //! @param[in] db The DgnDb in which the element is to reside
        //! @param[in] modelId The Id of the model in which the element is to reside
        //! @param[in] classId The Id of the element's ECClass
        //! @param[in] category The category to which the element belongs
        //! @param[in] code The element's code
        //! @param[in] label (Optional) element label
        //! @param[in] parent (Optional) Id of this element's parent element
        //! @param[in] parentRelClassId (Optional) The ECClassId of the parent relationship. Must be a subclass of BisCore:ElementOwnsChildElements
        //! @param[in] federationGuid (Optional) FederationGuid for this element
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCategoryId category, DgnCodeCR code=DgnCode(), Utf8CP label=nullptr, DgnElementId parent=DgnElementId(), DgnClassId parentRelClassId=DgnClassId(), BeSQLite::BeGuidCR federationGuid=BeSQLite::BeGuid(false))
            : T_Super(db, modelId, classId, code, label, parent, parentRelClassId, federationGuid), m_category(category) {}

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @param[in]      category The category to which the element belongs
        //! @return 
        explicit CreateParams(DgnElement::CreateParams const& params, DgnCategoryId category=DgnCategoryId()) : T_Super(params), m_category(category) {}
    };

    BE_PROP_NAME(GeometryStream)
    BE_PROP_NAME(BBoxLow)
    BE_PROP_NAME(BBoxHigh)
    BE_PROP_NAME(Origin)
    BE_PROP_NAME(TypeDefinition)
    BE_PROP_NAME(Category)

    BE_JSON_NAME(origin)
    BE_JSON_NAME(placement)
    BE_JSON_NAME(typeDefinition)
    BE_JSON_NAME(category)
    BE_JSON_NAME(geom)

protected:
    DgnCategoryId m_categoryId;
    GeometryStream m_geom;
    mutable bool m_multiChunkGeomStream;

    explicit GeometricElement(CreateParams const& params) : T_Super(params), m_categoryId(params.m_category), m_multiChunkGeomStream(false) {}

    virtual bool _IsPlacementValid() const = 0;
    virtual Utf8CP _GetGeometryColumnClassName() const = 0;
    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT DgnDbStatus _InsertInDb() override;
    DGNPLATFORM_EXPORT DgnDbStatus _UpdateInDb() override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT void _OnInserted(DgnElementP copiedFrom) const override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnUpdate(DgnElementCR) override;
    DGNPLATFORM_EXPORT void _OnDeleted() const override;
    DGNPLATFORM_EXPORT void _OnAppliedDelete() const override;
    DGNPLATFORM_EXPORT void _OnAppliedAdd() const override;
    DGNPLATFORM_EXPORT void _OnUpdateFinished() const override;
    DGNPLATFORM_EXPORT void _RemapIds(DgnImportContext&) override;
    uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() + (sizeof(*this) - sizeof(T_Super)) + m_geom.GetAllocSize();}
    DGNPLATFORM_EXPORT bool _EqualProperty(ECN::ECPropertyValueCR prop, DgnElementCR other) const override; // Handles GeometryStream
    static void RegisterGeometricPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR);

    GeometryStreamCR GetGeometryStream() const {return m_geom;}
    DgnDbStatus InsertGeomStream() const;
    DgnDbStatus UpdateGeomStream() const;
    DgnDbStatus WriteGeomStream() const;
    DgnDbStatus Validate() const;
    DGNPLATFORM_EXPORT DgnDbStatus DoSetCategoryId(DgnCategoryId catId);
    void CopyFromGeometrySource(GeometrySourceCR);
};

//=======================================================================================
//! Base class for elements with 3d geometry.
//! GeometricElement3d elements are not inherently spatially located, but can be spatially located.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Paul.Connelly   02/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricElement3d : GeometricElement, GeometrySource3d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_GeometricElement3d, GeometricElement)
    friend struct dgn_ElementHandler::Geometric3d;

public:
    //! Parameters for constructing a 3d geometric element
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(GeometricElement3d::T_Super::CreateParams);

        Placement3d m_placement; //!< The element's placement in 3d space

        //! Construct from supplied parameters
        //! @param[in] db        The DgnDb in which the element is to reside
        //! @param[in] modelId   The Id of the model in which the element is to reside
        //! @param[in] classId   The Id of the element's ECClass
        //! @param[in] category  The category to which the element belongs
        //! @param[in] placement The element's placement in 3d space
        //! @param[in] code      The element's code
        //! @param[in] label     (Optional) element label
        //! @param[in] parentId  (Optional) Id of this element's parent element
        //! @param[in] parentRelClassId (Optional) The ECClassId of the parent relationship.  Must be a subclass of BisCore:ElementOwnsChildElements
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCategoryId category, Placement3dCR placement=Placement3d(),
                DgnCodeCR code=DgnCode(), Utf8CP label=nullptr, DgnElementId parentId=DgnElementId(), DgnClassId parentRelClassId=DgnClassId())
            : T_Super(db, modelId, classId, category, code, label, parentId, parentRelClassId), m_placement(placement) {}

        //! Construct from base parameters. Chiefly for internal use
        //! @param[in] params    The base element parameters
        //! @param[in] category  The category to which the element belongs
        //! @param[in] placement The element's placement in 3d space
        explicit CreateParams(DgnElement::CreateParams const& params, DgnCategoryId category=DgnCategoryId(), Placement3dCR placement=Placement3d())
            : T_Super(params, category), m_placement(placement) {}
    };

protected:
    Placement3d m_placement;
    RelatedElement m_typeDefinition;

    explicit GeometricElement3d(CreateParams const& params) : T_Super(params), m_placement(params.m_placement) {}
    bool _IsPlacementValid() const override final {return m_placement.IsValid();}
    DgnDbR _GetSourceDgnDb() const override final {return GetDgnDb();}
    DgnElementCP _ToElement() const override final {return this;}
    GeometrySourceCP _ToGeometrySource() const override final {return this;}
    GeometrySource3dCP _GetAsGeometrySource3d() const override final {return this;}
    Utf8CP _GetGeometryColumnClassName() const override final {return BIS_CLASS_GeometricElement3d;}
    DgnCategoryId _GetCategoryId() const override final {return m_categoryId;}
    DGNPLATFORM_EXPORT DgnDbStatus _SetCategoryId(DgnCategoryId) override;
    GeometryStreamCR _GetGeometryStream() const override final {return m_geom;}
    Placement3dCR _GetPlacement() const override final {return m_placement;}
    DGNPLATFORM_EXPORT DgnDbStatus _SetPlacement(Placement3dCR placement) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR) override;
    DGNPLATFORM_EXPORT void _AdjustPlacementForImport(DgnImportContext const&) override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnUpdate(DgnElementCR) override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT void _OnDeleted() const override;

public:
    BE_PROP_NAME(InSpatialIndex)
    BE_PROP_NAME(Yaw)
    BE_PROP_NAME(Pitch)
    BE_PROP_NAME(Roll)

    //! Set the TypeDefinitionElement associated with this GeometricElement3d
    //! @param[in] typeDefinitionId The DgnElementId of the TypeDefinitionElement to be associated with this GeometricElement3d
    //! @param[in] relClassId The ECClassId of the ECRelationshipClass that must be a subclass of BisCore:GeometricElement3dHasTypeDefinition
    DGNPLATFORM_EXPORT DgnDbStatus SetTypeDefinition(DgnElementId typeDefinitionId, ECN::ECClassId relClassId);

    //! Get the DgnElementId of the TypeDefinitionElement for this GeometricElement3d
    //! @return Will be invalid if there is no TypeDefinitionElement associated with this GeometricElement3d
    DgnElementId GetTypeDefinitionId() const {return m_typeDefinition.m_id;}

    //! Get the DgnClassId of the relationship class that associates the TypeDefinitionElement with this GeometricElement3d
    //! @return Will be invalid if there is no TypeDefinitionElement associated with this GeometricElement3d
    DgnClassId GetTypeDefinitionRelClassId() const {return m_typeDefinition.m_relClassId;}
};

//=======================================================================================
//! Base class for elements with 2d geometry
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Paul.Connelly   02/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricElement2d : GeometricElement, GeometrySource2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_GeometricElement2d, GeometricElement)
    friend struct dgn_ElementHandler::Geometric2d;

public:
    //! Parameters for constructing a 2d geometric element
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(GeometricElement2d::T_Super::CreateParams);

        Placement2d m_placement; //!< The element's placement in 2d space

        //! Construct from supplied parameters
        //! @param[in] db        The DgnDb in which the element is to reside
        //! @param[in] modelId   The Id of the model in which the element is to reside
        //! @param[in] classId   The Id of the element's ECClass
        //! @param[in] category  The Id of the category to which the element belongs
        //! @param[in] placement The element's placement in 2d space
        //! @param[in] code      The element's code
        //! @param[in] label     (Optional) element label
        //! @param[in] parent    (Optional) Id of this element's parent element
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCategoryId category, Placement2dCR placement=Placement2d(),
                DgnCodeCR code=DgnCode(), Utf8CP label=nullptr, DgnElementId parent=DgnElementId())
            : T_Super(db, modelId, classId, category, code, label, parent), m_placement(placement) {}

        //! Construct from base parameters. Chiefly for internal use.
        //! @param[in] params    The base element parameters
        //! @param[in] category  The Id of the category to which the element belongs
        //! @param[in] placement The element's placement in 2d space
        explicit CreateParams(DgnElement::CreateParams const& params, DgnCategoryId category=DgnCategoryId(), Placement2dCR placement=Placement2d())
            : T_Super(params, category), m_placement(placement) {}
    };

protected:
    Placement2d m_placement;
    RelatedElement m_typeDefinition;

    explicit GeometricElement2d(CreateParams const& params) : T_Super(params), m_placement(params.m_placement) {}
    bool _IsPlacementValid() const override final {return m_placement.IsValid();}
    DgnDbR _GetSourceDgnDb() const override final {return GetDgnDb();}
    DgnElementCP _ToElement() const override final {return this;}
    GeometrySourceCP _ToGeometrySource() const override final {return this;}
    GeometrySource2dCP _GetAsGeometrySource2d() const override final {return this;}
    Utf8CP _GetGeometryColumnClassName() const override final {return BIS_CLASS_GeometricElement2d;}
    DgnCategoryId _GetCategoryId() const override final {return m_categoryId;}
    DGNPLATFORM_EXPORT DgnDbStatus _SetCategoryId(DgnCategoryId) override;
    GeometryStreamCR _GetGeometryStream() const override final {return m_geom;}
    Placement2dCR _GetPlacement() const override final {return m_placement;}
    DGNPLATFORM_EXPORT DgnDbStatus _SetPlacement(Placement2dCR placement) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR) override;
    DGNPLATFORM_EXPORT void _AdjustPlacementForImport(DgnImportContext const&) override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT void _OnDeleted() const override;

public:
    BE_PROP_NAME(Rotation)

    //! Set the TypeDefinitionElement associated with this GeometricElement2d
    //! @param[in] typeDefinitionId The DgnElementId of the TypeDefinitionElement to be associated with this GeometricElement2d
    //! @param[in] relClassId The ECClassId of the ECRelationshipClass that must be a subclass of BisCore:GeometricElement2dHasTypeDefinition
    DGNPLATFORM_EXPORT DgnDbStatus SetTypeDefinition(DgnElementId typeDefinitionId, ECN::ECClassId relClassId);

    //! Get the DgnElementId of the TypeDefinitionElement for this GeometricElement2d
    //! @return Will be invalid if there is no TypeDefinitionElement associated with this GeometricElement2d
    DgnElementId GetTypeDefinitionId() const {return m_typeDefinition.m_id;}

    //! Get the DgnClassId of the relationship class that associates the TypeDefinitionElement with this GeometricElement2d
    //! @return Will be invalid if there is no TypeDefinitionElement associated with this GeometricElement2d
    DgnClassId GetTypeDefinitionRelClassId() const {return m_typeDefinition.m_relClassId;}
};

//=======================================================================================
//! A 3-dimensional geometric element that is used to convey information in 3-dimensional graphical presentations.
//! It is common for the GeometryStream of a GraphicalElement3d to contain display-oriented metadata such as symbology overrides, styles, etc.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    02/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GraphicalElement3d : GeometricElement3d
{
    DEFINE_T_SUPER(GeometricElement3d);
protected:
    explicit GraphicalElement3d(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! An abstract base class for elements that occupy real world 3-dimensional space
//! It is uncommon for the GeometryStream of a SpatialElement to contain display-oriented metadata. 
//! Instead, display-oriented settings should come from the SubCategories that classify the geometry in the GeometryStream.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpatialElement : GeometricElement3d
{
    DEFINE_T_SUPER(GeometricElement3d);
protected:
    explicit SpatialElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A PhysicalElement is a SpatialElement that has mass and can be physically "touched".
//! Examples (which would be subclasses) include pumps, walls, and light posts.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalElement : SpatialElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_PhysicalElement, SpatialElement)
    friend struct dgn_ElementHandler::Physical;

protected:
    explicit PhysicalElement(CreateParams const& params) : T_Super(params) {}

public:
    //! Get the PhysicalType for this PhysicalElement
    //! @return Will be invalid if there is no PhysicalType associated with this PhysicalElement
    DGNPLATFORM_EXPORT PhysicalTypeCPtr GetPhysicalType() const;
};

//=======================================================================================
//! A PhysicalPortion represents an arbitrary portion of a larger PhysicalElement that will be broken down in more detail in a separate (sub) PhysicalModel.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalPortion : PhysicalElement
{
    DEFINE_T_SUPER(PhysicalElement);
protected:
    explicit PhysicalPortion(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A SpatialElement that identifies a "tracked" real word 3-dimensional location but has no mass and cannot be "touched".
//! Examples include grid lines, parcel boundaries, and work areas.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpatialLocationElement : SpatialElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_SpatialLocationElement, SpatialElement)
    friend struct dgn_ElementHandler::SpatialLocation;

protected:
    explicit SpatialLocationElement(CreateParams const& params) : T_Super(params) {}

public:
    //! Get the SpatialLocationType for this SpatialLocationElement
    //! @return Will be invalid if there is no SpatialLocationType associated with this SpatialLocationElement
    DGNPLATFORM_EXPORT SpatialLocationTypeCPtr GetSpatialLocationType() const;
};

//=======================================================================================
//! A SpatialLocationPortion represents an arbitrary portion of a larger SpatialLocationElement that will be broken down in more detail in a separate (sub) SpatialLocationModel.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpatialLocationPortion : SpatialLocationElement
{
    DEFINE_T_SUPER(SpatialLocationElement);
protected:
    explicit SpatialLocationPortion(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A 2-dimensional geometric element that is used to convey information within graphical presentations (like drawings).
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    02/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GraphicalElement2d : GeometricElement2d
{
    DEFINE_T_SUPER(GeometricElement2d);

protected:
    explicit GraphicalElement2d(CreateParams const& params) : T_Super(params) {}

public:
    //! Get the GraphicalType for this GraphicalElement2d
    //! @return Will be invalid if there is no GraphicalType associated with this GraphicalElement2d
    DGNPLATFORM_EXPORT GraphicalType2dCPtr GetGraphicalType() const;
};

//=======================================================================================
//! A 2-dimensional geometric element used to annotate drawings and sheets.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationElement2d : GraphicalElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_AnnotationElement2d, GraphicalElement2d)
    friend struct dgn_ElementHandler::Annotation2d;
public:
    //! Create a AnnotationElement2d from CreateParams.
    static AnnotationElement2dPtr Create(CreateParams const& params) {return new AnnotationElement2d(params);}
protected:
    AnnotationElement2dCP _ToAnnotationElement2d() const override final {return this;}

    explicit AnnotationElement2d(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A 2-dimensional graphical element used in drawings
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingGraphic : GraphicalElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DrawingGraphic, GraphicalElement2d)
    friend struct dgn_ElementHandler::DrawingGraphic;
protected:
    DrawingGraphicCP _ToDrawingGraphic() const override final {return this;}
    DGNPLATFORM_EXPORT Utf8String _GetInfoString(Utf8CP delimiter) const override;
    explicit DrawingGraphic(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a DrawingGraphic from CreateParams.
    static DrawingGraphicPtr Create(CreateParams const& params) {return new DrawingGraphic(params);}
    //! Create an instance of a DrawingGraphic by specifying the model and category
    DGNPLATFORM_EXPORT static DrawingGraphicPtr Create(GraphicalModel2dCR model, DgnCategoryId categoryId);

    //! Return the element that this DrawingGraphic represents (if it represents another element)
    DGNPLATFORM_EXPORT DgnElementCPtr GetRepresentedElement() const;
};

//=======================================================================================
//! Helper class for maintaining and querying the ElementGroupsMembers relationship
//! @see IElementGroup
//! @private
// @bsiclass                                                    Shaun.Sewall    10/15
//=======================================================================================
struct ElementGroupsMembers : NonCopyableClass
{
public:
    DGNPLATFORM_EXPORT static DgnDbStatus Insert(DgnElementCR group, DgnElementCR member, int priority);
    DGNPLATFORM_EXPORT static DgnDbStatus Delete(DgnElementCR group, DgnElementCR member);
    DGNPLATFORM_EXPORT static bool HasMember(DgnElementCR group, DgnElementCR member);
    DGNPLATFORM_EXPORT static DgnElementIdSet QueryMembers(DgnElementCR group);
    DGNPLATFORM_EXPORT static DgnElementIdSet QueryGroups(DgnElementCR member);
    DGNPLATFORM_EXPORT static int QueryMemberPriority(DgnElementCR group, DgnElementCR member);
};

//=======================================================================================
//! Base interface to query a group (element) that has other elements as members
//! @see IElementGroupOf
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    11/15
//=======================================================================================
struct IElementGroup
{
protected:
    //! Override to return the <em>this</em> pointer of the group DgnElement
    virtual DgnElementCP _ToGroupElement() const = 0;

public:
    //! Query for the members of this group
    DgnElementIdSet QueryMembers() const {return ElementGroupsMembers::QueryMembers(*_ToGroupElement());}

    //! Returns true if this group has the specified member
    bool HasMemberElement(DgnElementCR member) const {return ElementGroupsMembers::HasMember(*_ToGroupElement(), member);}

    //! Query for the priority of the specified member within this group
    //! @return the priority or -1 in case of an error
    int QueryMemberPriority(DgnElementCR member) const {return ElementGroupsMembers::QueryMemberPriority(*_ToGroupElement(), member);}
};

//=======================================================================================
//! Templated class used for an element to group other member elements and manage the
//! members of the group in a type-safe way.
//! @note Template type T must be a subclass of DgnElement.
//! @note The class that implements this interface must also be an element.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    10/15
//=======================================================================================
template<class T> class IElementGroupOf : public IElementGroup
{
protected:
    //! Called prior to member being added to group
    virtual DgnDbStatus _OnMemberAdd(T const& member) const {return DgnDbStatus::Success;}

    //! Called after member is added to group
    virtual void _OnMemberAdded(T const& member) const {}

    //! Called prior to member being removed from group
    virtual DgnDbStatus _OnMemberRemove(T const& member) const {return DgnDbStatus::Success;}
    //! Called after member removed from group
    virtual void _OnMemberRemoved(T const& member) const {}

    IElementGroupOf()
        {
        static_assert(std::is_base_of<DgnElement, T>::value, "IElementGroupOf can only group subclasses of DgnElement");
        }

public:
    //! Add a member to this group
    DgnDbStatus AddMember(T const& member, int priority=0) const
        {
        DgnElementCR groupElement = *_ToGroupElement();
        DgnElementCR memberElement = static_cast<DgnElementCR>(member); // see static_assert in constructor

        DgnDbStatus status = _OnMemberAdd(member);
        if (DgnDbStatus::Success != status)
            return status;

        status = ElementGroupsMembers::Insert(groupElement, memberElement, priority);
        if (DgnDbStatus::Success != status)
            return status;

        _OnMemberAdded(member);
        return DgnDbStatus::Success;
        }

    //! Remove a member from this group
    DgnDbStatus RemoveMember(T const& member) const
        {
        DgnElementCR groupElement = *_ToGroupElement();
        DgnElementCR memberElement = static_cast<DgnElementCR>(member); // see static_assert in constructor

        DgnDbStatus status = _OnMemberRemove(member);
        if (DgnDbStatus::Success != status)
            return status;

        status = ElementGroupsMembers::Delete(groupElement, memberElement);
        if (DgnDbStatus::Success != status)
            return status;

        _OnMemberRemoved(member);
        return DgnDbStatus::Success;
        }
    
    //! Returns true if this group has the specified member
    bool HasMember(T const& member) const
        {
        DgnElementCR memberElement = static_cast<DgnElementCR>(member); // see static_assert in constructor
        return HasMemberElement(memberElement);
        }

    //! Query for the groups that the specified element is a member of
    static DgnElementIdSet QueryGroups(T const& member)
        {
        DgnElementCR memberElement = static_cast<DgnElementCR>(member); // see static_assert in constructor
        return ElementGroupsMembers::QueryGroups(memberElement);
        }
};

//=======================================================================================
//! An InformationContentElement identifies and names information content.
//! @see InformationCarrierElement
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE InformationContentElement : DgnElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_InformationContentElement, DgnElement);
    friend struct dgn_ElementHandler::InformationContent;

protected:
    InformationContentElementCP _ToInformationContentElement() const override final {return this;}
    explicit InformationContentElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A Document is an InformationContentElement that identifies the content of a document.
//! The realized form of a document is called a DocumentCarrier (different class than Document). 
//! For example, a will is a legal document.  The will published into a PDF file is an ElectronicDocumentCopy.
//! The will printed onto paper is a PrintedDocumentCopy.
//! In this example, the Document only identifies, names, and tracks the content of the will.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Document : InformationContentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_Document, InformationContentElement)
    friend struct dgn_ElementHandler::Document;

protected:
    DocumentCP _ToDocumentElement() const override final {return this;}
    explicit Document(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Drawing : Document
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_Drawing, Document)
    friend struct dgn_ElementHandler::Drawing;

protected:
    explicit Drawing(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a DgnCode for a Drawing in the specified DocumentListModel
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DocumentListModelCR model, Utf8StringCR name);
    //! Create a unique DgnCode for a Drawing within the specified DocumentListModel
    //! @param[in] model The uniqueness scope for the DgnCode
    //! @param[in] baseName The base name for the CodeValue. A suffix will be appended (if necessary) to make it unique within the specified scope.
    //! @private
    DGNPLATFORM_EXPORT static DgnCode CreateUniqueCode(DocumentListModelCR model, Utf8CP baseName);

    //! Creates a new Drawing in the specified DocumentListModel
    //! @param[in] model Create the Drawing element in this DocumentListModel
    //! @param[in] name This name will be used to form the Drawing element's DgnCode
    DGNPLATFORM_EXPORT static DrawingPtr Create(DocumentListModelCR model, Utf8StringCR name);
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SectionDrawing: Drawing
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_SectionDrawing, Drawing)
    friend struct dgn_ElementHandler::SectionDrawing;

protected:
    explicit SectionDrawing(CreateParams const& params) : T_Super(params) {}

public:
    //! Creates a new SectionDrawing in the specified DocumentListModel
    //! @param[in] model Create the SectionDrawing element in this DocumentListModel
    //! @param[in] name This name will be used to form the SectionDrawing element's DgnCode
    DGNPLATFORM_EXPORT static SectionDrawingPtr Create(DocumentListModelCR model, Utf8StringCR name);
};

//=======================================================================================
//! An InformationCarrierElement is a proxy for an information carrier in the physical world.  
//! For example, the arrangement of ink on a paper document or an electronic file is an information carrier.
//! The content is tracked separately from the carrier.
//! @see InformationContentElement
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE InformationCarrierElement : DgnElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_InformationCarrierElement, DgnElement);
    friend struct dgn_ElementHandler::InformationCarrier;

protected:
    explicit InformationCarrierElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! An information element whose main purpose is to hold an information record.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE InformationRecordElement : InformationContentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_InformationRecordElement, InformationContentElement);
    friend struct dgn_ElementHandler::InformationRecord;

protected:
    explicit InformationRecordElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! Element used in conjunction with bis:ElementDrivesElement relationships to bundle multiple inputs before driving the output element.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DriverBundleElement : InformationContentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DriverBundleElement, InformationContentElement);
    friend struct dgn_ElementHandler::DriverBundle;

protected:
    explicit DriverBundleElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A DefinitionElement resides in (and only in) a DefinitionModel.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DefinitionElement : InformationContentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DefinitionElement, InformationContentElement);
    friend struct dgn_ElementHandler::Definition;

    BE_PROP_NAME(IsPrivate)

    bool m_isPrivate = false;

    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR) override;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    DefinitionElementCP _ToDefinitionElement() const override final {return this;}
public:
    explicit DefinitionElement(CreateParams const& params) : T_Super(params) {}

    BE_JSON_NAME(isPrivate)
    bool IsPrivate() const {return m_isPrivate;} //!< Test if this definition is private (should not be listed in the GUI, for example)
    void SetIsPrivate(bool isPrivate) {m_isPrivate = isPrivate;} //!< Specify that this definition is private (should not appear in the GUI, for example)

    //! Return the DefinitionModel that contains (or will contain) this DefinitionElement
    DGNPLATFORM_EXPORT DefinitionModelPtr GetDefinitionModel() const;
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypeDefinitionElement : DefinitionElement
{
    DEFINE_T_SUPER(DefinitionElement);

private:
    BE_PROP_NAME(Recipe)

protected:
    explicit TypeDefinitionElement(CreateParams const& params) : T_Super(params) {}

public:
    //! Set the recipe for this TypeDefinitionElement
    //! @param[in] recipeId The DgnElementId of the recipe to be associated with this TypeDefinitionElement
    //! @param[in] relClassId The ECClassId of the ECRelationshipClass that must be a subclass of TypeDefinitionHasRecipe
    DgnDbStatus SetRecipe(DgnElementId recipeId, ECN::ECClassId relClassId) {return SetPropertyValue(prop_Recipe(), recipeId, relClassId);}

    //! Get the DgnElementId of the recipe for this TypeDefinitionElement
    //! @return Will be invalid if there is no recipe associated with this TypeDefinitionElement
    DgnElementId GetRecipeId() const {return GetPropertyValueId<DgnElementId>(prop_Recipe());}

    //! Get the RecipeDefinitionElement for this TypeDefinitionElement
    //! @return Will be invalid if there is no RecipeDefinitionElement associated with this TypeDefinitionElement
    DGNPLATFORM_EXPORT RecipeDefinitionElementCPtr GetRecipe() const;
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RecipeDefinitionElement : DefinitionElement
{
    DEFINE_T_SUPER(DefinitionElement);

protected:
    virtual TemplateRecipe3dCP _ToTemplateRecipe3d() const {return nullptr;}
    virtual TemplateRecipe2dCP _ToTemplateRecipe2d() const {return nullptr;}
    explicit RecipeDefinitionElement(CreateParams const& params) : T_Super(params) {}

public:
    TemplateRecipe3dCP ToTemplateRecipe3d() const {return _ToTemplateRecipe3d();} //!< more efficient substitute for dynamic_cast<TemplateRecipe3dCP>(el)
    TemplateRecipe2dCP ToTemplateRecipe2d() const {return _ToTemplateRecipe2d();} //!< more efficient substitute for dynamic_cast<TemplateRecipe2dCP>(el)
};

//=======================================================================================
//! A PhysicalType typically corresponds to a @em type of physical object that can be ordered from a catalog.
//! The PhysicalType system is also a database normalization strategy because properties that are the same
//! across all instances are stored with the PhysicalType versus being repeated per PhysicalElement instance.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    08/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalType : TypeDefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_PhysicalType, TypeDefinitionElement)
    friend struct dgn_ElementHandler::PhysicalType;

protected:
    explicit PhysicalType(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a DgnCode for a PhysicalType element within the scope of the specified model
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DefinitionModelCR, Utf8StringCR);
};

//=======================================================================================
//! The SpatialLocationType system is a database normalization strategy because properties that are the same
//! across all instances are stored with the SpatialLocationType versus being repeated per SpatialLocationElement instance.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    08/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpatialLocationType : TypeDefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_SpatialLocationType, TypeDefinitionElement)
    friend struct dgn_ElementHandler::SpatialLocationType;

protected:
    explicit SpatialLocationType(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a DgnCode for a SpatialLocationType element within the scope of the specified model
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DefinitionModelCR, Utf8StringCR);
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TemplateRecipe3d : RecipeDefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_TemplateRecipe3d, RecipeDefinitionElement)
    friend struct dgn_ElementHandler::TemplateRecipe3d;

protected:
    TemplateRecipe3dCP _ToTemplateRecipe3d() const override {return this;}
    explicit TemplateRecipe3d(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a DgnCode for a TemplateRecipe3d element within the scope of the specified model
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DefinitionModelCR, Utf8StringCR);

    //! Create a TemplateRecipe3d element of the specified name within the specified model
    DGNPLATFORM_EXPORT static TemplateRecipe3dPtr Create(DefinitionModelCR model, Utf8StringCR name);
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    08/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GraphicalType2d : TypeDefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_GraphicalType2d, TypeDefinitionElement)
    friend struct dgn_ElementHandler::GraphicalType2d;

protected:
    explicit GraphicalType2d(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a DgnCode for a GraphicalType2d element within the scope of the specified model
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DefinitionModelCR, Utf8StringCR);
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TemplateRecipe2d : RecipeDefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_TemplateRecipe2d, RecipeDefinitionElement)
    friend struct dgn_ElementHandler::TemplateRecipe2d;

protected:
    TemplateRecipe2dCP _ToTemplateRecipe2d() const override {return this;}
    explicit TemplateRecipe2d(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a DgnCode for a TemplateRecipe2d element within the scope of the specified model
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DefinitionModelCR model, Utf8StringCR name);

    //! Create a TemplateRecipe2d element of the specified name within the specified model
    DGNPLATFORM_EXPORT static TemplateRecipe2dPtr Create(DefinitionModelCR model, Utf8StringCR name);
};

//=======================================================================================
//! An InformationPartitionElement provides a starting point for a DgnModel hierarchy
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE InformationPartitionElement : InformationContentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_InformationPartitionElement, InformationContentElement);
    friend struct dgn_ElementHandler::InformationPartition;

private:
    BE_PROP_NAME(Description)

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override {return !codeSpec.IsNullCodeSpec();}
    DGNPLATFORM_EXPORT static DgnElement::CreateParams InitCreateParams(SubjectCR parentSubject, Utf8StringCR name, DgnDomain::Handler& handler);
    explicit InformationPartitionElement(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a DgnCode for an InformationPartitionElement with the specified Subject as its parent
    DGNPLATFORM_EXPORT static DgnCode CreateCode(SubjectCR parentSubject, Utf8StringCR name);
    //! Create a unique DgnCode for an InformationPartitionElement with the specified Subject as its parent
    //! @param[in] parentSubject The uniqueness scope for the DgnCode
    //! @param[in] baseName The base name for the CodeValue. A suffix will be appended (if necessary) to make it unique within the specified scope.
    //! @private
    DGNPLATFORM_EXPORT static DgnCode CreateUniqueCode(SubjectCR parentSubject, Utf8CP baseName);

    //! Get the description of this InformationPartitionElement
    Utf8String GetDescription() const {return GetPropertyValueString(prop_Description());}
    //! Set the description of this InformationPartitionElement
    void SetDescription(Utf8CP description) {SetPropertyValue(prop_Description(), description);}
};

//=======================================================================================
//! A DefinitionPartition provides a starting point for a DefinitionModel hierarchy
//! @note DefinitionPartition elements only reside in the RepositoryModel
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DefinitionPartition : InformationPartitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DefinitionPartition, InformationPartitionElement);
    friend struct dgn_ElementHandler::DefinitionPartition;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnSubModelInsert(DgnModelCR model) const override;
    explicit DefinitionPartition(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new DefinitionPartition
    //! @param[in] parentSubject The new DefinitionPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this DefinitionPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static DefinitionPartitionPtr Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
    //! Create and insert a new DefinitionPartition
    //! @param[in] parentSubject The new DefinitionPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this DefinitionPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static DefinitionPartitionCPtr CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
};

//=======================================================================================
//! A DocumentPartition provides a starting point for a DocumentListModel hierarchy
//! @note DocumentPartition elements only reside in the RepositoryModel
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DocumentPartition : InformationPartitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DocumentPartition, InformationPartitionElement);
    friend struct dgn_ElementHandler::DocumentPartition;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnSubModelInsert(DgnModelCR model) const override;
    explicit DocumentPartition(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new DocumentPartition
    //! @param[in] parentSubject The new DocumentPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this DocumentPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static DocumentPartitionPtr Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
    //! Create and insert a new DocumentPartition
    //! @param[in] parentSubject The new DocumentPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this DocumentPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static DocumentPartitionCPtr CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
};

//=======================================================================================
//! A GroupInformationPartition provides a starting point for a GroupInformationModel hierarchy
//! @note GroupInformationPartition elements only reside in the RepositoryModel
// @bsiclass                                                    Shaun.Sewall    10/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GroupInformationPartition : InformationPartitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_GroupInformationPartition, InformationPartitionElement);
    friend struct dgn_ElementHandler::GroupInformationPartition;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnSubModelInsert(DgnModelCR model) const override;
    explicit GroupInformationPartition(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new GroupInformationPartition
    //! @param[in] parentSubject The new GroupInformationPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this GroupInformationPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static GroupInformationPartitionPtr Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
    //! Create and insert a new GroupInformationPartition
    //! @param[in] parentSubject The new GroupInformationPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this GroupInformationPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static GroupInformationPartitionCPtr CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
};

//=======================================================================================
//! An InformationRecordPartition provides a starting point for a InformationRecordModel hierarchy
//! @note InformationRecordPartition elements only reside in the RepositoryModel
// @bsiclass                                                    Shaun.Sewall    03/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE InformationRecordPartition : InformationPartitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_InformationRecordPartition, InformationPartitionElement);
    friend struct dgn_ElementHandler::InformationRecordPartition;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnSubModelInsert(DgnModelCR model) const override;
    explicit InformationRecordPartition(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new InformationRecordPartition
    //! @param[in] parentSubject The new InformationRecordPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this InformationRecordPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static InformationRecordPartitionPtr Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
    //! Create and insert a new InformationRecordPartition
    //! @param[in] parentSubject The new InformationRecordPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this InformationRecordPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static InformationRecordPartitionCPtr CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
};

//=======================================================================================
//! A PhysicalPartition provides a starting point for a PhysicalModel hierarchy
//! @note PhysicalPartition elements only reside in the RepositoryModel
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalPartition : InformationPartitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_PhysicalPartition, InformationPartitionElement);
    friend struct dgn_ElementHandler::PhysicalPartition;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnSubModelInsert(DgnModelCR model) const override;
    explicit PhysicalPartition(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new PhysicalPartition
    //! @param[in] parentSubject The new PhysicalPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this PhysicalPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static PhysicalPartitionPtr Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
    //! Create and insert a new PhysicalPartition
    //! @param[in] parentSubject The new PhysicalPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this PhysicalPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static PhysicalPartitionCPtr CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
};

//=======================================================================================
//! A SpatialLocationPartition provides a starting point for a SpatialLocationModel hierarchy
//! @note SpatialLocationPartition elements only reside in the RepositoryModel
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpatialLocationPartition : InformationPartitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_SpatialLocationPartition, InformationPartitionElement);
    friend struct dgn_ElementHandler::SpatialLocationPartition;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnSubModelInsert(DgnModelCR model) const override;
    explicit SpatialLocationPartition(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new SpatialLocationPartition
    //! @param[in] parentSubject The new SpatialLocationPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this SpatialLocationPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static SpatialLocationPartitionPtr Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
    //! Create and insert a new SpatialLocationPartition
    //! @param[in] parentSubject The new SpatialLocationPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this SpatialLocationPartition
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static SpatialLocationPartitionCPtr CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
};

//=======================================================================================
//! An InformationCarrierElement is a proxy for an information carrier in the physical world.  
//! For example, the arrangement of ink on a paper document or an electronic file is an information carrier.
//! The content is tracked separately from the carrier.
//! @see InformationContentElement
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE InformationReferenceElement : InformationContentElement
{
    DEFINE_T_SUPER(InformationContentElement);
protected:
    explicit InformationReferenceElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A Subject resides in (and only in) a RepositoryModel.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Subject : InformationReferenceElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_Subject, InformationReferenceElement);
    friend struct dgn_ElementHandler::Subject;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnSubModelInsert(DgnModelCR model) const override;
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override {return !codeSpec.IsNullCodeSpec();}

    explicit Subject(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a DgnCode for a Subject with a specified parent Subject
    DGNPLATFORM_EXPORT static DgnCode CreateCode(SubjectCR parentSubject, Utf8StringCR name);

    //! Creates a new child Subject of the specified parent Subject
    //! @param parentSubject    The parent of the new Subject
    //! @param name             The name of the new Subject
    //! @param description      The description of the new Subject
    //! @return a new, non-persistent Subject element or an invalid value if any of the arguments are invalid
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static SubjectPtr Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);
    //! Creates a new child Subject of the specified parent Subject and inserts it into the DgnDb 
    //! @param parentSubject    The parent of the new Subject
    //! @param name             The name of the new Subject
    //! @param description      The description of the new Subject
    //! @return a new persistent Subject element or an invalid value if any of the arguments are invalid or the insert fails
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static SubjectCPtr CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description=nullptr);

    BE_PROP_NAME(Description)
    Utf8String GetDescription() const {return GetPropertyValueString(prop_Description());}
    void SetDescription(Utf8CP description) {SetPropertyValue(prop_Description(), description);}

    BE_JSON_NAME(Subject); //<! The namespace reserved for Subject Json properties
    BE_JSON_NAME(Job); //<! The sub-namespace reserved for Job Subject Json properties
    BE_JSON_NAME(Model); //<! The sub-namespace reserved for Model Subject Json properties
    //! Get Json properties
    ECN::AdHocJsonValueCR GetSubjectJsonProperties() const {return GetJsonProperties(json_Subject());}

    //! Get Json properties from a particular sub-namespace
    ECN::AdHocJsonValue GetSubjectJsonProperties(Utf8CP sns) const {return GetJsonProperties(json_Subject()).GetMember(sns);}

    //! Set Json properties
    void SetSubjectJsonProperties(JsonValueCR props) {SetJsonProperties(json_Subject(), props);}

    //! Set Json properties from a particular sub-namespace
    void SetSubjectJsonProperties(Utf8CP sns, JsonValueCR props) {m_jsonProperties.GetMemberR(json_Subject())[sns] = props;}
};

//=======================================================================================
//! Helper functions for working with Subject elements that are "job subjects"
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct JobSubjectUtils
{
    BE_JSON_NAME(Bridge); //<! The name reserved within the Job subspace for the required "Bridge" property. This is the registry subkey value used by the bridge.
    BE_JSON_NAME(Transform); //<! The name reserved within the Job subspace for the optional "Transform" property
    BE_JSON_NAME(Comments); //<! The name reserved within the Job subspace for the optional "Comments" property.
    BE_JSON_NAME(Properties); //<! The name reserved within the Job subspace for the optional "Properties" property. This is where bridge-specific properties go.

    //! Test to see if the specified subject is a @em job subject. See InitializeProperties to set up a subject element as a job subject.
    //! @param subj The job subject
    static bool IsJobSubject(SubjectCR subj) {return subj.GetSubjectJsonProperties().isMember(Subject::json_Job());}

    //! @name Primitive Property Access Functions
    //! @{

    //! Test if a specified job subject property exists
    //! @param subj The job subject
    //! @param propName The name of a job subject property
    static bool HasProperty(SubjectCR subj, Utf8CP propName) {BeAssert(IsJobSubject(subj)); return subj.GetSubjectJsonProperties(Subject::json_Job()).isMember(propName);}

    //! Get all of the job subject's properties at once
    //! @param subj The job subject
    //! @return the value of the job subject's properties
    static ECN::AdHocJsonValue GetProperties(SubjectCR subj) {BeAssert(IsJobSubject(subj)); return subj.GetSubjectJsonProperties(Subject::json_Job());}

    //! Get a job subject property
    //! @param subj The job subject
    //! @param propName The name of a job subject property
    //! @return the value of the job subject property
    static Json::Value GetProperty(SubjectCR subj, Utf8CP propName) {BeAssert(IsJobSubject(subj) && HasProperty(subj, propName)); return subj.GetSubjectJsonProperties(Subject::json_Job())[propName];}

    //! Set all of the job subject properties at once. subj must already be a job subject element. Call InitializeProperties to set up a subject as a job subject.
    //! @param subj The job subject
    //! @param value The properties
    static void SetProperties(SubjectR subj, JsonValueCR value) {BeAssert(IsJobSubject(subj)); subj.SetSubjectJsonProperties(Subject::json_Job(), value);}

    //! Set a job subject property
    //! @param subj The job subject
    //! @param propName The name of a job subject property
    //! @param value The property name
    static void SetProperty(SubjectR subj, Utf8CP propName, JsonValueCR value) {auto props = GetProperties(subj); props[propName] = value; SetProperties(subj, props);}

    //! @}

    //! @name High-level JobSubject Property Access Functions
    //! @{

    //! Initialize the job's properties. This is a convenience method that allows the caller to set required and some of the option job subject properties with a single function call.
    //! Any and all previously existing job properties are removed.
    //! @param[in] jobSubject An editable copy of the job subject
    //! @param[in] bridgeRegSubKey the registry subkey identifier used by the bridge.
    //! @param[in] comments Optional comments
    //! @param[in] properties Optional bridge-specific properties
    DGNPLATFORM_EXPORT static void InitializeProperties(SubjectR jobSubject, Utf8StringCR bridgeRegSubKey, Utf8CP comments = nullptr, JsonValueCP properties = nullptr);

    //! Get the job's Bridge property. This is the registry subkey value used by the bridge.
    //! @param[in] jobSubject The job subject
    //! @return non-zero error status if the job subject does not have a Bridge.
    static Utf8String GetBridge(SubjectCR jobSubject) {auto v = GetProperty(jobSubject, json_Bridge()); return v.isString()? v.asCString(): "";}

    //! Get the transform that should be applied to all spatial data converted by the job.
    //! @note The transform should be @em pre-multiplied to any GCS transform that the bridge has already calculated.
    //! @param[out] trans Transform to apply to spatial data.
    //! @param[in] jobSubject The job subject
    //! @return non-zero error status if the job subject does not have a transform.
    DGNPLATFORM_EXPORT static BentleyStatus GetTransform(TransformR trans, SubjectCR jobSubject);

    //! Get the transform that should be applied to all spatial data converted by the job.
    //! @note The transform will be @em pre-multiplied to any GCS transform that the bridge has already calculated.
    //! @param[in] jobSubject An editable copy of the job subject
    //! @param[in] trans Transform to apply to spatial data.
    DGNPLATFORM_EXPORT static void SetTransform(SubjectR jobSubject, TransformCR trans);

    //! @}

};

//=======================================================================================
//! A GroupInformationElement resides in (and only in) a GroupInformationModel.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GroupInformationElement : InformationReferenceElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_GroupInformationElement, InformationReferenceElement);
    friend struct dgn_ElementHandler::GroupInformation;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    explicit GroupInformationElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! Abstract base class for roles played by other (typically physical) elements.
//! For example:
//! - <i>Lawyer</i> and <i>employee</i> are potential roles of a person
//! - <i>Asset</i> and <i>safety hazard</i> are potential roles of a PhysicalElement
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    05/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoleElement : DgnElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_RoleElement, DgnElement)
    friend struct dgn_ElementHandler::Role;

protected:
    RoleElementCP _ToRoleElement() const override final {return this;}
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    explicit RoleElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! The DgnElements for a DgnDb.
//! This class holds a cache of reference-counted DgnElements. All in-memory DgnElements for a DgnDb are held in its DgnElements member.
//! When the reference count of an element goes to zero, it is not immediately freed. Instead, it is held by this class
//! and may be "reclaimed" later if/when it is needed again. The memory held by DgnElements is not actually freed until
//! their reference count goes to 0 and the cache is subsequently purged.
//! @see DgnDb::Elements
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct DgnElements : DgnDbTable
{
    friend struct DgnDb;
    friend struct DgnElement;
    friend struct DgnModel;
    friend struct DgnModels;
    friend struct DgnGeometryPart;
    friend struct ElementHandler;
    friend struct TxnManager;
    friend struct dgn_TxnTable::Element;
    friend struct GeometricElement;
    friend struct ElementAutoHandledPropertiesECInstanceAdapter;

private:
    // THIS MUST NOT BE EXPORTED, AS IT BYPASSES THE ECCRUDWRITETOKEN
    struct AutoHandledPropertyUpdaterCache : ECInstanceUpdaterCache
        {
        void _GetPropertiesToBind(bvector<ECN::ECPropertyCP>&, DgnDbR, ECN::ECClassCR) override;
        };

    struct ElementSelectStatement
    {
        BeSQLite::EC::CachedECSqlStatementPtr m_statement;
        ECSqlClassParamsCR m_params;
        ElementSelectStatement(BeSQLite::EC::CachedECSqlStatement* stmt, ECSqlClassParamsCR params) : m_statement(stmt), m_params(params) {}
    };
    typedef bmap<DgnClassId, ECSqlClassInfo> ClassInfoMap;
    typedef bmap<DgnClassId, ECSqlClassParams> T_ClassParamsMap;
    std::unique_ptr<struct ElementMRU> m_mruCache;
    uint64_t m_extant = 0;
    BeSQLite::StatementCache m_stmts;
    Byte m_snappyFromBuffer[BeSQLite::SnappyReader::SNAPPY_UNCOMPRESSED_BUFFER_SIZE];
    BeSQLite::SnappyFromMemory m_snappyFrom;
    BeSQLite::SnappyToBlob m_snappyTo;
    DgnElementIdSet m_undisplayedSet;
    mutable BeMutex m_mutex;
    mutable ClassInfoMap m_classInfos;      // information about custom-handled properties 
    mutable T_ClassParamsMap m_classParams; // information about custom-handled properties 
    mutable AutoHandledPropertyUpdaterCache m_updaterCache;
    mutable std::map<uint64_t, std::unique_ptr<BeSQLite::EC::JsonECSqlSelectAdapter>> m_jsonSelectAdapterCache;

    void Destroy();
    void AddToPool(DgnElementCR) const;
    void FinishUpdate(DgnElementCR replacement, DgnElementCR original);
    DgnElementCPtr LoadElement(DgnElement::CreateParams const& params, Utf8CP jsonProps, bool makePersistent) const;
    DgnElementCPtr LoadElement(DgnElementId elementId, bool makePersistent) const;
    DgnElementCPtr PerformInsert(DgnElementR element, DgnDbStatus&);
    DgnDbStatus PerformDelete(DgnElementCR);
    explicit DgnElements(DgnDbR db);
    ~DgnElements();

    DGNPLATFORM_EXPORT DgnElementCPtr InsertElement(DgnElementR element, DgnDbStatus* stat);
    DGNPLATFORM_EXPORT DgnElementCPtr UpdateElement(DgnElementR element, DgnDbStatus* stat);

    ElementSelectStatement GetPreparedSelectStatement(DgnElementR el) const;
    BeSQLite::EC::CachedECSqlStatementPtr GetPreparedInsertStatement(DgnElementR el) const;
    BeSQLite::EC::CachedECSqlStatementPtr GetPreparedUpdateStatement(DgnElementR el) const;

    BeSQLite::SnappyToBlob& GetSnappyTo() {return m_snappyTo;} // NB: Not to be used during insert or update of a GeometricElement or GeometryPart!

    ECSqlClassParams const& GetECSqlClassParams(DgnClassId) const;

    // *** WIP_SCHEMA_IMPORT - temporary work-around needed because ECClass objects are deleted when a schema is imported
    void ClearECCaches();
public:
    DGNPLATFORM_EXPORT BeSQLite::SnappyFromMemory& GetSnappyFrom() {return m_snappyFrom;} // NB: Not to be used during loading of a GeometricElement or GeometryPart!

    BeMutex& GetMutex() {return m_mutex;}

    //! @private
    Utf8StringCR GetSelectEcPropsECSql(ECSqlClassInfo&, ECN::ECClassCR) const;
    //! @private
    DGNPLATFORM_EXPORT Utf8StringCR GetAutoHandledPropertiesSelectECSql(ECN::ECClassCR ecclass) const;
    //! @private
    ECSqlClassInfo& FindClassInfo(DgnElementCR el) const;
    //! @private
    ECSqlClassInfo& FindClassInfo(DgnClassId classId) const;
    
    //! @private
    BeSQLite::EC::JsonECSqlSelectAdapter const& GetJsonSelectAdapter(BeSQLite::EC::ECSqlStatement const& stmt) const;

    DGNPLATFORM_EXPORT BeSQLite::CachedStatementPtr GetStatement(Utf8CP sql) const; //!< Get a statement from the element-specific statement cache for this DgnDb @private
    DGNPLATFORM_EXPORT void DropFromPool(DgnElementCR) const; //!< @private
    DgnDbStatus LoadGeometryStream(GeometryStreamR geom, void const* blob, int blobSize); //!< @private

    //! Look up an element in the pool of loaded elements for this DgnDb.
    //! @return A pointer to the element, or nullptr if the is not in the pool.
    //! @note This method is rarely needed. You should almost always use GetElement. It will return nullptr if the element is not currently loaded. That does not mean the element doesn't exist in the database.
    //! @private
    DGNPLATFORM_EXPORT DgnElementCP FindLoadedElement(DgnElementId id) const;

    //! Query the DgnModelId of the model that contains the specified element.
    DGNPLATFORM_EXPORT DgnModelId QueryModelId(DgnElementId elementId) const;

    //! @private Allow Navigator to try to resolve URIs created in this version and in Graphite05 for things like issues and clashes.
    //! @param uri The encoded URI that was created by CreateElementUri or by a previous version of Graphite.
    //! @return The ID of the element identified by the URI or an invalid ID if no element in this Db matches the URI's query parameters.
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdByURI(Utf8CP uri) const;

    //! @private Allow Navigator to create a URI that can be stored outside of the Db and resolved later.
    //! @param[out] uriStr  The encoded URI
    //! @param[in] el       The element that is to be the target of the URI
    //! @param[in] fallBackOnV8Id   If  true, V8 provenance is used as a fallback if the element does not have a code
    //! @param[in] fallBackOnDgnDbId   If  true, the element's DgnDb element ID is used as a fallback if the element does not have a code or provenance. This is not recommended if the Db will be re-created by a publisher.
    //! @return non-zero error status if the URI could not be created, because it has neither a Code nor V8 provenance and IDs are not an acceptable fallback.
    DGNPLATFORM_EXPORT BentleyStatus CreateElementUri(Utf8StringR uriStr, DgnElementCR el, bool fallBackOnV8Id, bool fallBackOnDgnDbId=false) const;

    //! Query for the DgnElementId of the element that has the specified code
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdByCode(DgnCodeCR code) const;

    //! Query for the DgnElementId of the element that has the specified code
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdByCode(CodeSpecId codeSpecId, DgnElementId codeScopeElementId, Utf8StringCR codeValue) const;

    //! Query for the DgnElementId of the element that has the specified code
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdByCode(Utf8CP codeSpecName, DgnElementId codeScopeElementId, Utf8StringCR codeValue) const;

    //! Create a new, non-persistent element from the supplied ECInstance.
    //! The supplied instance must specify the element's ModelId and Code. It does not have to specify the ElementId/ECInstaceId. Typically, it will not.
    //! @param properties The instance that contains all of the element's business properties
    //! @param stat  Optional. If not null, an error status is returned here if the element cannot be created.
    //! @return a new, non-persistent element if successfull, or an invalid ptr if not.
    //! @note The returned element, if any, is non-persistent. The caller must call the element's Insert method to add it to the bim.
    DGNPLATFORM_EXPORT DgnElementPtr CreateElement(ECN::IECInstanceCR properties, DgnDbStatus* stat = nullptr) const;
    DGNPLATFORM_EXPORT DgnElementPtr CreateElement(ECN::IECInstanceCR properties, bool ignoreUnknownProperties, DgnDbStatus* stat = nullptr) const;

    template<class T> RefCountedPtr<T> Create(ECN::IECInstanceCR properties, DgnDbStatus* stat=nullptr) const {return dynamic_cast<T*>(CreateElement(properties,stat).get());}

    //! Get a DgnElement from this DgnDb by its DgnElementId.
    //! @remarks The element is loaded from the database if necessary.
    //! @return Invalid if the element does not exist.
    DGNPLATFORM_EXPORT DgnElementCPtr GetElement(DgnElementId id) const;

    //! Get a DgnElement by its DgnElementId, and dynamic_cast the result to a specific subclass of DgnElement.
    //! This is merely a templated shortcut to dynamic_cast the return of #GetElement to a subclass of DgnElement.
    template<class T> RefCountedCPtr<T> Get(DgnElementId id) const {return dynamic_cast<T const*>(GetElement(id).get());}

    //! Get an editable copy of an element by DgnElementId.
    //! @return Invalid if the element does not exist, or if it cannot be edited.
    template<class T> RefCountedPtr<T> GetForEdit(DgnElementId id) const {RefCountedCPtr<T> orig=Get<T>(id); return orig.IsValid() ? (T*)orig->CopyForEdit().get() : nullptr;}

    //! Query for a DgnElement in this DgnDb by its FederationGuid. The element is loaded from the database if necessary.
    //! @note It is always more efficient to find elements by their DgnElementId if it is available.
    //! @return Invalid if the element is not found.
    //! @see GetElement
    //! @see DgnElement::CopyForEdit
    DGNPLATFORM_EXPORT DgnElementCPtr QueryElementByFederationGuid(BeSQLite::BeGuidCR federationGuid) const;

    //! argument for MakeIterator
    enum class PolymorphicQuery : bool {No = false, Yes = true};

    //! Make an iterator over elements of the specified ECClass in this DgnDb.
    //! @param[in] className The <i>full</i> ECClass name of the element class.  For example: BIS_SCHEMA(BIS_CLASS_PhysicalElement)
    //! @param[in] whereClause The optional where clause starting with WHERE
    //! @param[in] orderByClause The optional order by clause starting with ORDER BY
    //! @param[in] polymorphic If false only specified class is returned. The default is true which also returns all derived classes.
    DGNPLATFORM_EXPORT ElementIterator MakeIterator(Utf8CP className, Utf8CP whereClause=nullptr, Utf8CP orderByClause=nullptr, PolymorphicQuery polymorphic=PolymorphicQuery::Yes) const;

    //! Make an iterator over ElementAspects of the specified ECClass in this DgnDb.
    //! @param[in] className The <i>full</i> ECClass name of the aspect class.  For example: BIS_SCHEMA(BIS_CLASS_ElementMultiAspect)
    //! @param[in] whereClause The optional where clause starting with WHERE
    //! @param[in] orderByClause The optional order by clause starting with ORDER BY
    //! @see DgnElement::MakeAspectIterator
    DGNPLATFORM_EXPORT ElementAspectIterator MakeAspectIterator(Utf8CP className, Utf8CP whereClause=nullptr, Utf8CP orderByClause=nullptr) const;

    //! Return the DgnElementId for the root Subject
    DgnElementId GetRootSubjectId() const {return DgnElementId((uint64_t)1LL);}
    //! Return the root Subject
    SubjectCPtr GetRootSubject() const {return Get<Subject>(GetRootSubjectId());}

    //! Get the DgnElementId of the partition that lists the RealityData source for @b this DgnDb
    DgnElementId GetRealityDataSourcesPartitionId() const {return DgnElementId((uint64_t)14LL);}
    //! Get the DgnElementId of the Dictionary partition for @b this DgnDb
    DgnElementId GetDictionaryPartitionId() const {return DgnElementId((uint64_t)16LL);}

    //! Insert a copy of the supplied DgnElement into this DgnDb.
    //! @param[in] element The DgnElement to insert.
    //! @param[in] stat An optional status value. Will be DgnDbStatus::Success if the insert was successful, error status otherwise.
    //! @return RefCountedCPtr to the newly persisted /b copy of /c element. Will be invalid if the insert failed.
    //! @note The element's code must be unique among all elements within the DgnDb, or this method will fail with DgnDbStatus::DuplicateCode.
    //! @note This function can only be safely invoked from the client thread.
    template<class T> RefCountedCPtr<T> Insert(T& element, DgnDbStatus* stat=nullptr) {return (T const*) InsertElement(element, stat).get();}

    //! Update the original persistent DgnElement from which the supplied DgnElement was copied.
    //! @param[in] modifiedElement The modified copy of the DgnElement to update.
    //! @param[in] stat An optional status value. Will be DgnDbStatus::Success if the update was successful, error status otherwise.
    //! @return RefCountedCPtr to the modified persistent element. Will be invalid if the update failed.
    //! @note This call returns a RefCountedCPtr to the *original* peristent element (which has now been updated to reflect the changes from
    //! modifiedElement). modifiedElement does *not* become persistent from this call.
    //! @note This function can only be safely invoked from the client thread.
    template<class T> RefCountedCPtr<T> Update(T& modifiedElement, DgnDbStatus* stat=nullptr) {return (T const*) UpdateElement(modifiedElement, stat).get();}

    //! Delete a DgnElement from this DgnDb.
    //! @param[in] element The element to delete.
    //! @return DgnDbStatus::Success if the element was deleted, error status otherwise.
    //! @note This function can only be safely invoked from the client thread.
    DGNPLATFORM_EXPORT DgnDbStatus Delete(DgnElementCR element);

    //! Delete a DgnElement from this DgnDb by DgnElementId.
    //! @return DgnDbStatus::Success if the element was deleted, error status otherwise.
    //! @note This method is merely a shortcut to #GetElement and then #Delete
    DgnDbStatus Delete(DgnElementId id) {auto el=GetElement(id); return el.IsValid() ? Delete(*el) : DgnDbStatus::NotFound;}

    DgnElementIdSet const& GetUndisplayedSet() const {return m_undisplayedSet;} //!< @private
    bool IsUndisplayed(DgnElementId) const;
    void SetUndisplayed(DgnElementId, bool isUndisplayed); //!< @private

    //! Set the maximum number of elements to be held by the "Most Recentley Used" element cache for this DgnDb. 
    //! @param newMax The maximum number of elements to be held in the element MRU cache. After this many elements are in memory,
    //! the least recently used element is discarded. Set to 0 to disable MRU cache.
    //! @note If there are currently more than newMax elements in memory, the oldest ones are removed until the size is newMax.
    DGNPLATFORM_EXPORT void SetCacheSize(uint32_t newMax);

    //! Empty the Most Recentley Used element cache for this DgnDb.
    DGNPLATFORM_EXPORT void ClearCache();
};

//=======================================================================================
//! The basic element copier. Makes a persistent copy of elements and their children.
// @bsiclass                                                BentleySystems
//=======================================================================================
struct ElementCopier
{
protected:
    DgnCloneContext& m_context;
    bool m_copyChildren;
    bool m_copyGroups;

public:
    DGNPLATFORM_EXPORT ElementCopier(DgnCloneContext& c);

    DgnCloneContext& GetCloneContext() {return m_context;}

    //! Specify if children should be deep-copied or not. The default is yes, deep-copy children.
    void SetCopyChildren(bool b) {m_copyChildren=b;}

    //! Specify if group members should be deep-copied or not. The default is no, do not deep-copy group members.
    void SetCopyGroups(bool b) {m_copyGroups=b;}

    //! Make a persistent copy of a specified Physical element and its children.
    //! This function copies the input element's children, unless you call SetCopyChildren and pass false.
    //! If the input element is a group, this function will optionally copy its group members. See SetCopyGroups.
    //! When copying children, this function will either copy a child into its own model or its parent's model. See SetPreserveOriginalModels.
    //! The same strategy is used to choose the destination model of group members.
    //! @param[out] stat        Optional. If not null, then an error code is stored here in case the copy fails.
    //! @param[in] targetModel  The model where the instance is to be inserted
    //! @param[in] sourceElement The element that is to be copied
    //! @param[in] code         The code to assign to the new element. If invalid, then a code will be generated by the sourceElement's CodeSpec
    //! @param[in] newParentId  Optional. The element that should be the parent of the new element. If not specified, then the parent of the new element
    //!                             will either be the parent of the source element or the element to which the source parent has been remapped. See DgnCloneContext.
    //! @return a new element if successful
    DGNPLATFORM_EXPORT DgnElementCPtr MakeCopy(DgnDbStatus* stat, DgnModelR targetModel, DgnElementCR sourceElement, DgnCode const& code, DgnElementId newParentId = DgnElementId());
};

//=======================================================================================
//! Utility methods for working with geometric element assemblies.
// @bsiclass                                                BentleySystems
//=======================================================================================
struct ElementAssemblyUtil
{
    //! Get the parent DgnElementId of the assembly for which the input DgnElement is a member.
    //! @param[in] el  The element selected
    //! @param[in] topMost Return top most assembly parent.
    //! @return DgnElementId of parent. Will be invalid if there is no parent.
    DGNPLATFORM_EXPORT static DgnElementId GetAssemblyParentId(DgnElementCR el, bool topMost=false);

    //! Query the DgnDb for members of the assembly for which the input DgnElement is a member.
    //! @return DgnElementIdSet containing the DgnElementIds of assembly elements. Will be empty if not an assembly.
    DGNPLATFORM_EXPORT static DgnElementIdSet GetAssemblyElementIdSet(DgnElementCR el);

    //! Load child elements into assemblIdSet.
    DGNPLATFORM_EXPORT static void GetChildElementIdSet(DgnElementIdSet& assemblyIdSet, DgnElementId parentId, DgnDbR dgnDb);
};

//=======================================================================================
//! Utility to collect editable elements.
//! Order is \em not preserved.
//! The collection holds only one copy of an element with a given ElementId.
// @bsiclass                                                BentleySystems
//=======================================================================================
struct DgnEditElementCollector
{
protected:
     bvector<DgnElementP> m_elements; // The editable elements in the set. We manage their refcounts as we add and remove them 
     bmap<DgnElementId,DgnElementP> m_ids; // The Elements in the set that have IDs. Child elements will always have IDs. Some top-level elements may not have an Id.

     DGNPLATFORM_EXPORT void EmptyAll();
     DGNPLATFORM_EXPORT void CopyFrom(DgnEditElementCollector const&);

public:
    //! Construct an empty collection
    DgnEditElementCollector() {}

    //! Create a copy of a collection of elements. Note that the new collection will have its own copies of the elements.
    DgnEditElementCollector(DgnEditElementCollector const& rhs) {CopyFrom(rhs);}

    //! Create a copy of a collection of elements. Note that the new collection will have its own copies of the elements.
    DGNPLATFORM_EXPORT DgnEditElementCollector& operator=(DgnEditElementCollector const&);

    //! Destroy this collection
    ~DgnEditElementCollector() {EmptyAll();}

    DGNPLATFORM_EXPORT void Dump(Utf8StringR str, DgnElement::ComparePropertyFilter const&) const;
    DGNPLATFORM_EXPORT void DumpTwo(Utf8StringR str, DgnEditElementCollector const& other, DgnElement::ComparePropertyFilter const&) const;

    //! See if this collection and \a other have the equivalent set of elements
    //! @param other    The other collection
    //! @param filter   The properties to exclude from the comparison.
    //! @return true if the collections are equivalent
    DGNPLATFORM_EXPORT bool Equals(DgnEditElementCollector const& other, DgnElement::ComparePropertyFilter const& filter) const;

    //! Add the specified editable copy of an element to the collection. 
    //! @param el  The editable copy to be added
    //! @return The element that is in the collection. 
    DGNPLATFORM_EXPORT DgnElementPtr AddElement(DgnElementR el);

    //! Add editable copies of the children of an element to the collection. 
    //! @param el  The parent element to be queried
    //! @param maxDepth The levels of child elements to add. Pass 1 to add only the immediate children.
    DGNPLATFORM_EXPORT void AddChildren(DgnElementCR el, size_t maxDepth = std::numeric_limits<size_t>::max());
    
    //! Add an element and editable copies of its children to the collection
    //! @param el       The parent element to be added and queried
    //! @param maxDepth The levels of child elements to add. Pass 1 to add only the immediate children.
    void AddAssembly(DgnElement& el, size_t maxDepth = std::numeric_limits<size_t>::max()) {AddElement(el); AddChildren(el, maxDepth);}

    //! Add an editable copy of the specified element to the collection.
    //! If the collection already contains an element with the same ElementId, then \a el is not added and the existing element is returned.
    //! @param el  The element to be edited
    //! @return The element that is in the collection or nullptr if the element could not be copied for edit.
    DgnElementPtr EditElement(DgnElementCR el) {auto ee = el.CopyForEdit(); if (ee.IsValid()) return AddElement(*ee); else return nullptr;}
    
    //! Add an editable copy of the specified element and its children to the collection.
    //! @param el       The element to be added
    //! @param maxDepth The levels of child elements to add. Pass 1 to add only the immediate children.
    void EditAssembly(DgnElementCR el, size_t maxDepth = std::numeric_limits<size_t>::max()) {auto ee = el.CopyForEdit(); if (ee.IsValid()) AddAssembly(*ee, maxDepth);}

    //! Look up the editable copy of an element in the collection by its ElementId.
    //! @return The element that is in the collection or nullptr if not found.
    DGNPLATFORM_EXPORT DgnElementPtr FindElementById(DgnElementId);

    //! Remove the specified editable copy of an element from the collection.
    //! @param el  The editable copy of an element in the collection
    //! @note \a el must be the editable copy of the element that is in this collection.
    //! @see FindElementById 
    DGNPLATFORM_EXPORT void RemoveElement(DgnElementR el);

    //! Remove an element's children (by Id) from the collection.
    //! @param el  The parent element to query.
    //! @param maxDepth The levels of child elements to add. Pass 1 to add only the immediate children.
    DGNPLATFORM_EXPORT void RemoveChildren(DgnElementCR el, size_t maxDepth = std::numeric_limits<size_t>::max());
    
    //! Remove the specified editable copy of an element and its children (by Id) from the collection.
    //! @param el  The editable copy of the parent element to remove and to query.
    //! @param maxDepth The levels of child elements to add. Pass 1 to add only the immediate children.
    void RemoveAssembly(DgnElementR el, size_t maxDepth = std::numeric_limits<size_t>::max()) {RemoveElement(el); RemoveChildren(el, maxDepth);}

    //! Get the number of elements currently in the collection
    size_t size() {return m_elements.size();}

    //! Get an iterator pointing to the beginning of the collection
    bvector<DgnElementP>::const_iterator begin() const {return m_elements.begin();}

    //! Get an iterator pointing to the end of the collection
    bvector<DgnElementP>::const_iterator end() const {return m_elements.end();}

    //! Insert or update all elements in the collection. Elements with valid ElementIds are updated. Elements with no ElementIds are inserted. 
    //! @param[out] anyInserts  Optional. If not null, then true is returned if any element in the collection had to be inserted.
    //! @return non-zero error status if any insert or update fails. In that case some elements in the collection may not be written.
    DGNPLATFORM_EXPORT DgnDbStatus Write(bool* anyInserts = nullptr);

    //! Find the first element in the collection that is-a instance of the specified class. @note This is an is-a test, not an exact test.
    DGNPLATFORM_EXPORT DgnElementPtr FindElementByClass(ECN::ECClassCR ecclass);

    //! Find an element in the collection by class
    template<typename T> RefCountedPtr<T> FindByClass(ECN::ECClassCR ecclass) {return dynamic_cast<T*>(FindElementByClass(ecclass).get());}
};

//=======================================================================================
//! Applies a transform to one or more elements
// @bsiclass                                                BentleySystems
//=======================================================================================
struct DgnElementTransformer
{
    DGNPLATFORM_EXPORT static DgnDbStatus ApplyTransformTo(DgnElementR el, Transform const& t); 

    template<typename COLL> static DgnDbStatus ApplyTransformToAll(COLL& collection, Transform const& t) 
        {
        for (auto& item : collection)
            {
            DgnDbStatus status = ApplyTransformTo(*item, t);
            if (DgnDbStatus::Success != status)
                return status;
           }
        return DgnDbStatus::Success;
       }
};

END_BENTLEY_DGN_NAMESPACE
