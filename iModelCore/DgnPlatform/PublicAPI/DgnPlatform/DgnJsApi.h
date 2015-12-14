/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnJsApi.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#ifndef _DGN_JS_API_H_
#define _DGN_JS_API_H_

#include <BeJavaScript/BeJavaScript.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/DgnModel.h>
#include <DgnPlatform/GeomJsApi.h>
#include <Logging/bentleylogging.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#define STUB_OUT_SET_METHOD(PROPNAME,PROPTYPE)  void Set ## PROPNAME (PROPTYPE) {BeAssert(false);}

struct JsDgnModel;
typedef JsDgnModel* JsDgnModelP;

struct JsDgnModels;
typedef JsDgnModels* JsDgnModelsP;

struct JsECDbSchemaManager;
typedef JsECDbSchemaManager* JsECDbSchemaManagerP;

struct JsComponentModel;
typedef JsComponentModel* JsComponentModelP;

struct JsECInstance;
typedef JsECInstance* JsECInstanceP;

struct JsECClass;
typedef JsECClass* JsECClassP;

struct JsECProperty;
typedef JsECProperty* JsECPropertyP;

#define JS_ITERATOR_IMPL(JSITCLASS,CPPCOLL) typedef CPPCOLL T_CppColl;\
    T_CppColl::const_iterator m_iter;\
    JSITCLASS(CPPCOLL::const_iterator it) : m_iter(it) {;}

#define JS_COLLECTION_IMPL(JSCOLLCLASS,JSITCLASS) JSITCLASS::T_CppColl m_collection;\
    JSCOLLCLASS(JSITCLASS::T_CppColl const& c) : m_collection(c) {;}\
    JSITCLASS* Begin() {return new JSITCLASS(m_collection.begin());}\
    bool IsValid(JSITCLASS* iter) {return iter && iter->m_iter != m_collection.end();}\
    bool ToNext(JSITCLASS* iter) {if (nullptr == iter) return false; ++(iter->m_iter); return IsValid(iter);}

template<typename T>
JsECInstanceP getCustomAttribute(T* ccontainer, Utf8StringCR className)
    {
    if (nullptr == ccontainer)
        return nullptr;
    ECN::IECInstancePtr ca = ccontainer->GetCustomAttribute(className);
    return ca.IsValid()? new JsECInstance(*ca): nullptr;
    }


//=======================================================================================
// Needed by generated callbacks to construct instances of wrapper classes.
// @bsiclass                                                    Sam/Steve.Wilson    7/15
//=======================================================================================
struct RefCountedBaseWithCreate : public RefCounted <IRefCounted>
{
    template <typename T, typename... Arguments>
    static RefCountedPtr<T> Create (Arguments&&... arguments)
        {
        return new T (std::forward<Arguments> (arguments)...);
        };

