/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnElement.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BENTLEY_NAMESPACE_TYPEDEFS(HeapZone);
#include <Bentley/BeAssert.h>
#include "RepositoryManager.h"
#include "MemoryManager.h"

BEGIN_BENTLEY_RENDER_NAMESPACE
struct Graphic;
DEFINE_REF_COUNTED_PTR(Graphic)

//=======================================================================================
// Cached set of Render::Graphics for a GeometrySource
// @bsiclass                                                    Keith.Bentley   09/15
//=======================================================================================
struct GraphicSet
{
    struct PtrCompare{bool operator()(Render::GraphicPtr first, Render::GraphicPtr second) const {return first.get()<second.get();}};
    mutable bset<Render::GraphicPtr, PtrCompare, 8> m_graphics;
    DGNPLATFORM_EXPORT Render::Graphic* Find(DgnViewportCR, double metersPerPixel) const;
    DGNPLATFORM_EXPORT void Drop(Render::Graphic&);
    DGNPLATFORM_EXPORT void DropFor(DgnViewportCR viewport);
    void Save(Render::Graphic& graphic) {m_graphics.insert(&graphic);}
    void Clear() {m_graphics.clear();}
    bool IsEmpty() const {return m_graphics.empty();}
};
END_BENTLEY_RENDER_NAMESPACE

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler {struct Element; struct Geometric2d; struct Geometric3d; struct Physical; struct SpatialLocation; struct Annotation2d; struct DrawingGraphic; struct Group; struct Information; struct InformationCarrier; struct Definition; struct Subject;};
namespace dgn_TxnTable {struct Element; struct Model;};

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
    bmap<DgnAuthorityId, DgnAuthorityId> m_authorityId;
    bmap<DgnFontId, DgnFontId> m_fontId;

    template<typename T> T Find(bmap<T,T> const& table, T sourceId) const {auto i = table.find(sourceId); return (i == table.end())? T(): i->second;}
    template<typename T> T FindElement(T sourceId) const {return T(Find<DgnElementId>(m_elementId, sourceId).GetValueUnchecked());}

public:
    DgnRemapTables& Get(DgnDbR);
    DgnAuthorityId Find(DgnAuthorityId sourceId) const {return Find<DgnAuthorityId>(m_authorityId, sourceId);}
    DgnAuthorityId Add(DgnAuthorityId sourceId, DgnAuthorityId targetId) {return m_authorityId[sourceId] = targetId;}
    DgnModelId Find(DgnModelId sourceId) const {return Find<DgnModelId>(m_modelId, sourceId);}
    DgnModelId Add(DgnModelId sourceId, DgnModelId targetId) {return m_modelId[sourceId] = targetId;}
    DgnElementId Find(DgnElementId sourceId) const {return Find<DgnElementId>(m_elementId, sourceId);}
    DgnElementId Add(DgnElementId sourceId, DgnElementId targetId) {return m_elementId[sourceId] = targetId;}
    DgnGeometryPartId Find(DgnGeometryPartId sourceId) const {return Find<DgnGeometryPartId>(m_geomPartId, sourceId);}
    DgnGeometryPartId Add(DgnGeometryPartId sourceId, DgnGeometryPartId targetId) {return m_geomPartId[sourceId] = targetId;}
    DgnCategoryId Find(DgnCategoryId sourceId) const {return FindElement<DgnCategoryId>(sourceId);}
    DgnCategoryId Add(DgnCategoryId sourceId, DgnCategoryId targetId) {return DgnCategoryId((m_elementId[sourceId] = targetId).GetValueUnchecked());}
    DgnMaterialId Find(DgnMaterialId sourceId) const {return FindElement<DgnMaterialId>(sourceId);}
    DgnMaterialId Add(DgnMaterialId sourceId, DgnMaterialId targetId) {return DgnMaterialId((m_elementId [sourceId] = targetId).GetValueUnchecked());}
    DgnTextureId Find(DgnTextureId sourceId) const {return FindElement<DgnTextureId>(sourceId);}
    DgnTextureId Add(DgnTextureId sourceId, DgnTextureId targetId) {return DgnTextureId((m_elementId [sourceId] = targetId).GetValueUnchecked());}
    DgnStyleId Find(DgnStyleId sourceId) const {return FindElement<DgnStyleId>(sourceId);}
    DgnStyleId Add(DgnStyleId sourceId, DgnStyleId targetId) {return DgnStyleId((m_elementId [sourceId] = targetId).GetValueUnchecked());}
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

//=======================================================================================
//! Helps models, elements, aspects and other data structures copy themselves between DgnDbs
//=======================================================================================
struct DgnImportContext : DgnCloneContext
{
private:
    bool            m_areCompatibleDbs;
    DPoint3d        m_xyzOffset;
    AngleInDegrees  m_yawAdj;
    DgnDbR          m_sourceDb;
    DgnDbR          m_destDb;
    bmap<LsComponentId, uint32_t> m_importedComponents;
    mutable bmap<ECN::ECClassCP, BeSQLite::EC::ECInstanceUpdater*> m_updaterCache;

    void ComputeGcsAndGOadjustment();

public:
    //! Construct a DgnImportContext object.
    DGNPLATFORM_EXPORT DgnImportContext(DgnDbR source, DgnDbR dest);

    //! @name Source and Destination Dbs
    //! @{
    DgnDbR GetSourceDb() const {return m_sourceDb;}
    DgnDbR GetDestinationDb() const {return m_destDb;}
    bool IsBetweenDbs() const {return &GetDestinationDb() != &GetSourceDb();}
    //! @}

    //! @name Id remapping
    //! @{
    //! Make sure that a DgnAuthority has been imported
    DGNPLATFORM_EXPORT DgnAuthorityId RemapAuthorityId(DgnAuthorityId sourceId);
    //! Register a copy of a DgnAuthority
    DgnAuthorityId AddAuthorityId(DgnAuthorityId sourceId, DgnAuthorityId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Look up a copy of a model
    DgnModelId FindModelId(DgnModelId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a model
    DgnModelId AddModelId(DgnModelId sourceId, DgnModelId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a GeometryPart has been imported
    DGNPLATFORM_EXPORT DgnGeometryPartId RemapGeometryPartId(DgnGeometryPartId sourceId);
    //! Look up a copy of a Category
    DgnCategoryId FindCategory(DgnCategoryId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a Category
    DgnCategoryId AddCategory(DgnCategoryId sourceId, DgnCategoryId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a Category has been imported
    DGNPLATFORM_EXPORT DgnCategoryId RemapCategory(DgnCategoryId sourceId);
    //! Look up a copy of an subcategory
    DgnSubCategoryId FindSubCategory(DgnSubCategoryId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a SubCategory
    DgnSubCategoryId AddSubCategory(DgnSubCategoryId sourceId, DgnSubCategoryId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a SubCategory has been imported
    DGNPLATFORM_EXPORT DgnSubCategoryId RemapSubCategory(DgnCategoryId destCategoryId, DgnSubCategoryId sourceId);
    //! Make sure that an ECClass has been imported
    DGNPLATFORM_EXPORT DgnClassId RemapClassId(DgnClassId sourceId);
    //! Look up a copy of a Material
    DgnMaterialId FindMaterialId(DgnMaterialId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a Material
    DgnMaterialId AddMaterialId(DgnMaterialId sourceId, DgnMaterialId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a Material has been imported
    DGNPLATFORM_EXPORT DgnMaterialId RemapMaterialId(DgnMaterialId sourceId);
    //! Look up a copy of a Material
    DgnTextureId FindTextureId(DgnTextureId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a Texture
    DgnTextureId AddTextureId(DgnTextureId sourceId, DgnTextureId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a Texture has been imported
    DGNPLATFORM_EXPORT DgnTextureId RemapTextureId(DgnTextureId sourceId);
    //! Look up a copy of a LineStyle
    DgnStyleId FindLineStyleId(DgnStyleId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a LineStyle
    DgnStyleId AddLineStyleId(DgnStyleId sourceId, DgnStyleId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a LineStyle has been imported
    DgnStyleId RemapLineStyleId(DgnStyleId sourceId);
    //! Look up a copy of a LineStyle component
    LsComponentId FindLineStyleComponentId(LsComponentId sourceId) const;
    //! Register a copy of a LineStyle component
    void AddLineStyleComponentId(LsComponentId sourceId, LsComponentId targetId);
    //! Look up a copy of a Material
    //! Make sure that any ids referenced by the supplied GeometryStream have been imported
    DGNPLATFORM_EXPORT DgnDbStatus RemapGeometryStreamIds(GeometryStreamR geom);
    //! Remap a font between databases. If it exists by-type and -name, the Id is simply remapped; if not, a deep copy is made. If a deep copy is made and the source database contained the font data, the font data is also deep copied.
    DGNPLATFORM_EXPORT DgnFontId RemapFont(DgnFontId);
    //! @}

    BeSQLite::EC::ECInstanceUpdater const& GetUpdater(ECN::ECClassCR) const;

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

//__PUBLISH_SECTION_END__

//=======================================================================================
//! Returns all auto- or custom-handled properties on a class that are for the specified type of statements
// @bsiclass                                                    Sam.Wilson      07/16
//=======================================================================================
struct AutoHandledPropertiesCollection
    {
    ECN::ECPropertyIterable m_props;
    ECN::ECPropertyIterable::const_iterator m_end;
    ECN::ECClassCP m_customHandledProperty;
    ECN::ECClassCP m_propertyStatementType;
    ECSqlClassParams::StatementType m_stype;
    bool m_wantCustomHandledProps;

    AutoHandledPropertiesCollection(ECN::ECClassCR eclass, DgnDbR db, ECSqlClassParams::StatementType stype, bool wantCustomHandledProps);

    bool HasCustomHandledProperty(ECN::ECPropertyCR) const;
    ECSqlClassParams::StatementType GetStatementType(ECN::ECPropertyCR) const;

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
            ECN::ECPropertyCP operator*() const { BeAssert(m_i != m_coll.m_end); return *m_i; }
            Iterator& operator++();
            bool operator!=(Iterator const& rhs) const { return !(*this == rhs); }
            bool operator==(Iterator const& rhs) const { return m_i == rhs.m_i; }
            ECSqlClassParams::StatementType GetStatementType() const {return m_stype;}
        };

    typedef Iterator const_iterator;
    const_iterator begin() const { return Iterator(m_props.begin(), *this); }
    const_iterator end() const { return Iterator(m_props.end(), *this); }
    };

//__PUBLISH_SECTION_START__
#define DGNELEMENT_DECLARE_MEMBERS(__ECClassName__,__superclass__) \
    private: typedef __superclass__ T_Super;\
    public: static Utf8CP MyHandlerECClassName() {return __ECClassName__;}\
    protected: virtual Utf8CP _GetHandlerECClassName() const override {return MyHandlerECClassName();}\
               virtual Utf8CP _GetSuperHandlerECClassName() const override {return T_Super::_GetHandlerECClassName();}

#define DGNASPECT_DECLARE_MEMBERS(__ECSchemaName__,__ECClassName__,__superclass__) \
    private:    typedef __superclass__ T_Super;\
    public:     static Utf8CP MyECSchemaName() {return __ECSchemaName__;}\
                static Utf8CP MyECClassName() {return __ECClassName__;}\
    protected:  virtual Utf8CP _GetECSchemaName() const override {return MyECSchemaName();}\
                virtual Utf8CP _GetECClassName() const override {return MyECClassName();}\
                virtual Utf8CP _GetSuperECClassName() const override {return T_Super::_GetECClassName();}


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
* A DgnElement subclass will normally override a few virtual functions in order to support @ref ElementCopying.
*
* <h2>Virtual Member Functions</h2>
* DgnElement defines several virtual functions that control copying and importing.
*   * DgnElement::_CopyFrom must copy member variables from source element. It is used in many different copying operations.
*   * DgnElement::_Clone must make a copy of an element, suitable for inserting into the DgnDb.
*   * DgnElement::_CloneForImport must make a copy of an element in a source DgnDb, suitable for inserting into a target DgnDb. 
*   * DgnElement::_RemapIds must remap any IDs stored in the element or its aspects.
* 
* If you define a new subclass of DgnElement, you may need to override one or more of these virtual methods.
*
* If subclass ...|It must override ...
* ---------------|--------------------
* Defines new member variables|DgnElement::_CopyFrom to copy them.
* Defines new properties that are IDs of any kind|DgnElement::_RemapIds to relocate them to the destination DgnDb.
* Stores some of its data in Aspects|DgnElement::_Clone and DgnElement::_CloneForImport, as described below.
* 
* Normally, there is no need to override _Clone, as the base class implementation will work for subclasses, as it calls _CopyFrom.
*
* If you don't use Aspects, then normally, you won't need to override _Clone and _CloneForImport.
*
* <h2>The Central role of _CopyFrom</h2>
*
* _Clone, _CloneForImport, and CopyForEdit all call _CopyFrom to do one specific part of the copying work: copying the member variables. 
* _CopyFrom must make a straight, faithful copy of the C++ element struct’s member variables only. It must be quick. 
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
* User-defined properties are stored together in JSON format in a single column called "UserProperties" on an instance. ECSQL select statements may
* query UserProperties using ECSQL's JSON functions.
* See DgnElement::GetUserProperty for how to add, update, and query individual user properties in C++.
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
* The base class implementation of DgnElement::_GetPropertyValue and DgnElement::_SetPropertyValue will provide access to all custom-handled properties.
* New or updated auto-handled properties are automatically written to the Db when the element is inserted or updated.
*
* <h4>Validating Auto-Handled Properties</h4>
* The domain schema can specifiy some validation rules for auto-handled properties in the ECSchema, such as the IsNullable CustomAttribute.
* Beyond that, in order to apply custom validation rules to auto-handled properties, a domain must define an element subclass that overrides 
* the DgnElement::_SetPropertyValue method that checks property values. In this case, the ECSchema should <em>also</em> specify @ref ElementRestrictions.
*
* <h3>Custom Properties</h3>
* If, in rare cases, a subclass of DgnElement may want to map a property to a C++ member variable or must provide a custom API for a property.
* That is often necessary for binary data. In such cases, the subclass can take over the job of loading and storing that one property.
* This is called "custom-handling" a property. To opt into custom handling, the property definition in the schema must include the @a CustomHandledProperty
* CustomAttribute. The subclass of DgnElement must then override DgnElement::_BindInsertParams, DgnElement::_BindUpdateParams, and DgnElement::_ReadSelectParams in order to load and store
* the custom-handled properties. The subclass must also override DgnElement::_GetPropertyValue and DgnElement::_SetPropertyValue to provide name-based get/set support for its custom-handled properties.
* Finally, the subclass must override DgnElement::_CopyFrom and possibly other virtual methods in order to support copying and importing of its custom-handled properties.
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
struct EXPORT_VTABLE_ATTRIBUTE DgnElement : NonCopyableClass, ICodedEntity
{
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

public:
    friend struct DgnElements;
    friend struct DgnModel;
    friend struct ElemIdTree;
    friend struct dgn_ElementHandler::Element;
    friend struct dgn_TxnTable::Element;
    friend struct MultiAspect;
    friend struct GeometrySource;

    //! Parameters for creating a new DgnElement
    struct CreateParams
    {
    public:
        DgnDbR          m_dgndb;
        DgnModelId      m_modelId;
        DgnClassId      m_classId;
        DgnCode         m_code;
        Utf8String      m_userLabel;
        DgnElementId    m_id;
        DgnElementId    m_parentId;

        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCode const& code=DgnCode(), Utf8CP label=nullptr, DgnElementId parent=DgnElementId())
            : m_dgndb(db), m_modelId(modelId), m_classId(classId), m_code(code), m_parentId(parent) {SetUserLabel(label);}

        DGNPLATFORM_EXPORT void RelocateToDestinationDb(DgnImportContext&);
        void SetCode(DgnCode code) {m_code = code;}                 //!< Set the DgnCode for elements created with this CreateParams
        void SetUserLabel(Utf8CP label) {m_userLabel.AssignOrClear(label);} //!< Set the Label for elements created with this CreateParams
        void SetElementId(DgnElementId id) {m_id = id;}             //!< @private
        void SetParentId(DgnElementId parent) {m_parentId=parent;}  //!< Set the ParentId for elements created with this CreateParams
        bool IsValid() const {return m_modelId.IsValid() && m_classId.IsValid();}
    };

    //! The Hilite state of a DgnElement. If an element is "hilited", its appearance is changed to call attention to it.
    enum class Hilited : uint8_t
    {
        None         = 0, //!< the element is displayed normally (not hilited)
        Normal       = 1, //!< the element is displayed using the normal hilite appearance
        Background   = 2, //!< the element is displayed with the background color
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

        enum class DropMe {No=0, Yes=1};

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

        //! Called after an update to the element was reversed by undo.
        //! @param[in] original the original DgnElement (after undo)
        //! @param[in] modified the modified DgnElement (before undo)
        //! @return DropMe::Yes to drop this appData, DropMe::No to leave it attached to the DgnElement.
        //! @note This method is called for @b all AppData on both the original and the modified DgnElements.
        virtual DropMe _OnReversedUpdate(DgnElementCR original, DgnElementCR modified) {return DropMe::Yes;}

        //! Called after the element was Deleted.
        //! @param[in]  el the DgnElement that was deleted
        //! @return DropMe::Yes to drop this appData, DropMe::No to leave it attached to the DgnElement.
        virtual DropMe _OnDeleted(DgnElementCR el) {return DropMe::Yes;}
    };

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

        DGNPLATFORM_EXPORT Aspect();

        //! The subclass must implement this method to return the name of the schema that defines the aspect.
        virtual Utf8CP _GetECSchemaName() const = 0;

        //! The subclass must implement this method to return the name of the class that defines the aspect.
        virtual Utf8CP _GetECClassName() const {return BIS_CLASS_ElementAspect;}

        //! The subclass must implement this method to return the name of the superclass
        virtual Utf8CP _GetSuperECClassName() const {return nullptr;}

        //! The subclass must implement this method to report an existing instance on the host element that this instance will replace.
        virtual BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) = 0;

        //! The subclass must override this method in order to insert an empty instance into the Db and associate it with the host element.
        //! @param el   The host element
        //! @note The caller will call _UpdateProperties immediately after calling this method.
        virtual DgnDbStatus _InsertInstance(DgnElementCR el) = 0;

        //! The subclass must override this method in order to delete an existing instance in the Db, plus any ECRelationship that associates it with the host element.
        //! @param el   The host element
        virtual DgnDbStatus _DeleteInstance(DgnElementCR el) = 0;

        //! The subclass must implement this method to update the instance properties.
        virtual DgnDbStatus _UpdateProperties(DgnElementCR el) = 0;

        //! The subclass must implement this method to load properties from the Db.
        //! @param el   The host element
        virtual DgnDbStatus _LoadProperties(DgnElementCR el) = 0;

    public:
        //! Get the Id of this aspect
        BeSQLite::EC::ECInstanceId GetAspectInstanceId() const {return m_instanceId;}

        Utf8CP GetECClassName() const {return _GetECClassName();}
        Utf8CP GetSuperECClassName() const {return _GetSuperECClassName();}

        //! Prepare to delete this aspect.
        //! @note The aspect will not actually be deleted in the Db until you call DgnElements::Update on the aspect's host element.
        void Delete() {m_changeType = ChangeType::Delete;}

        //! Get the Id of the ECClass for this aspect
        DGNPLATFORM_EXPORT DgnClassId GetECClassId(DgnDbR) const;

        //! Get the ECClass for this aspect
        DGNPLATFORM_EXPORT ECN::ECClassCP GetECClass(DgnDbR) const;

        //! The aspect should make a copy of itself.
        DGNPLATFORM_EXPORT virtual RefCountedPtr<DgnElement::Aspect> _CloneForImport(DgnElementCR sourceEl, DgnImportContext& importer) const;

        //! The subclass should override this method if it holds any IDs that must be remapped when it is copied (perhaps between DgnDbs)
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
    //! (Note: This is not stored directly as AppData, but is held by an AppData that aggregates instances for this class.)
    //! @note A domain that defines a subclass of MultiAspect may also define a subclass of dgn_AspectHandler to load it.
    struct EXPORT_VTABLE_ATTRIBUTE MultiAspect : Aspect
    {
        DEFINE_T_SUPER(Aspect)
    protected:
        DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) override final;
        DGNPLATFORM_EXPORT DgnDbStatus _DeleteInstance(DgnElementCR el) override final;
        DGNPLATFORM_EXPORT DgnDbStatus _InsertInstance(DgnElementCR el) override final;

    public:
        //! Load the specified instance
        //! @param el   The host element
        //! @param ecclass The class of ElementAspect to load
        //! @param ecinstanceid The Id of the ElementAspect to load
        //! @note Call this method only if you intend to modify the aspect. Use ECSql to query existing instances of the subclass.
        DGNPLATFORM_EXPORT static MultiAspect* GetAspectP(DgnElementR el, ECN::ECClassCR ecclass, BeSQLite::EC::ECInstanceId ecinstanceid);

        template<typename T> static T* GetP(DgnElementR el, ECN::ECClassCR cls, BeSQLite::EC::ECInstanceId id) {return dynamic_cast<T*>(GetAspectP(el,cls,id));}

        //! Prepare to insert an aspect for the specified element
        //! @param el The host element
        //! @param aspect The new aspect to be adopted by the host.
        //! @note \a el will add a reference to \a aspect and will hold onto it.
        //! @note The aspect will not actually be inserted into the Db until you call DgnElements::Insert or DgnElements::Update on \a el
        DGNPLATFORM_EXPORT static void AddAspect(DgnElementR el, MultiAspect& aspect);
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
        Key& GetKey(DgnDbR db) {return GetKey(*GetECClass(db));}
        static UniqueAspect* Find(DgnElementCR, ECN::ECClassCR);
        static RefCountedPtr<DgnElement::UniqueAspect> Load0(DgnElementCR, DgnClassId); // Loads *but does not call AddAppData*
        static UniqueAspect* Load(DgnElementCR, DgnClassId);
        DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) override;
        static void SetAspect0(DgnElementCR el, UniqueAspect& aspect);
        DGNPLATFORM_EXPORT DgnDbStatus _DeleteInstance(DgnElementCR el) override;
        DGNPLATFORM_EXPORT DgnDbStatus _InsertInstance(DgnElementCR el) override final;

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

    //! Allows a business key (unique identifier string) from an external system (identified by DgnAuthorityId) to be associated with a DgnElement via a persistent ElementAspect
    struct EXPORT_VTABLE_ATTRIBUTE ExternalKeyAspect : AppData
    {
    private:
        DgnAuthorityId m_authorityId;
        Utf8String m_externalKey;

        ExternalKeyAspect(DgnAuthorityId authorityId, Utf8CP externalKey)
            {
            m_authorityId = authorityId;
            m_externalKey = externalKey;
            }

    protected:
        DGNPLATFORM_EXPORT virtual DropMe _OnInserted(DgnElementCR) override;

    public:
        DGNPLATFORM_EXPORT static Key const& GetAppDataKey();
        DGNPLATFORM_EXPORT static RefCountedPtr<ExternalKeyAspect> Create(DgnAuthorityId authorityId, Utf8CP externalKey);
        DGNPLATFORM_EXPORT static DgnDbStatus Query(Utf8StringR, DgnElementCR, DgnAuthorityId);
        DGNPLATFORM_EXPORT static DgnDbStatus Delete(DgnElementCR, DgnAuthorityId);
        DgnAuthorityId GetAuthorityId() const {return m_authorityId;}
        Utf8CP GetExternalKey() const {return m_externalKey.c_str();}
    };

    typedef RefCountedPtr<ExternalKeyAspect> ExternalKeyAspectPtr;


private:
    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement, bool isForUpdate);
    template<class T> void CallAppData(T const& caller) const;

    void LoadUserProperties() const;
    void UnloadUserProperties() const;
    DgnDbStatus SaveUserProperties() const;
    void CopyUserProperties(DgnElementCR other);

    bool IsCustomHandledProperty(Utf8CP) const;
    bool IsCustomHandledProperty(ECN::ECPropertyCR) const;

protected:
    //! @private
    struct Flags
    {
        uint32_t m_persistent:1;
        uint32_t m_preassignedId:1;
        uint32_t m_inSelectionSet:1;
        uint32_t m_hilited:3;
        uint32_t m_undisplayed:1;
        uint32_t m_hasAutoHandledProps:2; // 0==unknown, 1==yes, 2==no
        uint32_t m_autoHandledPropsDirty:1;
        Flags() {memset(this, 0, sizeof(*this));}
    };

    mutable BeAtomic<uint32_t> m_refCount;
    DgnDbR        m_dgndb;
    DgnElementId  m_elementId;
    DgnElementId  m_parentId;
    DgnModelId    m_modelId;
    DgnClassId    m_classId;
    DgnCode       m_code;
    Utf8String    m_userLabel;
    mutable ECN::IECInstancePtr m_autoHandledProperties;
    mutable Flags m_flags;
    mutable ECN::AdHocJsonContainerP m_userProperties;
    mutable bmap<AppData::Key const*, RefCountedPtr<AppData>, std::less<AppData::Key const*>, 8> m_appData;

#if !defined (DOCUMENTATION_GENERATOR)
    virtual Utf8CP _GetHandlerECClassName() const {return MyHandlerECClassName();}
    virtual Utf8CP _GetSuperHandlerECClassName() const {return nullptr;}

    void SetPersistent(bool val) const {m_flags.m_persistent = val;}
    void InvalidateElementId() {m_elementId = DgnElementId();}
    void InvalidateCode() {m_code = DgnCode();}
#endif
    
    ECN::IECInstanceP GetAutoHandledProperties() const;
    BeSQLite::EC::ECInstanceUpdater* GetAutoHandledPropertiesUpdater() const;
    DgnDbStatus UpdateAutoHandledProperties();

    static CreateParams InitCreateParamsFromECInstance(DgnDbStatus*, DgnDbR db, ECN::IECInstanceCR);

    //! Invokes _CopyFrom() in the context of _Clone() or _CloneForImport(), preserving this element's code as specified by the CreateParams supplied to those methods.
    void CopyForCloneFrom(DgnElementCR src);

    DGNPLATFORM_EXPORT virtual ~DgnElement();

    //! Return the value of a DateTime ECProperty by name
    //! @note Returns an invalid DateTime if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT DateTime GetPropertyValueDateTime(Utf8CP propertyName) const;
    //! Return the value of a DPoint3d ECProperty by name
    //! @note Returns DPoint3d::From(0,0,0) if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT DPoint3d GetPropertyValueDPoint3d(Utf8CP propertyName) const;
    //! Return the value of a DPoint2d ECProperty by name
    //! @note Returns DPoint2d::From(0,0,0) if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT DPoint2d GetPropertyValueDPoint2d(Utf8CP propertyName) const;
    //! Return the value of a boolean ECProperty by name
    //! @note Returns false if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT bool GetPropertyValueBoolean(Utf8CP propertyName) const;
    //! Return the value of a double ECProperty by name
    //! @note Returns 0.0 if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT double GetPropertyValueDouble(Utf8CP propertyName) const;
    //! Return the value of a integer ECProperty by name
    //! @note Returns 0 if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT int32_t GetPropertyValueInt32(Utf8CP propertyName) const;
    //! Return the value of a UInt64 ECProperty by name
    //! @note Returns 0 if underlying property is null.  Use GetPropertyValue if this behavior is not acceptable.
    //! @see GetPropertyValue
    DGNPLATFORM_EXPORT uint64_t GetPropertyValueUInt64(Utf8CP propertyName) const;
    //! Return the value of an ECNavigationProperty by name
    template <class TBeInt64Id> TBeInt64Id GetPropertyValueId(Utf8CP propertyName) const
        {
        return TBeInt64Id(GetPropertyValueUInt64(propertyName));
        }
    //! Return the value of a string ECProperty by name
    DGNPLATFORM_EXPORT Utf8String GetPropertyValueString(Utf8CP propertyName) const;
    //! Get the 3 property values that back a YPR
    YawPitchRollAngles GetPropertyValueYpr(Utf8CP yawName, Utf8CP pitchName, Utf8CP rollName) const;

    //! Set a DateTime ECProperty by name
    //! @see SetPropertyValue
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, DateTimeCR value);
    //! Set a DPoint3d ECProperty by name
    //! @see SetPropertyValue
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, DPoint3dCR value);
    //! Set a DPoint2d ECProperty by name
    //! @see SetPropertyValue
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, DPoint2dCR value);
    //! Set a boolean ECProperty by name
    //! @see SetPropertyValue
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, bool value);
    //! Set a double ECProperty by name
    //! @see SetPropertyValue
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, double value);
    //! Set an integer ECProperty by name
    //! @see SetPropertyValue
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, int32_t value);
    //! Set an ECNavigationProperty by name
    //! @see SetPropertyValue
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, BeInt64Id value);
    //! Set a string ECProperty by name
    //! @see SetPropertyValue
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValue(Utf8CP propertyName, Utf8CP value);
    //! Set the three property values that back a YPR
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValueYpr(YawPitchRollAnglesCR angles, Utf8CP yawName, Utf8CP pitchName, Utf8CP rollName);

    //! Invoked when loading an element from the database, to allow subclasses to extract their custom-handled property values
    //! from the SELECT statement. The parameters are those which are marked in the schema with the CustomHandledProperty CustomAttribute.
    //! @param[in] statement The SELECT statement which selected the data from the database
    //! @param[in] selectParams The custom-handled properties selected by the SELECT statement. Use this to obtain an index into the statement.
    //! @return DgnDbStatus::Success if the data was loaded successfully, else an error status.
    //! @note If you override this method, you @em must first call T_Super::_ReadSelectParams, forwarding its status.
    //! You should then extract your subclass custom-handled properties from the supplied ECSqlStatement, using
    //! selectParams.GetParameterIndex() to look up the index of each parameter within the statement.
    //! @see ElementProperties
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParamsCR selectParams);

    //! Override this method if your element needs to load additional data from the database when it is loaded (for example,
    //! look up related data in another table).
    //! @note If you override this method, you @em must call T_Super::_LoadFromDb() first, forwarding its status
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LoadFromDb();

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert();

    //! Called to bind the element's custom-handled property values to the ECSqlStatement when inserting
    //! a new element. The parameters to bind are the ones which are marked in the schema with the CustomHandledProperty CustomAttribute.
    //! @note If you override this method, you should bind your subclass custom-handled properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with each custom-handled property's name.
    //! Then you @em must call T_Super::_BindInsertParams, forwarding its status.
    //! @see ElementProperties
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& statement);