    DEFINE_BENTLEY_NEW_DELETE_OPERATORS
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      10/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS (Bentley.Dgn)
enum class ECPropertyPrimitiveType : uint32_t
    {
    Unknown                   = 0,
    Binary                    = 0x0101,//ECN::PRIMITIVETYPE_Binary,
    Boolean                   = 0x0201,//ECN::PRIMITIVETYPE_Boolean,
    DateTime                  = 0x0301,//ECN::PRIMITIVETYPE_DateTime,
    Double                    = 0x0401,//ECN::PRIMITIVETYPE_Double,
    Integer                   = 0x0501,//ECN::PRIMITIVETYPE_Integer,
    Long                      = 0x0601,//ECN::PRIMITIVETYPE_Long,
    Point2D                   = 0x0701,//ECN::PRIMITIVETYPE_Point2D,
    Point3D                   = 0x0801,//ECN::PRIMITIVETYPE_Point3D,
    String                    = 0x0901,//ECN::PRIMITIVETYPE_String,
    IGeometry                 = 0x0a01 //ECN::PRIMITIVETYPE_IGeometry
    };

//=======================================================================================
//! Logging Severity 
// @bsiclass                                                    Sam.Wilson      10/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS (Bentley.Dgn)
enum class LoggingSeverity : uint32_t
    {
    // *** WARNING: Keep this consistent with DgnPlatformLib::ScriptAdmin::LoggingSeverity
    Fatal   = 0, 
    Error   = 1, 
    Warning = 2, 
    Info    = 3, 
    Debug   = 4, 
    Trace   = 5
    };

//=======================================================================================
//! Access to the message log
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct Logging : RefCountedBaseWithCreate // ***  NEEDS WORK: It should not be necessary to derive from RefCountedBase, since I suppress my constructor. This is a bug in BeJavaScript that should be fixed.
    {
    //! Set the severity level for the specified category
    //! @param category     The logging category
    //! @param severity     The minimum severity to display. Note that messages will not be logged if their severity is below this level.
    static void SetSeverity(Utf8StringCR category, LoggingSeverity severity);

    //! Test if the specified severity level is enabled for the specified category
    //! @param category     The logging category
    //! @param severity     The severity of the message. Note that the message will not be logged if 'severity' is below the current logging severity level
    static bool IsSeverityEnabled(Utf8StringCR category, LoggingSeverity severity);

    //! Send a message to the log
    //! @param category     The logging category
    //! @param severity     The severity of the message. Note that the message will not be logged if \a severity is below the severity level set by calling SetSeverity
    //! @param message      The message to log
    static void Message(Utf8StringCR category, LoggingSeverity severity, Utf8StringCR message);
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct Script : RefCountedBaseWithCreate // ***  NEEDS WORK: It should not be necessary to derive from RefCountedBase, since I suppress my constructor. This is a bug in BeJavaScript that should be fixed.
{
    //! Make sure the that specified library is loaded
    //! @param libName  The name of the library that is to be loaded
    static void ImportLibrary (Utf8StringCR libName);

    //! Report an error. An error is more than a message. The platform is will treat it as an error. For example, the platform may terminate the current command.
    //! @param description  A description of the error
    static void ReportError(Utf8StringCR description);
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnObjectId : RefCountedBaseWithCreate
{
    uint64_t m_id;
    JsDgnObjectId() : m_id(0) {;}
    explicit JsDgnObjectId(uint64_t v) : m_id(v) {;}
    bool IsValid() {return 0 != m_id;}
    bool Equals(JsDgnObjectId* rhs) {return rhs && rhs->m_id == m_id;}
};

typedef JsDgnObjectId* JsDgnObjectIdP;

struct JsDgnObjectIdSetIterator : RefCountedBaseWithCreate
    {
    JS_ITERATOR_IMPL(JsDgnObjectIdSetIterator, bset<uint64_t>)
    };

typedef JsDgnObjectIdSetIterator* JsDgnObjectIdSetIteratorP;

struct JsDgnObjectIdSet : RefCountedBaseWithCreate
{
    JsDgnObjectIdSet() {;}
    JsDgnObjectIdSet(DgnCategoryIdSet const& ids) {for (auto id: ids) m_collection.insert(id.GetValueUnchecked());}
    JsDgnObjectIdSet(DgnElementIdSet const& ids) {for (auto id: ids) m_collection.insert(id.GetValueUnchecked());}

    JS_COLLECTION_IMPL(JsDgnObjectIdSet,JsDgnObjectIdSetIterator)

    int Size() {return (int)m_collection.size();}
    void Clear() {m_collection.clear();}
    void Insert(JsDgnObjectIdP id) {if (id && id->m_id) m_collection.insert(id->m_id);}
    JsDgnObjectIdP GetId(JsDgnObjectIdSetIteratorP iter) {return IsValid(iter)? new JsDgnObjectId(*iter->m_iter): nullptr;}
};

typedef JsDgnObjectIdSet* JsDgnObjectIdSetP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsAuthorityIssuedCode : RefCountedBaseWithCreate
{
    AuthorityIssuedCode m_code;
    explicit JsAuthorityIssuedCode(AuthorityIssuedCode const& c) : m_code(c) {;}

    Utf8String GetValue() const {return m_code.GetValue();}
    Utf8String GetNamespace() const {return m_code.GetNamespace();}
    JsDgnObjectIdP GetAuthority() {return new JsDgnObjectId(m_code.GetAuthority().GetValueUnchecked());}

    STUB_OUT_SET_METHOD(Value,Utf8String)
    STUB_OUT_SET_METHOD(Namespace,Utf8String)
    STUB_OUT_SET_METHOD(Authority,JsDgnObjectIdP)
};

typedef JsAuthorityIssuedCode* JsAuthorityIssuedCodeP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnDb : RefCountedBaseWithCreate
{
    DgnDbPtr m_db;
    explicit JsDgnDb(DgnDbR db) : m_db(&db) {;}

    JsDgnModelsP GetModels();
    JsECDbSchemaManagerP GetSchemas();

    STUB_OUT_SET_METHOD(Models,JsDgnModelsP)
    STUB_OUT_SET_METHOD(Schemas,JsECDbSchemaManagerP)
};

typedef JsDgnDb* JsDgnDbP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnElement : RefCountedBaseWithCreate
{
    DgnElementPtr m_el;

    JsDgnElement(DgnElementR el) : m_el(&el) {;}

    JsDgnObjectIdP GetElementId() {return new JsDgnObjectId(m_el->GetElementId().GetValueUnchecked());}
    JsAuthorityIssuedCodeP GetCode() const {return new JsAuthorityIssuedCode(m_el->GetCode());}
    JsDgnModelP GetModel();
    void SetModel(JsDgnModelP) {BeAssert(false);} // *** WIP_SCRIPT - this should be a read-only property
    int32_t Insert() {return m_el.IsValid()? m_el->Insert().IsValid()? 0: -1: -2;}
    int32_t Update() {return m_el.IsValid()? m_el->Update().IsValid()? 0: -1: -2;}
    void SetParent(JsDgnElement* parent) {if (m_el.IsValid() && (nullptr != parent)) m_el->SetParentId(parent->m_el->GetElementId());}

    STUB_OUT_SET_METHOD(ElementId,JsDgnObjectIdP)
    STUB_OUT_SET_METHOD(Code,JsAuthorityIssuedCodeP)
};

typedef JsDgnElement* JsDgnElementP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsPhysicalElement : JsDgnElement
{
    JsPhysicalElement(PhysicalElementR el) : JsDgnElement(el) {;}

    static JsPhysicalElement* Create(JsDgnModelP model, JsDgnObjectIdP categoryId, Utf8StringCR elementClassName);
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnModel : RefCountedBaseWithCreate
{
    DgnModelPtr m_model;

    ComponentModel* ToDgnComponentModel() {return dynamic_cast<ComponentModel*>(m_model.get());}

    JsDgnModel(DgnModelR m) : m_model(&m) {;}

    JsDgnObjectIdP GetModelId() {return new JsDgnObjectId(m_model->GetModelId().GetValueUnchecked());}
    JsAuthorityIssuedCodeP GetCode() const {return new JsAuthorityIssuedCode(m_model->GetCode());}
    JsDgnDbP GetDgnDb() {return new JsDgnDb(m_model->GetDgnDb());}
    void DeleteAllElements();
    static JsAuthorityIssuedCodeP CreateModelCode(Utf8StringCR name) {return new JsAuthorityIssuedCode(DgnModel::CreateModelCode(name));}

    JsComponentModelP ToComponentModel();

    STUB_OUT_SET_METHOD(ModelId,JsDgnObjectIdP)
    STUB_OUT_SET_METHOD(Code,JsAuthorityIssuedCodeP)
    STUB_OUT_SET_METHOD(DgnDb,JsDgnDbP)
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsPlacement3d : RefCountedBaseWithCreate
{
    Placement3d m_placement;
    JsPlacement3d() {;}
    JsPlacement3d(JsDPoint3dP origin, JsYawPitchRollAnglesP angles) : m_placement(origin? origin->Get(): DPoint3d::FromZero(), angles? angles->GetYawPitchRollAngles(): YawPitchRollAngles(), ElementAlignedBox3d()) {;}
    JsPlacement3d(Placement3d const& p) : m_placement(p) {;}

    JsDPoint3dP GetOrigin() const {return new JsDPoint3d(m_placement.GetOrigin());}
    void SetOrigin(JsDPoint3dP p) {m_placement.GetOriginR() = p->Get();}
    JsYawPitchRollAnglesP GetAngles() const {return new JsYawPitchRollAngles(m_placement.GetAngles());}
    void SetAngles(JsYawPitchRollAnglesP p) {m_placement.GetAnglesR() = p->GetYawPitchRollAngles();}
};

typedef JsPlacement3d* JsPlacement3dP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsComponentModel : JsDgnModel
{
    JsComponentModel(ComponentModel& m) : JsDgnModel(m) {;}

    Utf8String GetName() {return m_model.IsValid()? m_model->GetCode().GetValue(): "";}
    JsDgnElement* MakeInstance(JsDgnModelP targetModel, Utf8StringCR capturedSolutionName, Utf8StringCR paramsJSON, JsAuthorityIssuedCodeP code);
    void DeleteAllElements();
    static JsComponentModelP FindModelByName(JsDgnDbP db, Utf8StringCR name) {auto cm = ComponentModel::FindModelByName(*db->m_db, name); return cm.IsValid()? new JsComponentModel(*cm): nullptr; }

    STUB_OUT_SET_METHOD(Name,Utf8String)
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnModels : RefCountedBaseWithCreate
{
    DgnModels& m_models;

    JsDgnModels(DgnModels& m) : m_models(m) {;}

    JsDgnObjectIdP QueryModelId(JsAuthorityIssuedCodeP code) {return new JsDgnObjectId(m_models.QueryModelId(code->m_code).GetValueUnchecked());}
    JsDgnModelP GetModel(JsDgnObjectIdP mid) {auto model = m_models.GetModel(DgnModelId(mid->m_id)); return model.IsValid()? new JsDgnModel(*model): nullptr;}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsElementGeometryBuilder : RefCountedBaseWithCreate
{
    ElementGeometryBuilderPtr m_builder;

    JsElementGeometryBuilder(JsDgnElementP el, JsDPoint3dP o, JsYawPitchRollAnglesP angles);
    ~JsElementGeometryBuilder() {}

    void Append(JsSolidPrimitiveP solid) {if (solid && solid->GetISolidPrimitivePtr().IsValid()) m_builder->Append(*solid->GetISolidPrimitivePtr());}
    BentleyStatus SetGeomStreamAndPlacement (JsDgnElementP el) {return m_builder->SetGeomStreamAndPlacement(*el->m_el->ToGeometrySourceP());}
};
typedef JsElementGeometryBuilder* JsElementGeometryBuilderP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnCategory : RefCountedBaseWithCreate
{
    DgnCategoryCPtr m_category;

    JsDgnCategory(DgnCategoryCR cat) : m_category(&cat) {;}

    JsDgnDbP GetDgnDb() {return m_category.IsValid()? new JsDgnDb(m_category->GetDgnDb()): nullptr;}
    JsDgnObjectIdP GetCategoryId() {return m_category.IsValid()? new JsDgnObjectId(m_category->GetCategoryId().GetValueUnchecked()): nullptr;}
    JsDgnObjectIdP GetDefaultSubCategoryId() {return m_category.IsValid()? new JsDgnObjectId(m_category->GetDefaultSubCategoryId().GetValueUnchecked()): nullptr;}
    Utf8String GetCategoryName() {return m_category.IsValid()? m_category->GetCategoryName(): "";}
    static JsDgnObjectIdP QueryCategoryId(Utf8StringCR name, JsDgnDbP db) {return (db && db->m_db.IsValid())? new JsDgnObjectId(DgnCategory::QueryCategoryId(name, *db->m_db).GetValueUnchecked()): nullptr;}
    static JsDgnCategory* QueryCategory(JsDgnObjectIdP id, JsDgnDbP db) 
        {
        if (!db || !db->m_db.IsValid())
            return nullptr;
        auto cat = DgnCategory::QueryCategory(DgnCategoryId(id->m_id), *db->m_db);
        return cat.IsValid()? new JsDgnCategory(*cat): nullptr;
        }
    static JsDgnObjectIdSetP QueryCategories(JsDgnDbP db)
        {
        if (!db || !db->m_db.IsValid())
            return nullptr;
        return new JsDgnObjectIdSet(DgnCategory::QueryCategories(*db->m_db));
        }

    STUB_OUT_SET_METHOD(DgnDb,JsDgnDbP)
    STUB_OUT_SET_METHOD(CategoryId,JsDgnObjectIdP)
    STUB_OUT_SET_METHOD(DefaultSubCategoryId,JsDgnObjectIdP)
    STUB_OUT_SET_METHOD(CategoryName,Utf8String)

};
typedef JsDgnCategory* JsDgnCategoryP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECClassCollectionIterator : RefCountedBaseWithCreate
    {
    JS_ITERATOR_IMPL(JsECClassCollectionIterator, ECN::ECBaseClassesList)
    };

typedef JsECClassCollectionIterator* JsECClassCollectionIteratorP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECClassCollection : RefCountedBaseWithCreate
{
    JS_COLLECTION_IMPL(JsECClassCollection,JsECClassCollectionIterator)
    JsECClassP GetECClass(JsECClassCollectionIteratorP iter);
};

typedef JsECClassCollection* JsECClassCollectionP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECPropertyCollectionIterator : RefCountedBaseWithCreate
{
    JS_ITERATOR_IMPL(JsECPropertyCollectionIterator, ECN::ECPropertyIterable)
};

typedef JsECPropertyCollectionIterator* JsECPropertyCollectionIteratorP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECPropertyCollection : RefCountedBaseWithCreate
{
    JS_COLLECTION_IMPL(JsECPropertyCollection,JsECPropertyCollectionIterator)
    JsECPropertyP GetECProperty(JsECPropertyCollectionIteratorP iter);
};

typedef JsECPropertyCollection* JsECPropertyCollectionP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECClass : RefCountedBaseWithCreate
{
    ECN::ECClassCP m_ecClass;

    JsECClass(ECN::ECClassCR c) : m_ecClass(&c) {;}

    Utf8String GetName() {return m_ecClass? m_ecClass->GetName(): "";}
    JsECClassCollectionP GetBaseClasses() const {return m_ecClass? new JsECClassCollection(m_ecClass->GetBaseClasses()): nullptr;}
    JsECClassCollectionP GetDerivedClasses() const {return m_ecClass? new JsECClassCollection(m_ecClass->GetDerivedClasses()): nullptr;}
    JsECPropertyCollectionP GetProperties() const {return m_ecClass? new JsECPropertyCollection(m_ecClass->GetProperties()): nullptr;}

    JsECInstanceP GetCustomAttribute(Utf8StringCR className) {return getCustomAttribute(m_ecClass, className);}

    STUB_OUT_SET_METHOD(Name,Utf8String)
    STUB_OUT_SET_METHOD(Properties,JsECPropertyCollectionP)
    STUB_OUT_SET_METHOD(BaseClasses,JsECClassCollectionP)
    STUB_OUT_SET_METHOD(DerivedClasses,JsECClassCollectionP)
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECSchema : RefCountedBaseWithCreate
{
    ECN::ECSchemaCP m_ecSchema;

    JsECSchema(ECN::ECSchemaCR c) : m_ecSchema(&c) {;}

    Utf8String GetName() {return m_ecSchema? m_ecSchema->GetName(): "";}

    JsECClassP GetECClass(Utf8StringCR cls) 
        {
        auto eccls = m_ecSchema->GetClassCP(cls.c_str());
        return eccls? new JsECClass(*eccls): nullptr;
        }

    STUB_OUT_SET_METHOD(Name,Utf8String)
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECDbSchemaManager : RefCountedBaseWithCreate
{
    BeSQLite::EC::ECDbSchemaManager const& m_schemas;

    JsECDbSchemaManager(BeSQLite::EC::ECDbSchemaManager const& schemas) : m_schemas(schemas) {;}

    JsECClassP GetECClass(Utf8StringCR ns, Utf8StringCR cls) 
        {
        auto eccls = m_schemas.GetECClass(ns.c_str(), cls.c_str());
        return eccls? new JsECClass(*eccls): nullptr;
        }
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECProperty : RefCountedBaseWithCreate
{
    ECN::ECPropertyCP m_property;

    JsECProperty(ECN::ECPropertyCR prop) : m_property(&prop) {;}

    Utf8String GetName() {return m_property? m_property->GetName(): "";}
    bool GetIsPrimitive() const {return m_property && m_property->GetIsPrimitive();}
    JsECInstanceP GetCustomAttribute(Utf8StringCR className) {return getCustomAttribute(m_property, className);}
    
    STUB_OUT_SET_METHOD(Name,Utf8String)
    STUB_OUT_SET_METHOD(IsPrimitive,bool)
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsPrimitiveECProperty : JsECProperty
{
    JsPrimitiveECProperty(ECN::PrimitiveECPropertyCR prop) : JsECProperty(prop) {;}

    ECPropertyPrimitiveType GetType() {return (m_property && m_property->GetIsPrimitive())? (ECPropertyPrimitiveType)m_property->GetAsPrimitiveProperty()->GetType(): ECPropertyPrimitiveType::Unknown;}

    STUB_OUT_SET_METHOD(Type,ECPropertyPrimitiveType)
};

typedef JsPrimitiveECProperty* JsPrimitiveECPropertyP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECValue : RefCountedBaseWithCreate
{
    ECN::ECValue m_value;

    JsECValue(ECN::ECValueCR v) : m_value(v) {;}
    bool GetIsPrimitive() const {return m_value.IsPrimitive();}
    bool GetIsNull() const {return m_value.IsNull();}
    ECPropertyPrimitiveType GetPrimitiveType() const {return (ECPropertyPrimitiveType)m_value.GetPrimitiveType();}
    Utf8String GetString() const {return m_value.ToString();}
    int32_t GetInteger() const {return m_value.GetInteger();}
    double GetDouble() const {return m_value.GetDouble();}

    STUB_OUT_SET_METHOD(PrimitiveType,ECPropertyPrimitiveType)
    STUB_OUT_SET_METHOD(IsPrimitive,bool)
    STUB_OUT_SET_METHOD(IsNull,bool)
    STUB_OUT_SET_METHOD(String,Utf8StringCR)
    STUB_OUT_SET_METHOD(Integer,int32_t)
    STUB_OUT_SET_METHOD(Double,double)
};

typedef JsECValue* JsECValueP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECInstance : RefCountedBaseWithCreate
{
    ECN::IECInstancePtr m_instance;

    JsECInstance(ECN::IECInstanceR i) : m_instance(&i) {;}

    JsECClassP GetClass() {return m_instance.IsValid()? new JsECClass(m_instance->GetClass()): nullptr;} 
    JsECValueP GetValue(Utf8StringCR propertyName) 
        {
        if (!m_instance.IsValid())
            return nullptr;
        ECN::ECValue v;
        if (ECN::ECObjectsStatus::Success != m_instance->GetValue(v, propertyName.c_str()))
            return nullptr;
        return new JsECValue(v);
        }

    STUB_OUT_SET_METHOD(Class,JsECClassP)
};

typedef JsECInstance* JsECInstanceP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct DgnJsApi : DgnPlatformLib::Host::ScriptAdmin::ScriptLibraryImporter
{
    BeJsContextR m_context;

    DgnJsApi(BeJsContextR);
    ~DgnJsApi();

    void _ImportScriptLibrary(BeJsContextR, Utf8CP) override;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _DGN_JS_API_H_