    //! Override this method if your element needs to do additional Inserts into the database (for example,
    //! insert a relationship between the element and something else).
    //! @note If you override this method, you @em must call T_Super::_InsertInDb() first.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _InsertInDb();

    //! Called after a DgnElement was successfully inserted into the database.
    //! @note If you override this method, you @em must call T_Super::_OnInserted.
    DGNPLATFORM_EXPORT virtual void _OnInserted(DgnElementP copiedFrom) const;

    //! Called after a DgnElement was successfully imported into the database.
    virtual void _OnImported(DgnElementCR original, DgnImportContext& importer) const {}
    
    //! Called after a DgnElement that was previously deleted has been reinstated by an undo.
    //! @note If you override this method, you @em must call T_Super::_OnReversedDelete.
    DGNPLATFORM_EXPORT virtual void _OnReversedDelete() const;

    //! Called when this element is about to be replace its original element in the DgnDb.
    //! @param [in] original the original state of this element.
    //! Subclasses may override this method to control whether their instances are updated.
    //! @return DgnDbStatus::Success to allow the update, otherwise the update will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnUpdate, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR original);

    //! Called to bind the element's property values to the ECSqlStatement when updating
    //! an existing element. The parameters to bind are the ones which are marked in the schema with the CustomHandledProperty CustomAttribute.
    //! @note If you override this method, you should bind your subclass custom-handled properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with each custom-handled property's name.
    //! Then you @em must call T_Super::_BindUpdateParams, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement);

    //! Called to update a DgnElement in the DgnDb with new values. Override to update subclass custom-handled properties.
    //! @note This method is called from DgnElements::Update, on the persistent element, after its values have been
    //! copied from the modified version. If the update fails, the original data will be copied back into this DgnElement. Only
    //! override this method if your element needs to do additional work when updating the element, such as updating
    //! a relationship.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateInDb();

    //! Called on the replacement element, after a DgnElement was successfully updated, but before the data is 
    //! copied into the original element and before its parent is notified.
    //! The replacement element will be in its post-updated state and the original element is supplied.
    //! @note If you override this method, you @em must call T_Super::_OnUpdated.
    DGNPLATFORM_EXPORT virtual void _OnUpdated(DgnElementCR original) const;

    //! Called after a DgnElement was successfully updated from a replacment element and it now holds the data from the replacement.
    //! @note If you override this method, you @em must call T_Super::_OnUpdateFinished.
    virtual void _OnUpdateFinished() const {}

    //! Called after an update to this DgnElement was reversed by undo. The element will be in its original (pre-change, post-undo) state.
    //! @note If you override this method, you @em must call T_Super::_OnReversedUpdate.
    DGNPLATFORM_EXPORT virtual void _OnReversedUpdate(DgnElementCR changed) const;

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

    //! Called after a DgnElement that was previously added has been removed by undo.
    //! @note If you override this method, you @em must call T_Super::_OnReversedAdd.
    DGNPLATFORM_EXPORT virtual void _OnReversedAdd() const;

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
    virtual DgnDbStatus _OnChildImport(DgnElementCR child, DgnModelR destModel, DgnImportContext& importer) const {return DgnDbStatus::Success;}

    //! Called after an element, with this element as its parent, was successfully imported
    //! @param[in] original The original element which was cloned for import, which is @em not necessarily a child of this element.
    //! @param[in] imported The clone which was imported, which is a child of this element.
    //! @param[in] importer Enables the element to copy the resources that it needs (if copying between DgnDbs) and to remap any references that it holds to things outside itself to the copies of those things.
    virtual void _OnChildImported(DgnElementCR original, DgnElementCR imported, DgnImportContext& importer) const {}

    //! Get the size, in bytes, used by this DgnElement. This is used by the element memory management routines to gauge the "weight" of
    //! each element, so it is not necessary for the value to be 100% accurate.
    //! @note Subclasses of DgnElement that add any member variables should override this method using this template:
    //! uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() + (sizeof(*this) - sizeof(T_Super)) + myAllocedSize;}
    //! where "myAllocedSize" is the number of bytes allocated for this element, held through member variable pointers.
    virtual uint32_t _GetMemSize() const {return sizeof(*this);}

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
    virtual Utf8String _GetDisplayLabel() const {return HasUserLabel() ? m_userLabel : GetCode().GetValue();}

    //! Change the parent (owner) of this DgnElement.
    //! The default implementation sets the parent without doing any checking.
    //! @return DgnDbStatus::Success if the parentId was changed, error status otherwise.
    //! Override to validate the parent/child relationship and return a value other than DgnDbStatus::Success to reject proposed new parent.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetParentId(DgnElementId parentId);

    //! Disclose any locks which must be acquired and/or codes which must be reserved in order to perform the specified operation on this element.
    //! @param[in]      request  Request to populate
    //! @param[in]      opcode   The operation to be performed
    //! @param[in]      original If DbOpcode::Update, the persistent state of this element; otherwise, nullptr.
    //! @return RepositoryStatus::Success, or an error code if for example a required lock or code is known to be unavailable without querying the repository manager.
    //! @note If you override this function you @b must call T_Super::_PopulateRequest(), forwarding its status.
    DGNPLATFORM_EXPORT virtual RepositoryStatus _PopulateRequest(IBriefcaseManager::Request& request, BeSQLite::DbOpcode opcode, DgnElementCP original) const;

    virtual DgnCode const& _GetCode() const override final {return m_code;}
    virtual bool _SupportsCodeAuthority(DgnAuthorityCR) const override {return true;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetCode(DgnCode const& code) override final;
    DGNPLATFORM_EXPORT virtual DgnCode _GenerateDefaultCode() const override;
    virtual DgnElementCP _ToDgnElement() const override final {return this;}
    virtual DgnDbR _GetDgnDb() const override final {return m_dgndb;}
    virtual GeometrySourceCP _ToGeometrySource() const {return nullptr;}
    virtual AnnotationElement2dCP _ToAnnotationElement2d() const {return nullptr;}
    virtual DrawingGraphicCP _ToDrawingGraphic() const {return nullptr;}
    virtual InformationElementCP _ToInformationElement() const {return nullptr;}
    virtual DefinitionElementCP _ToDefinitionElement() const {return nullptr;}
    virtual GroupInformationElementCP _ToGroupInformationElement() const {return nullptr;}
    virtual IElementGroupCP _ToIElementGroup() const {return nullptr;}
    virtual DgnGeometryPartCP _ToGeometryPart() const {return nullptr;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetPropertyValue(Utf8CP name, ECN::ECValueCR value);
    DGNPLATFORM_EXPORT virtual DgnDbStatus _GetPropertyValue(ECN::ECValueR value, Utf8CP name) const;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetPropertyValues(ECN::IECInstanceCR);

    //! Construct a DgnElement from its params
    DGNPLATFORM_EXPORT explicit DgnElement(CreateParams const& params);

    void ClearAllAppData(){m_appData.clear();}//< @private
    

    //! Generate the CreateParams to use for Import
    //! @param destModel Specifies the model into which the element is being cloned
    //! @param importer Specifies source and destination DgnDbs and knows how to remap IDs
    //! @return CreateParams initialized with the element's current data
    //! @remarks The m_id fields are \em not set, as it is never correct for two elements to have the same Id. The m_parentId field is not set,
    //! as it is not clear if the copy should be a child of the same parent as the original. The caller can set this if appropriate.
    //! The m_code field is copied \em only when cloning between two different DgnDbs, as it is never correct for two elements to have the same code.
    CreateParams GetCreateParamsForImport(DgnModelR destModel, DgnImportContext& importer) const;

public:
    static Utf8CP MyHandlerECClassName() {return BIS_CLASS_Element;}                //!< @private
    Utf8CP GetHandlerECClassName() const {return _GetHandlerECClassName();}             //!< @private
    Utf8CP GetSuperHandlerECClassName() const {return _GetSuperHandlerECClassName();}   //!< @private

    DGNPLATFORM_EXPORT void AddRef() const;  //!< @private
    DGNPLATFORM_EXPORT void Release() const; //!< @private
    uint32_t GetRefCount() const {return m_refCount.load();} //!< Get the current reference count for this DgnElement.

    //! @name Dynamic casting to subclasses of DgnElement 
    //! @{
    GeometrySourceCP ToGeometrySource() const {return _ToGeometrySource();} //!< more efficient substitute for dynamic_cast<GeometrySourceCP>(el)
    DGNPLATFORM_EXPORT GeometrySource2dCP ToGeometrySource2d() const;
    DGNPLATFORM_EXPORT GeometrySource3dCP ToGeometrySource3d() const;

    DgnGeometryPartCP ToGeometryPart() const {return _ToGeometryPart();}                //!< more efficient substitute for dynamic_cast<DgnGeometryPartCP>(el)
    InformationElementCP ToInformationElement() const {return _ToInformationElement();} //!< more efficient substitute for dynamic_cast<InformationElementCP>(el)
    DefinitionElementCP ToDefinitionElement() const {return _ToDefinitionElement();}    //!< more efficient substitute for dynamic_cast<DefinitionElementCP>(el)
    AnnotationElement2dCP ToAnnotationElement2d() const {return _ToAnnotationElement2d();} //!< more efficient substitute for dynamic_cast<AnnotationElement2dCP>(el)
    DrawingGraphicCP ToDrawingGraphic() const {return _ToDrawingGraphic();}             //!< more efficient substitute for dynamic_cast<DrawingGraphicCP>(el)
    IElementGroupCP ToIElementGroup() const {return _ToIElementGroup();}                //!< more efficient substitute for dynamic_cast<IElementGroup>(el)
    GroupInformationElementCP ToGroupInformationElement() const {return _ToGroupInformationElement();} //!< more efficient substitute for dynamic_cast<GroupInformationElementCP>(el)
    
    GeometrySourceP ToGeometrySourceP() {return const_cast<GeometrySourceP>(_ToGeometrySource());} //!< more efficient substitute for dynamic_cast<GeometrySourceP>(el)
    GeometrySource2dP ToGeometrySource2dP() {return const_cast<GeometrySource2dP>(ToGeometrySource2d());} //!< more efficient substitute for dynamic_cast<GeometrySource2dP>(el)
    GeometrySource3dP ToGeometrySource3dP() {return const_cast<GeometrySource3dP>(ToGeometrySource3d());} //!< more efficient substitute for dynamic_cast<GeometrySource3dP>(el)

    DgnGeometryPartP ToGeometryPartP() {return const_cast<DgnGeometryPartP>(_ToGeometryPart());} //!< more efficient substitute for dynamic_cast<DgnGeometryPartCP>(el)
    InformationElementP ToInformationElementP() {return const_cast<InformationElementP>(_ToInformationElement());} //!< more efficient substitute for dynamic_cast<InformationElementP>(el)
    DefinitionElementP ToDefinitionElementP() {return const_cast<DefinitionElementP>(_ToDefinitionElement());}  //!< more efficient substitute for dynamic_cast<DefinitionElementP>(el)
    GroupInformationElementP ToGroupInformationElementP() {return const_cast<GroupInformationElementP>(_ToGroupInformationElement());} //!< more efficient substitute for dynamic_cast<GroupInformationElementP>(el)
    AnnotationElement2dP ToAnnotationElement2dP() {return const_cast<AnnotationElement2dP>(_ToAnnotationElement2d());} //!< more efficient substitute for dynamic_cast<AnnotationElement2dP>(el)
    DrawingGraphicP ToDrawingGraphicP() {return const_cast<DrawingGraphicP>(_ToDrawingGraphic());} //!< more efficient substitute for dynamic_cast<DrawingGraphicP>(el)
    //! @}

    bool Is3d() const {return nullptr != ToGeometrySource3d();}                     //!< Determine whether this element is 3d or not
    bool Is2d() const {return nullptr != ToGeometrySource2d();}                     //!< Determine whether this element is 2d or not
    bool IsGeometricElement() const {return nullptr != ToGeometrySource();}         //!< Determine whether this element is a GeometricElement or not
    bool IsInformationElement() const {return nullptr != ToInformationElement();}   //!< Determine whether this element is an InformationElement or not
    bool IsDefinitionElement() const {return nullptr != ToDefinitionElement();}     //!< Determine whether this element is a DefinitionElement or not
    bool IsGroupInformationElement() const {return nullptr != ToGroupInformationElement();} //!< Determine whether this element is a GroupInformationElement or not
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

    //! Create a copy of this DgnElement and all of its extended content in a destination model.
    //! The copied element will be persistent in the destination DgnDb. Also see @ref ElementCopying.
    //! @param[out] stat Optional status to describe failures, a valid DgnElementPtr will only be returned if successful.
    //! @param[in] destModel The destination model (which must be in the importer's destination Db).
    //! @param[in] importer Enables the element to copy the resources that it needs (if copying between DgnDbs) and to remap any references that it holds to things outside itself to the copies of those things.
    //! @remarks The element's code will \em not be copied to the copied element if the import is being performed within a single DgnDb, as it is never correct for two elements within the same DgnDb to have the same code.
    //! @return The persistent copy of the element
    DGNPLATFORM_EXPORT DgnElementCPtr Import(DgnDbStatus* stat, DgnModelR destModel, DgnImportContext& importer) const;

    //! Update the persistent state of a DgnElement in the DgnDb from this modified copy of it.
    //! This is merely a shortcut for el.GetDgnDb().Elements().Update(el, stat);
    DGNPLATFORM_EXPORT DgnElementCPtr Update(DgnDbStatus* stat=nullptr);

    //! Insert this DgnElement into the DgnDb.
    //! This is merely a shortcut for el.GetDgnDb().Elements().Insert(el, stat);
    DGNPLATFORM_EXPORT DgnElementCPtr Insert(DgnDbStatus* stat=nullptr);

    //! Delete this DgnElement from the DgnDb,
    //! This is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    DGNPLATFORM_EXPORT DgnDbStatus Delete() const;

    //! Get the ElementHandler for this DgnElement.
    DGNPLATFORM_EXPORT ElementHandlerR GetElementHandler() const;

    //! @name AppData Management
    //! @{
    //! Get the HeapZone for the DgnDb of this element.

    //! Add Application Data to this element.
    //! @param[in] key The AppData's key. If AppData with this key already exists on this element, it is dropped and
    //! replaced with \a appData.
    //! @param[in] appData The appData object to attach to this element.
    DGNPLATFORM_EXPORT void AddAppData(AppData::Key const& key, AppData* appData) const;

    //! Drop Application data from this element.
    //! @param[in] key the key for the AppData to drop.
    //! @return SUCCESS if an entry with \a key is found and dropped.
    DGNPLATFORM_EXPORT StatusInt DropAppData(AppData::Key const& key) const;

    //! Find DgnElementAppData on this element by key.
    //! @param[in] key The key for the AppData of interest.
    //! @return the AppData for key \a key, or nullptr.
    DGNPLATFORM_EXPORT AppData* FindAppData(AppData::Key const& key) const;

    //! @private
    DGNPLATFORM_EXPORT void CopyAppDataFrom(DgnElementCR source) const;

    //! @}

    //! Get the DgnModelId of this DgnElement.
    DgnModelId GetModelId() const {return m_modelId;}

    //! Get the DgnModel of this DgnElement.
    DGNPLATFORM_EXPORT DgnModelPtr GetModel() const;

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

    //! Get the DgnElementId of the parent of this element.
    //! @see SetParentId
    //! @return Id will be invalid if this element does not have a parent element.
    DgnElementId GetParentId() const {return m_parentId;}
    //! Set the parent (owner) of this DgnElement.
    //! @see GetParentId, _SetParentId
    //! @return DgnDbStatus::Success if the parent was set
    //! @note This call can fail if a DgnElement subclass overrides _SetParentId and rejects the parent.
    DgnDbStatus SetParentId(DgnElementId parentId) {return parentId == GetParentId() ? DgnDbStatus::Success : _SetParentId(parentId);}

    //! Query the database for the last modified time of this DgnElement.
    DGNPLATFORM_EXPORT DateTime QueryTimeStamp() const;

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

    //! Disclose any locks which must be acquired and/or codes which must be reserved in order to perform the specified operation on this element.
    //! @param[in] request Request to populate
    //! @param[in] opcode The operation to be performed
    //! @return RepositoryStatus::Success, or an error code if for example a required lock or code is known to be unavailable without querying the repository manager.
    DGNPLATFORM_EXPORT RepositoryStatus PopulateRequest(IBriefcaseManager::Request& request, BeSQLite::DbOpcode opcode) const;

    //! @name Properties 
    //! @{

    //! Get the user property on this DgnElement to get or set its value. Also see @ref ElementProperties.
    //! @param[in] name Name of the user property
    //! @remarks The element needs to be held in memory to access the returned property value. 
    DGNPLATFORM_EXPORT ECN::AdHocJsonPropertyValue GetUserProperty(Utf8CP name) const;

    //! Returns true if the Element contains the user property. Also see @ref ElementProperties.
    //! @param[in] name Name of the user property
    DGNPLATFORM_EXPORT bool ContainsUserProperty(Utf8CP name) const;

    //! Get a user property on this DgnElement. Also see @ref ElementProperties.
    //! @param[in] name Name of the user property
    DGNPLATFORM_EXPORT void RemoveUserProperty(Utf8CP name);

    //! Clear all the user properties on this DgnElement. Also see @ref ElementProperties.
    DGNPLATFORM_EXPORT void ClearUserProperties();

    //! Get the value of a property. Also see @ref ElementProperties.
    //! @param value The returned value
    //! @param name The name of the property
    //! @return non-zero error status if this element has no such property or if the subclass has chosen not to expose it via this function
    DgnDbStatus GetPropertyValue(ECN::ECValueR value, Utf8CP name) const {return _GetPropertyValue(value, name);}

    //! Set the value of a property. 
    //! @note This function does not write to the bim. The caller must call Update in order to write the element and all of 
    //! its modified property to the DgnDb. Also see @ref ElementProperties.
    //! @param value The returned value
    //! @param name The name of the property
    //! @return non-zero error status if this element has no such property, if the value is illegal, or if the subclass has chosen not to expose the property via this function
    DgnDbStatus SetPropertyValue(Utf8CP name, ECN::ECValueCR value) {return _SetPropertyValue(name, value);}

    //! Set the properties of this element from the specified instance. Calls _SetPropertyValue for each non-NULL property in the input instance.
    //! @param instance The source of the properties that are to be copied to this element
    //! @return non-zero error status if any property could not be set. Note that some properties might be set while others are not in case of error.
    DGNPLATFORM_EXPORT DgnDbStatus SetPropertyValues(ECN::IECInstanceCR instance) {return _SetPropertyValues(instance);}

    //! @}
};

//=======================================================================================
//! A stream of geometry, stored on a DgnElement, created by an GeometryBuilder.
//! @ingroup GROUP_Geometry
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
struct GeometryStream : ByteStream
{
public:
    bool HasGeometry() const {return HasData();}  //!< return false if this GeometryStream is empty.

    //! @private
    DgnDbStatus ReadGeometryStream(BeSQLite::SnappyFromMemory& snappy, DgnDbR dgnDb, void const* blob, int blobSize);
    //! @private
    static DgnDbStatus WriteGeometryStream(BeSQLite::SnappyToBlob&, DgnDbR, DgnElementId, Utf8CP tableName, Utf8CP columnName);
    //! @private
    DgnDbStatus BindGeometryStream(bool& multiChunkGeometryStream, BeSQLite::SnappyToBlob&, BeSQLite::EC::ECSqlStatement&, Utf8CP parameterName) const;
};

//=======================================================================================
//! The position, orientation, and size of a 3d element.
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct Placement3d
{
protected:
    DPoint3d m_origin;
    YawPitchRollAngles m_angles;
    ElementAlignedBox3d m_boundingBox;

public:
    Placement3d() : m_origin(DPoint3d::FromZero())  {}
    Placement3d(DPoint3dCR origin, YawPitchRollAngles angles, ElementAlignedBox3dCR box = ElementAlignedBox3d()): m_origin(origin), m_angles(angles), m_boundingBox(box) {}
    Placement3d(Placement3d const& rhs) : m_origin(rhs.m_origin), m_angles(rhs.m_angles), m_boundingBox(rhs.m_boundingBox) {}
    Placement3d(Placement3d&& rhs) : m_origin(rhs.m_origin), m_angles(rhs.m_angles), m_boundingBox(rhs.m_boundingBox) {}
    Placement3d& operator=(Placement3d&& rhs) {m_origin=rhs.m_origin; m_angles=rhs.m_angles; m_boundingBox=rhs.m_boundingBox; return *this;}
    Placement3d& operator=(Placement3d const& rhs) {m_origin=rhs.m_origin; m_angles=rhs.m_angles; m_boundingBox=rhs.m_boundingBox; return *this;}

    //! Get the origin of this Placement3d.
    DPoint3dCR GetOrigin() const {return m_origin;}

    //! Get a writable reference to the origin of this Placement3d.
    DPoint3dR GetOriginR() {return m_origin;}

    //! Get the YawPitchRollAngles of this Placement3d.
    YawPitchRollAnglesCR GetAngles() const {return m_angles;}

    //! Get a writable reference to the YawPitchRollAngles of this Placement3d.
    YawPitchRollAnglesR GetAnglesR() {return m_angles;}

    //! Get the ElementAlignedBox3d of this Placement3d.
    ElementAlignedBox3d const& GetElementBox() const {return m_boundingBox;}

    //! Get a writable reference to the ElementAlignedBox3d of this Placement3d.
    ElementAlignedBox3d& GetElementBoxR() {return m_boundingBox;}

    //! Convert the origin and YawPitchRollAngles of this Placement3d into a Transform.
    Transform GetTransform() const {return m_angles.ToTransform(m_origin);}

    //! Calculate the AxisAlignedBox3d of this Placement3d.
    DGNPLATFORM_EXPORT AxisAlignedBox3d CalculateRange() const;

    //! Determine whether this Placement3d is valid.
    bool IsValid() const
        {
        if (!m_boundingBox.IsValid())
            return false;

        double maxCoord = DgnUnits::DiameterOfEarth();

        if (m_boundingBox.low.x < -maxCoord || m_boundingBox.low.y < -maxCoord || m_boundingBox.low.z < -maxCoord ||
            m_boundingBox.high.x > maxCoord || m_boundingBox.high.y > maxCoord || m_boundingBox.high.z > maxCoord)
            return false;

        if (fabs(m_origin.x) > maxCoord || fabs(m_origin.y) > maxCoord || fabs(m_origin.z) > maxCoord)
            return false;

        return true;
        }
};

//=======================================================================================
//! The position, rotation angle, and bounding box for a 2-dimensional element.
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct Placement2d
{
protected:
    DPoint2d            m_origin;
    AngleInDegrees      m_angle;
    ElementAlignedBox2d m_boundingBox;

public:
    Placement2d() : m_origin(DPoint2d::FromZero()) {}
    Placement2d(DPoint2dCR origin, AngleInDegrees const& angle, ElementAlignedBox2dCR box = ElementAlignedBox2d()) : m_origin(origin), m_angle(angle), m_boundingBox(box){}
    Placement2d(Placement2d const& rhs) : m_origin(rhs.m_origin), m_angle(rhs.m_angle), m_boundingBox(rhs.m_boundingBox) {}
    Placement2d(Placement2d&& rhs) : m_origin(rhs.m_origin), m_angle(rhs.m_angle), m_boundingBox(rhs.m_boundingBox) {}
    Placement2d& operator=(Placement2d&& rhs) {m_origin=rhs.m_origin; m_angle=rhs.m_angle; m_boundingBox=rhs.m_boundingBox; return *this;}
    Placement2d& operator=(Placement2d const& rhs) {m_origin=rhs.m_origin; m_angle=rhs.m_angle; m_boundingBox=rhs.m_boundingBox; return *this;}

    //! Get the origin of this Placement2d.
    DPoint2dCR GetOrigin() const {return m_origin;}

    //! Get a writable reference to the origin of this Placement2d.
    DPoint2dR GetOriginR() {return m_origin;}

    //! Get the angle of this Placement2d
    AngleInDegrees GetAngle() const {return m_angle;}

    //! Get a writable reference to the angle of this Placement2d.
    AngleInDegrees& GetAngleR() {return m_angle;}

    //! Get the ElementAlignedBox2d of this Placement2d.
    ElementAlignedBox2d const& GetElementBox() const {return m_boundingBox;}

    //! Get a writable reference to the ElementAlignedBox2d of this Placement2d.
    ElementAlignedBox2d& GetElementBoxR() {return m_boundingBox;}

    //! Convert the origin and angle of this Placement2d into a Transform.
    Transform GetTransform() const {Transform t; t.InitFromOriginAngleAndLengths(m_origin, m_angle.Radians(), 1.0, 1.0); return t;}

    //! Calculate an AxisAlignedBox3d for this Placement2d.
    //! @note the z values are arbitrarily set to +-.5 mm.
    DGNPLATFORM_EXPORT AxisAlignedBox3d CalculateRange() const;

    //! Determine whether this Placement2d is valid
    bool IsValid() const
        {
        if (!m_boundingBox.IsValid())
            return false;

        double maxCoord = DgnUnits::DiameterOfEarth();

        if (m_boundingBox.low.x < -maxCoord || m_boundingBox.low.y < -maxCoord ||
            m_boundingBox.high.x > maxCoord || m_boundingBox.high.y > maxCoord)
            return false;

        if (fabs(m_origin.x) > maxCoord || fabs(m_origin.y) > maxCoord)
            return false;

        return true;
        }
};

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  11/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometrySource
{
    friend struct GeometryBuilder;

protected:
    virtual Render::GraphicSet& _Graphics() const = 0;
    virtual DgnDbR _GetSourceDgnDb() const = 0;
    virtual DgnElementCP _ToElement() const = 0;
    virtual GeometrySource2dCP _ToGeometrySource2d() const = 0; // Either this method or _ToGeometrySource3d must return non-null.
    virtual GeometrySource3dCP _ToGeometrySource3d() const = 0; // Either this method or _ToGeometrySource2d must return non-null.
    virtual DgnCategoryId _GetCategoryId() const = 0;
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) = 0;
    virtual GeometryStreamCR _GetGeometryStream() const = 0;
    virtual AxisAlignedBox3d _CalculateRange3d() const = 0;
    DGNPLATFORM_EXPORT virtual Render::GraphicPtr _Stroke(ViewContextR, double pixelSize) const;
    DGNPLATFORM_EXPORT virtual Render::GraphicPtr _StrokeHit(ViewContextR, HitDetailCR) const;
    DGNPLATFORM_EXPORT virtual void _GetInfoString(HitDetailCR, Utf8StringR descr, Utf8CP delimiter) const;
    DGNPLATFORM_EXPORT virtual SnapStatus _OnSnap(SnapContextR) const;
    GeometryStreamR GetGeometryStreamR() {return const_cast<GeometryStreamR>(_GetGeometryStream());} // Only GeometryBuilder should have write access to the GeometryStream...
    virtual DgnElement::Hilited _IsHilited() const {if (nullptr == ToElement()) return DgnElement::Hilited::None; return (DgnElement::Hilited) ToElement()->m_flags.m_hilited;} //!< Get the current Hilited state of this element
    DGNPLATFORM_EXPORT virtual void _SetHilited(DgnElement::Hilited newState) const; //!< Change the current Hilited state of this element

public:
    bool HasGeometry() const {return _GetGeometryStream().HasGeometry();} //!< return false if this geometry source currently has no geometry (is empty).
    DgnDbR GetSourceDgnDb() const {return _GetSourceDgnDb();}
    DgnElementCP ToElement() const {return _ToElement();} //! Caller must be prepared to this to return nullptr.
    DgnElementP ToElementP() {return const_cast<DgnElementP>(_ToElement());} //! Caller must be prepared to this to return nullptr.
    GeometrySource2dCP ToGeometrySource2d() const {return _ToGeometrySource2d();}
    GeometrySource2dP ToGeometrySource2dP() {return const_cast<GeometrySource2dP>(_ToGeometrySource2d());}
    GeometrySource3dCP ToGeometrySource3d() const {return _ToGeometrySource3d();}
    GeometrySource3dP ToGeometrySource3dP() {return const_cast<GeometrySource3dP>(_ToGeometrySource3d());}
    DgnCategoryId GetCategoryId() const {return _GetCategoryId();}
    DgnDbStatus SetCategoryId(DgnCategoryId categoryId) {return _SetCategoryId(categoryId);}
    GeometryStreamCR GetGeometryStream() const {return _GetGeometryStream();}
    AxisAlignedBox3d CalculateRange3d() const {return _CalculateRange3d();}
    DGNPLATFORM_EXPORT Transform GetPlacementTransform() const;

    DgnElement::Hilited IsHilited() const {return _IsHilited();}
    bool IsInSelectionSet() const {if (nullptr == ToElement()) return false; return ToElement()->m_flags.m_inSelectionSet;}
    bool IsUndisplayed() const {if (nullptr == ToElement()) return false; return ToElement()->m_flags.m_undisplayed;} //!< @private
    void SetHilited(DgnElement::Hilited newState) const {_SetHilited(newState);} //!< Change the current Hilited state of this element
    DGNPLATFORM_EXPORT void SetInSelectionSet(bool yesNo) const; //!< @private
    DGNPLATFORM_EXPORT void SetUndisplayed(bool yesNo) const; //!< @private

    Render::GraphicSet& Graphics() const {return _Graphics();}
    Render::GraphicPtr Stroke(ViewContextR context, double pixelSize) const {return _Stroke(context, pixelSize);}
    Render::GraphicPtr StrokeHit(ViewContextR context, HitDetailCR hit) const {return _StrokeHit(context, hit);}
    void GetInfoString(HitDetailCR hit, Utf8StringR descr, Utf8CP delimiter) const {_GetInfoString(hit, descr, delimiter);}
    SnapStatus OnSnap(SnapContextR context) const {return _OnSnap(context);}
};

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  11/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometrySource3d : GeometrySource
{
protected:
    virtual GeometrySource2dCP _ToGeometrySource2d() const override final {return nullptr;}
    virtual AxisAlignedBox3d _CalculateRange3d() const override final {return _GetPlacement().CalculateRange();}
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
    virtual GeometrySource3dCP _ToGeometrySource3d() const override final {return nullptr;}
    virtual AxisAlignedBox3d _CalculateRange3d() const override final {return _GetPlacement().CalculateRange();}
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

    //! Parameters used to construct a GeometricElement
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(GeometricElement::T_Super::CreateParams);

        DgnCategoryId   m_category; //!< The category to which the element belongs

        //! Construct from the supplied parameters
        //! @param[in]      db       The DgnDb in which the element is to reside
        //! @param[in]      modelId  The Id of the model in which the element is to reside
        //! @param[in]      classId  The Id of the element's ECClass
        //! @param[in]      category The category to which the element belongs
        //! @param[in]      code     The element's code
        //! @param[in]      label    (Optional) element label
        //! @param[in]      parent   (Optional) Id of this element's parent element
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCategoryId category, DgnCodeCR code=DgnCode(), Utf8CP label=nullptr, DgnElementId parent=DgnElementId())
            : T_Super(db, modelId, classId, code, label, parent), m_category(category) {}

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @param[in]      category The category to which the element belongs
        //! @return 
        explicit CreateParams(DgnElement::CreateParams const& params, DgnCategoryId category=DgnCategoryId()) : T_Super(params), m_category(category) {}
    };
protected:
    DgnCategoryId               m_categoryId;
    GeometryStream              m_geom;
    mutable Render::GraphicSet  m_graphics;
    mutable bool                m_multiChunkGeomStream;

    explicit GeometricElement(CreateParams const& params) : T_Super(params), m_categoryId(params.m_category), m_multiChunkGeomStream(false) {}

    virtual bool _IsPlacementValid() const = 0;
    virtual Utf8CP _GetGeometryColumnTableName() const = 0;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _InsertInDb() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateInDb() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT virtual void _OnInserted(DgnElementP copiedFrom) const override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR) override;
    DGNPLATFORM_EXPORT virtual void _OnDeleted() const override;
    DGNPLATFORM_EXPORT virtual void _OnReversedAdd() const override;
    DGNPLATFORM_EXPORT virtual void _OnReversedDelete() const override;
    DGNPLATFORM_EXPORT virtual void _OnUpdateFinished() const override;
    DGNPLATFORM_EXPORT virtual void _RemapIds(DgnImportContext&) override;
    virtual uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() + (sizeof(*this) - sizeof(T_Super)) + m_geom.GetAllocSize();}

    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
    GeometryStreamCR GetGeometryStream() const {return m_geom;}
    DgnDbStatus InsertGeomStream() const;
    DgnDbStatus UpdateGeomStream() const;
    DgnDbStatus WriteGeomStream() const;
    DgnDbStatus Validate() const;
    DGNPLATFORM_EXPORT DgnDbStatus DoSetCategoryId(DgnCategoryId catId);
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

    DGNPLATFORM_EXPORT DgnDbStatus GetPlacementProperty(ECN::ECValueR value, Utf8CP name) const;
    DGNPLATFORM_EXPORT DgnDbStatus SetPlacementProperty(Utf8CP name, ECN::ECValueCR value);

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
        //! @param[in] parent    (Optional) Id of this element's parent element
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCategoryId category, Placement3dCR placement=Placement3d(),
                DgnCodeCR code=DgnCode(), Utf8CP label=nullptr, DgnElementId parent=DgnElementId())
            : T_Super(db, modelId, classId, category, code, label, parent), m_placement(placement) {}

        //! Construct from base parameters. Chiefly for internal use
        //! @param[in] params    The base element parameters
        //! @param[in] category  The category to which the element belongs
        //! @param[in] placement The element's placement in 3d space
        explicit CreateParams(DgnElement::CreateParams const& params, DgnCategoryId category=DgnCategoryId(), Placement3dCR placement=Placement3d())
            : T_Super(params, category), m_placement(placement) {}
    };
protected:
    Placement3d m_placement;

    explicit GeometricElement3d(CreateParams const& params) : T_Super(params), m_placement(params.m_placement) {}
    virtual bool _IsPlacementValid() const override final {return m_placement.IsValid();}
    virtual Render::GraphicSet& _Graphics() const override final {return m_graphics;}
    virtual DgnDbR _GetSourceDgnDb() const override final {return GetDgnDb();}
    virtual DgnElementCP _ToElement() const override final {return this;}
    virtual GeometrySourceCP _ToGeometrySource() const override final {return this;}
    virtual GeometrySource3dCP _ToGeometrySource3d() const override final {return this;}
    virtual Utf8CP _GetGeometryColumnTableName() const override final {return BIS_TABLE(BIS_CLASS_GeometricElement3d);}
    virtual DgnCategoryId _GetCategoryId() const override final {return m_categoryId;}
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override {return DoSetCategoryId(categoryId);}
    virtual GeometryStreamCR _GetGeometryStream() const override final {return m_geom;}
    virtual Placement3dCR _GetPlacement() const override final {return m_placement;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetPlacement(Placement3dCR placement) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR) override;
    DGNPLATFORM_EXPORT virtual void _AdjustPlacementForImport(DgnImportContext const&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement&) override;

    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement&);
public:

    DGNPLATFORM_EXPORT DgnDbStatus _GetPropertyValue(ECN::ECValueR value, Utf8CP name) const override;
    DGNPLATFORM_EXPORT DgnDbStatus _SetPropertyValue(Utf8CP name, ECN::ECValueCR value) override;
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

    explicit GeometricElement2d(CreateParams const& params) : T_Super(params), m_placement(params.m_placement) {}
    virtual bool _IsPlacementValid() const override final {return m_placement.IsValid();}
    virtual DgnDbR _GetSourceDgnDb() const override final {return GetDgnDb();}
    virtual DgnElementCP _ToElement() const override final {return this;}
    virtual GeometrySourceCP _ToGeometrySource() const override final {return this;}
    virtual GeometrySource2dCP _ToGeometrySource2d() const override final {return this;}
    virtual Utf8CP _GetGeometryColumnTableName() const override final {return BIS_TABLE(BIS_CLASS_GeometricElement2d);}
    virtual DgnCategoryId _GetCategoryId() const override final {return m_categoryId;}
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override {return DoSetCategoryId(categoryId);}
    virtual GeometryStreamCR _GetGeometryStream() const override final {return m_geom;}
    virtual Placement2dCR _GetPlacement() const override final {return m_placement;}
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetPlacement(Placement2dCR placement) override;
    virtual Render::GraphicSet& _Graphics() const override final {return m_graphics;}
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR) override;
    DGNPLATFORM_EXPORT virtual void _AdjustPlacementForImport(DgnImportContext const&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement&) override;

    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement&);
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
    virtual AnnotationElement2dCP _ToAnnotationElement2d() const override final {return this;}

    explicit AnnotationElement2d(CreateParams const& params) : T_Super(params) {}
}; // AnnotationElement2d

//=======================================================================================
//! A 2-dimensional geometric element used in drawings
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingGraphic : GraphicalElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DrawingGraphic, GraphicalElement2d)
    friend struct dgn_ElementHandler::DrawingGraphic;
public:
    //! Create a DrawingGraphic from CreateParams.
    static DrawingGraphicPtr Create(CreateParams const& params) {return new DrawingGraphic(params);}
protected:
    virtual DrawingGraphicCP _ToDrawingGraphic() const override final {return this;}

    explicit DrawingGraphic(CreateParams const& params) : T_Super(params) {}

}; // DrawingGraphic

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
//! An InformationElement identifies and names information content.
//! @see InformationCarrierElement
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE InformationElement : DgnElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_InformationElement, DgnElement);
    friend struct dgn_ElementHandler::Information;

protected:
    virtual InformationElementCP _ToInformationElement() const override final {return this;}
    explicit InformationElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! An InformationCarrierElement is a proxy for an information carrier in the physical world.  
//! For example, a paper document or an electronic file is an information carrier.
//! The content is tracked separately from the carrier.
//! @see InformationElement
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
//! A DefinitionElement resides in (and only in) a DefinitionModel.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DefinitionElement : InformationElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DefinitionElement, InformationElement);
    friend struct dgn_ElementHandler::Definition;

protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    virtual DefinitionElementCP _ToDefinitionElement() const override final {return this;}
    explicit DefinitionElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A Subject resides in (and only in) a RepositoryModel.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Subject : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_Subject, DefinitionElement);
    friend struct dgn_ElementHandler::Subject;

protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;

    explicit Subject(CreateParams const& params) : T_Super(params) {}

public:
    //! Creates a new child Subject of the specified parent Subject
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static SubjectPtr Create(SubjectCR parentSubject, Utf8CP label, Utf8CP description=nullptr);
    //! Creates a new child Subject of the specified parent Subject
    //! @see DgnElements::GetRootSubject
    DGNPLATFORM_EXPORT static SubjectCPtr CreateAndInsert(SubjectCR parentSubject, Utf8CP label, Utf8CP description=nullptr);
};

//=======================================================================================
//! Abstract base class for group-related information elements.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    04/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GroupInformationElement : InformationElement
{
    DEFINE_T_SUPER(InformationElement);
protected:
    virtual GroupInformationElementCP _ToGroupInformationElement() const override final {return this;}
    explicit GroupInformationElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! Abstract base class for roles played by other (typically physical) elements.
//! For example:
//! - <i>Lawyer</i> and <i>employee</i> are potential roles of a person
//! - <i>Asset</i> and <i>safey hazard</i> are potential roles of a PhysicalElement
//! @ingroup GROUP_DgnElement
// @bsiclass                                                    Shaun.Sewall    05/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoleElement : DgnElement
{
    DEFINE_T_SUPER(DgnElement);
protected:
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
struct DgnElements : DgnDbTable, MemoryConsumer
{
    friend struct DgnDb;
    friend struct DgnElement;
    friend struct DgnModel;
    friend struct DgnModels;
    friend struct DgnGeometryPart;
    friend struct ElementHandler;
    friend struct TxnManager;
    friend struct ProgressiveViewFilter;
    friend struct dgn_TxnTable::Element;
    friend struct GeometricElement;

    //! The totals for persistent DgnElements in this DgnDb. These values reflect the current state of the loaded elements.
    struct Totals
    {
        uint32_t m_extant;         //! total number of DgnElements extant (persistent and non-persistent)
        uint32_t m_entries;        //! total number of persistent elements
        uint32_t m_unreferenced;   //! total number of unreferenced persistent elements
        uint64_t m_allocedBytes;   //! total number of bytes of data held by persistent elements
    };

    //! Statistics for element activity in this DgnDb. these values can be reset at any point to gauge "element flux"
    //! (note: the same element may become garbage and then be reclaimed, each such occurrence is reflected here.)
    struct Statistics
    {
        uint32_t m_newElements;    //! number of newly created or loaded elements
        uint32_t m_unReferenced;   //! number of elements that became garbage since last reset
        uint32_t m_reReferenced;   //! number of garbage elements that were referenced
        uint32_t m_purged;         //! number of garbage elements that were purged
    };

    struct CachedSelectStatement : BeSQLite::EC::ECSqlStatement
    {
        friend struct DgnElements;
    private:
        mutable BeAtomic<uint32_t>  m_refCount;
        bool                        m_isInCache;
        BeSQLite::BeDbMutex&        m_mutex;

        CachedSelectStatement(BeSQLite::BeDbMutex& mutex, bool inCache) : m_mutex(mutex), m_isInCache(inCache) { }
    public:
        DEFINE_BENTLEY_NEW_DELETE_OPERATORS

        ~CachedSelectStatement() { }

        uint32_t AddRef() const { return m_refCount.IncrementAtomicPre(); }
        uint32_t GetRefCount() const { return m_refCount.load(); }
        DGNPLATFORM_EXPORT uint32_t Release();
    };

    typedef RefCountedPtr<CachedSelectStatement> CachedSelectStatementPtr;
private:
    struct ElementSelectStatement
    {
        CachedSelectStatementPtr m_statement;
        ECSqlClassParamsCR m_params;
        ElementSelectStatement(CachedSelectStatement* stmt, ECSqlClassParamsCR params) : m_statement(stmt), m_params(params) {}
    };

    struct ClassInfo : ECSqlClassInfo
    {
        CachedSelectStatementPtr    m_selectStmt;
    };

    typedef bmap<DgnClassId, ClassInfo> ClassInfoMap;
    typedef bmap<DgnClassId, ECSqlClassParams> T_ClassParamsMap;

    DgnElementId  m_nextAvailableId;
    struct ElemIdTree* m_tree;
    BeSQLite::StatementCache m_stmts;
    Byte m_snappyFromBuffer[BeSQLite::SnappyReader::SNAPPY_UNCOMPRESSED_BUFFER_SIZE];
    BeSQLite::SnappyFromMemory m_snappyFrom;
    BeSQLite::SnappyToBlob m_snappyTo;
    DgnElementIdSet m_selectionSet;
    mutable BeSQLite::BeDbMutex m_mutex;
    mutable ClassInfoMap m_classInfos;      // information about custom-handled properties 
    mutable T_ClassParamsMap m_classParams; // information about custom-handled properties 
    mutable bmap<DgnClassId, BeSQLite::EC::ECInstanceUpdater*> m_updaterCache; // cached updaters for custom-handled properties

    void OnReclaimed(DgnElementCR);
    void OnUnreferenced(DgnElementCR);
    void Destroy();
    void AddToPool(DgnElementCR) const;
    void SendOnLoadedEvent(DgnElementR elRef) const;
    void FinishUpdate(DgnElementCR replacement, DgnElementCR original);
    DgnElementCPtr LoadElement(DgnElement::CreateParams const& params, bool makePersistent) const;
    DgnElementCPtr LoadElement(DgnElementId elementId, bool makePersistent) const;
    void InitNextId();
    DgnElementCPtr PerformInsert(DgnElementR element, DgnDbStatus&);
    DgnDbStatus PerformDelete(DgnElementCR);
    explicit DgnElements(DgnDbR db);
    ~DgnElements();

    DGNPLATFORM_EXPORT DgnElementCPtr InsertElement(DgnElementR element, DgnDbStatus* stat);
    DGNPLATFORM_EXPORT DgnElementCPtr UpdateElement(DgnElementR element, DgnDbStatus* stat);

    ClassInfo& FindClassInfo(DgnElementCR el) const;
    ElementSelectStatement GetPreparedSelectStatement(DgnElementR el) const;
    BeSQLite::EC::CachedECSqlStatementPtr GetPreparedInsertStatement(DgnElementR el) const;
    BeSQLite::EC::CachedECSqlStatementPtr GetPreparedUpdateStatement(DgnElementR el) const;

    virtual uint64_t _CalculateBytesConsumed() const override {return GetTotalAllocated();}
    virtual uint64_t _Purge(uint64_t memTarget) override;

    BeSQLite::SnappyFromMemory& GetSnappyFrom() {return m_snappyFrom;} // NB: Not to be used during loading of a GeometricElement or GeometryPart!
    BeSQLite::SnappyToBlob& GetSnappyTo() {return m_snappyTo;} // NB: Not to be used during insert or update of a GeometricElement or GeometryPart!

    ECSqlClassParams const& GetECSqlClassParams(DgnClassId) const;

public:
    DGNPLATFORM_EXPORT BeSQLite::CachedStatementPtr GetStatement(Utf8CP sql) const; //!< Get a statement from the element-specific statement cache for this DgnDb @private
    DGNPLATFORM_EXPORT void ChangeMemoryUsed(int32_t delta) const; //! @private
    DGNPLATFORM_EXPORT void DropFromPool(DgnElementCR) const; //! @private

    //! Look up an element in the pool of loaded elements for this DgnDb.
    //! @return A pointer to the element, or nullptr if the is not in the pool.
    //! @note This method will return nullptr if the element is not currently loaded. That does not mean the element doesn't exist in the database.
    DGNPLATFORM_EXPORT DgnElementCP FindElement(DgnElementId id) const;

    //! Query the DgnModelId of the specified DgnElementId.
    DGNPLATFORM_EXPORT DgnModelId QueryModelId(DgnElementId elementId) const;

    // Function to allow apps such as Navigator to try to resolve URIs created in Graphite05 for things like issues and clashes.
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdGraphiteURI(Utf8CP uri) const;

    //! Query for the DgnElementId of the element that has the specified code
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdByCode(DgnCode const& code) const;

    //! Query for the DgnElementId of the element that has the specified code
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdByCode(DgnAuthorityId codeAuthorityId, Utf8StringCR codeValue, Utf8StringCR nameSpace="") const;

    //! Query for the DgnElementId of the element that has the specified code
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdByCode(Utf8CP codeAuthorityName, Utf8StringCR codeValue, Utf8StringCR nameSpace="") const;

    //! Get the total counts for the current state of the pool.
    DGNPLATFORM_EXPORT Totals const& GetTotals() const;

    //! Shortcut to get the Totals.m_allocatedBytes member
    int64_t GetTotalAllocated() const {return GetTotals().m_allocedBytes;}

    //! Get the statistics for the current state of the element pool.
    DGNPLATFORM_EXPORT Statistics GetStatistics() const;

    //! Reset the statistics for the element pool.
    DGNPLATFORM_EXPORT void ResetStatistics();

    //! Create a new, non-persistent element from the supplied ECInstance.
    //! The supplied instance must specify the element's ModelId and Code. It does not have to specify the ElementId/ECInstaceId. Typically, it will not.
    //! @param stat     Optional. If not null, an error status is returned here if the element cannot be created.
    //! @param properties The instance that contains all of the element's business properties
    //! @return a new, non-persistent element if successfull, or an invalid ptr if not.
    //! @note The returned element, if any, is non-persistent. The caller must call the element's Insert method to add it to the bim.
    DGNPLATFORM_EXPORT DgnElementPtr CreateElement(DgnDbStatus* stat, ECN::IECInstanceCR properties);

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

    //! Return the DgnElementId for the root Subject
    DgnElementId GetRootSubjectId() const {return DgnElementId((uint64_t)1LL);}
    //! Return the root Subject
    SubjectCPtr GetRootSubject() const {return Get<Subject>(GetRootSubjectId());}

    //! Get the RepositoryLink for @b this DgnDb
    DgnElementId GetRepositoryLinkId() const {return DgnElementId((uint64_t)16LL);}

    //! Insert a copy of the supplied DgnElement into this DgnDb.
    //! @param[in] element The DgnElement to insert.
    //! @param[in] stat An optional status value. Will be DgnDbStatus::Success if the insert was successful, error status otherwise.
    //! @return RefCountedCPtr to the newly persisted /b copy of /c element. Will be invalid if the insert failed.
    //! @note The element's code must be unique among all elements within the DgnDb, or this method will fail with DgnDbStatus::DuplicateCode.
    template<class T> RefCountedCPtr<T> Insert(T& element, DgnDbStatus* stat=nullptr) {return (T const*) InsertElement(element, stat).get();}

    //! Update the original persistent DgnElement from which the supplied DgnElement was copied.
    //! @param[in] modifiedElement The modified copy of the DgnElement to update.
    //! @param[in] stat An optional status value. Will be DgnDbStatus::Success if the update was successful, error status otherwise.
    //! @return RefCountedCPtr to the modified persistent element. Will be invalid if the update failed.
    //! @note This call returns a RefCountedCPtr to the *original* peristent element (which has now been updated to reflect the changes from
    //! modifiedElement). modifiedElement does *not* become persistent from this call.
    template<class T> RefCountedCPtr<T> Update(T& modifiedElement, DgnDbStatus* stat=nullptr) {return (T const*) UpdateElement(modifiedElement, stat).get();}

    //! Delete a DgnElement from this DgnDb.
    //! @param[in] element The element to delete.
    //! @return DgnDbStatus::Success if the element was deleted, error status otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus Delete(DgnElementCR element);

    //! Delete a DgnElement from this DgnDb by DgnElementId.
    //! @return DgnDbStatus::Success if the element was deleted, error status otherwise.
    //! @note This method is merely a shortcut to #GetElement and then #Delete
    DgnDbStatus Delete(DgnElementId id) {auto el=GetElement(id); return el.IsValid() ? Delete(*el) : DgnDbStatus::NotFound;}

    DgnElementIdSet const& GetSelectionSet() const {return m_selectionSet;}
    DgnElementIdSet& GetSelectionSetR() {return m_selectionSet;}

    //! For all loaded elements, drops any cached graphics associated with the specified viewport.
    //! This is typically invoked by applications when a viewport is closed or its attributes modified such that the cached graphics
    //! no longer reflect its state.
    //! @param[in]      viewport The viewport for which to drop graphics
    DGNPLATFORM_EXPORT void DropGraphicsForViewport(DgnViewportCR viewport);
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
    bool m_preserveOriginalModels;

public:
    DGNPLATFORM_EXPORT ElementCopier(DgnCloneContext& c);

    DgnCloneContext& GetCloneContext() {return m_context;}

    //! Specify if children should be deep-copied or not. The default is yes, deep-copy children.
    void SetCopyChildren(bool b) {m_copyChildren=b;}

    //! Specify if group members should be deep-copied or not. The default is no, do not deep-copy group members.
    void SetCopyGroups(bool b) {m_copyGroups=b;}

    //! Specify if child elements and group members should be copied into the parent/group element's destination model. If not, children and members are copied to their own models. The default is, yes, preserve original models.
    void SetPreserveOriginalModels(bool b) {m_preserveOriginalModels=b;}

    //! Make a persistent copy of a specified Physical element and its children.
    //! This function copies the input element's children, unless you call SetCopyChildren and pass false.
    //! If the input element is a group, this function will optionally copy its group members. See SetCopyGroups.
    //! When copying children, this function will either copy a child into its own model or its parent's model. See SetPreserveOriginalModels.
    //! The same strategy is used to choose the destination model of group members.
    //! @param[out] stat        Optional. If not null, then an error code is stored here in case the copy fails.
    //! @param[in] targetModel  The model where the instance is to be inserted
    //! @param[in] sourceElement The element that is to be copied
    //! @param[in] code         The code to assign to the new element. If invalid, then a code will be generated by the sourceElement's CodeAuthority
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
    //! @return DgnElementId of parent. Will be invalid if there is no parent.
    DGNPLATFORM_EXPORT static DgnElementId GetAssemblyParentId(DgnElementCR el);

    //! Query the DgnDb for members of the assembly for which the input DgnElement is a member.
    //! @return DgnElementIdSet containing the DgnElementIds of assembly elements. Will be empty if not an assembly.
    DGNPLATFORM_EXPORT static DgnElementIdSet GetAssemblyElementIdSet(DgnElementCR el);
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
     bset<DgnElementP> m_elements; // The editable elements in the set. We manage their refcounts as we add and remove them 
     bmap<DgnElementId,DgnElementP> m_ids; // The Elements in the set that have IDs. Child elements will always have IDs. Some top-level elements may not have an Id.

     DGNPLATFORM_EXPORT void EmptyAll();
     DGNPLATFORM_EXPORT void CopyFrom(DgnEditElementCollector const&);

public:
    DGNPLATFORM_EXPORT DgnEditElementCollector();
    DGNPLATFORM_EXPORT DgnEditElementCollector(DgnEditElementCollector const&);
    DGNPLATFORM_EXPORT DgnEditElementCollector& operator=(DgnEditElementCollector const&);
    DGNPLATFORM_EXPORT ~DgnEditElementCollector();

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
    DGNPLATFORM_EXPORT void AddAssembly(DgnElement& el, size_t maxDepth = std::numeric_limits<size_t>::max()) {AddElement(el); AddChildren(el, maxDepth);}

    //! Add an editable copy of the specified element to the collection.
    //! If the collection already contains an element with the same ElementId, then \a el is not added and the existing element is returned.
    //! @param el  The element to be edited
    //! @return The element that is in the collection or nullptr if the element could not be copied for edit.
    DGNPLATFORM_EXPORT DgnElementPtr EditElement(DgnElementCR el) {auto ee = el.CopyForEdit(); if (ee.IsValid()) return AddElement(*ee); else return nullptr;}
    
    //! Add an editable copy of the specified element and its children to the collection.
    //! @param el       The element to be added
    //! @param maxDepth The levels of child elements to add. Pass 1 to add only the immediate children.
    DGNPLATFORM_EXPORT void EditAssembly(DgnElementCR el, size_t maxDepth = std::numeric_limits<size_t>::max()) {auto ee = el.CopyForEdit(); if (ee.IsValid()) AddAssembly(*ee, maxDepth);}

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
    DGNPLATFORM_EXPORT void RemoveAssembly(DgnElementR el, size_t maxDepth = std::numeric_limits<size_t>::max()) {RemoveElement(el); RemoveChildren(el, maxDepth);}

    //! Get the number of elements currently in the collection
    size_t size() {return m_elements.size();}

    //! Get an iterator pointing to the beginning of the collection
    bset<DgnElementP>::const_iterator begin() const {return m_elements.begin();}

    //! Get an iterator pointing to the end of the collection
    bset<DgnElementP>::const_iterator end() const {return m_elements.end();}

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
//! Applies a transform one or more elements
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
