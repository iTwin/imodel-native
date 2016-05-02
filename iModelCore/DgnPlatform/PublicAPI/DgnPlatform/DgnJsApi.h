/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnJsApi.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

BEGIN_BENTLEY_DGN_NAMESPACE

#define STUB_OUT_SET_METHOD(PROPNAME,PROPTYPE)  void Set ## PROPNAME (PROPTYPE) {BeAssert(false);}

#define VALUE_TYPE_GET(TYPE,NAME) TYPE Get##NAME () {return m_value.Get##NAME();}
#define VALUE_TYPE_SET(TYPE,NAME) void Set##NAME (TYPE v) {m_value.Set##NAME(v);}

#define VALUE_TYPE_GET_SET(TYPE,NAME) VALUE_TYPE_GET(TYPE,NAME) VALUE_TYPE_SET(TYPE,NAME)

#define VALUE_TYPE_GET_CAST(JSTYPE,NAME) JSTYPE Get##NAME () {return (JSTYPE)m_value.Get##NAME();}
#define VALUE_TYPE_SET_CAST(JSTYPE,CPPTYPE,NAME) void Set##NAME (JSTYPE v) {m_value.Set##NAME((CPPTYPE)(v));}
#define VALUE_TYPE_GET_SET_CAST(JSTYPE,CPPTYPE,NAME) VALUE_TYPE_GET_CAST(JSTYPE,NAME) VALUE_TYPE_SET_CAST(JSTYPE,CPPTYPE,NAME)

#define DGNJSAPI_DGNSCRIPT_THROW(X,D) DgnPlatformLib::GetHost().GetScriptAdmin()._ThrowException(X,D)

#define DGNJSAPI_VALIDATE_ARGS_VOID(EXPR)   {if (!(EXPR)) {DGNJSAPI_DGNSCRIPT_THROW("Args",#EXPR); return;}}
#define DGNJSAPI_VALIDATE_ARGS_ERROR(EXPR)  {if (!(EXPR)) {DGNJSAPI_DGNSCRIPT_THROW("Args",#EXPR); return BSIERROR;}}
#define DGNJSAPI_VALIDATE_ARGS_NULL(EXPR)   {if (!(EXPR)) {DGNJSAPI_DGNSCRIPT_THROW("Args",#EXPR); return nullptr;}}
#define DGNJSAPI_VALIDATE_ARGS(EXPR,RETVAL) {if (!(EXPR)) {DGNJSAPI_DGNSCRIPT_THROW("Args",#EXPR); return (RETVAL);}}

#define DGNJSAPI_IS_VALID_JSOBJ(OBJ) ((OBJ) && (OBJ)->IsValid())
#define DGNJSAPI_IS_VALID_JSELEMENT_PLACEHOLDER(OBJ) DGNJSAPI_IS_VALID_JSOBJ(OBJ)

#define DGNJSAPI_ASSERT_ABSTRACT BeAssert(false && "This is just a placeholder for an interface");

struct JsDgnDb;
typedef JsDgnDb* JsDgnDbP;

struct JsDgnModel;
typedef JsDgnModel* JsDgnModelP;

struct JsDgnModels;
typedef JsDgnModels* JsDgnModelsP;

struct JsECDbSchemaManager;
typedef JsECDbSchemaManager* JsECDbSchemaManagerP;

struct JsComponentModel;
typedef JsComponentModel* JsComponentModelP;

struct JsComponentDef;
typedef JsComponentDef* JsComponentDefP;

struct JsECInstance;
typedef JsECInstance* JsECInstanceP;

struct JsECValue;
typedef JsECValue* JsECValueP;

struct JsAdHocJsonPropertyValue;
typedef JsAdHocJsonPropertyValue* JsAdHocJsonPropertyValueP;

struct JsECClass;
typedef JsECClass* JsECClassP;

struct JsECSchema;
typedef JsECSchema* JsECSchemaP;

struct JsECProperty;
typedef JsECProperty* JsECPropertyP;

struct JsPrimitiveECProperty;
typedef JsPrimitiveECProperty* JsPrimitiveECPropertyP;

struct JsDgnElements;
typedef JsDgnElements* JsDgnElementsP;

struct JsDgnCategory;
typedef JsDgnCategory* JsDgnCategoryP;

struct JsPlacement3d;
typedef JsPlacement3d* JsPlacement3dP;

struct JsGeometrySource;
typedef JsGeometrySource* JsGeometrySourceP;

struct JsGeometrySource3d;
typedef JsGeometrySource3d* JsGeometrySource3dP;

struct JsGeometrySource2d;
typedef JsGeometrySource2d* JsGeometrySource2dP;

struct JsGeometryCollection;
typedef JsGeometryCollection* JsGeometryCollectionP;

struct JsGeometryBuilder;
typedef JsGeometryBuilder* JsGeometryBuilderP;

#define JS_ITERATOR_IMPL(JSITCLASS,CPPCOLL) typedef CPPCOLL T_CppColl;\
    T_CppColl::const_iterator m_iter;\
    JSITCLASS(CPPCOLL::const_iterator it) : m_iter(it) {;}

#define JS_COLLECTION_IMPL(JSCOLLCLASS,JSITCLASS) JSITCLASS::T_CppColl m_collection;\
    JSCOLLCLASS(JSITCLASS::T_CppColl const& c) : m_collection(c) {;}\
    JSITCLASS* Begin() {return new JSITCLASS(m_collection.begin());}\
    bool IsValid(JSITCLASS* iter) {return iter && iter->m_iter != m_collection.end();}\
    bool ToNext(JSITCLASS* iter) {if (nullptr == iter) return false; ++(iter->m_iter); return IsValid(iter);}

//=======================================================================================
// Needed by generated callbacks to construct instances of wrapper classes.
// @bsiclass                                                    Sam/Steve.Wilson    7/15
//=======================================================================================
struct RefCountedBaseWithCreate : BeProjectedRefCounted
{
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      10/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS(Bentley.Dgn)
enum class BeSQLiteDbResult : uint32_t
    {
    BE_SQLITE_OK = 0,
    BE_SQLITE_ERROR = 1,
    BE_SQLITE_ROW = 100,
    BE_SQLITE_DONE = 101
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
	static int32_t LoadScript(JsDgnDbP db, Utf8StringCR scriptName, bool forceReload);

    static void ImportLibrary (Utf8StringCR libName);

    static void ReportError(Utf8StringCR description);

    static void BeginDisposeContext();
    static void EndDisposeContext();

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
    Utf8String ToString() const {return Utf8PrintfString("%" PRIu64, m_id); }
    void FromString(Utf8StringCR str) 
        {
        if (1 != sscanf(str.c_str(), "%" PRIu64, &m_id))
            DGNJSAPI_DGNSCRIPT_THROW("Args", str.c_str());
        }
};

typedef JsDgnObjectId* JsDgnObjectIdP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnObjectIdSetIterator : RefCountedBaseWithCreate
    {
    JS_ITERATOR_IMPL(JsDgnObjectIdSetIterator, bset<uint64_t>)
    };

typedef JsDgnObjectIdSetIterator* JsDgnObjectIdSetIteratorP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
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
    DgnCode m_code;
    explicit JsAuthorityIssuedCode(DgnCode const& c) : m_code(c) {;}
    bool IsValid() const {return m_code.IsValid();}

    Utf8String GetValue() const {return m_code.GetValue();}
    Utf8String GetNamespace() const {return m_code.GetNamespace();}
    JsDgnObjectIdP GetAuthority() {return new JsDgnObjectId(m_code.GetAuthority().GetValueUnchecked());}

    STUB_OUT_SET_METHOD(Value,Utf8String)
    STUB_OUT_SET_METHOD(Namespace,Utf8String)
    STUB_OUT_SET_METHOD(Authority,JsDgnObjectIdP)
};

typedef JsAuthorityIssuedCode* JsAuthorityIssuedCodeP;

struct JsECSqlArrayValue;

//=======================================================================================
// @bsiclass                                                Ramanujam.Raman      04/16
//=======================================================================================
struct JsECSqlValue : RefCountedBaseWithCreate
    {
    BeSQLite::EC::IECSqlValue const* m_value;

    JsECSqlValue(BeSQLite::EC::IECSqlValue const* value) : m_value(value) { ; }

    Utf8String GetText() { return m_value->GetText(); }
    Utf8String GetDateTime() { return m_value->GetDateTime().ToUtf8String(); }
    double GetDouble() { return m_value->GetDouble(); }
    JsDPoint3dP GetDPoint3d() { return new JsDPoint3d(m_value->GetPoint3D()); }
    int32_t GetInt() { return m_value->GetInt(); }
    JsDgnObjectIdP GetId() { return new JsDgnObjectId(m_value->GetUInt64()); }
    JsECSqlArrayValue* GetArray();
    };

typedef JsECSqlValue* JsECSqlValueP;

//=======================================================================================
// @bsiclass                                                Ramanujam.Raman      04/16
//=======================================================================================
struct JsECSqlArrayValueIterator : RefCountedBaseWithCreate
    {
    BeSQLite::EC::IECSqlArrayValue::const_iterator m_iter;
    JsECSqlArrayValueIterator(BeSQLite::EC::IECSqlArrayValue::const_iterator it) : m_iter(it) { ; }
    };

typedef JsECSqlArrayValueIterator* JsECSqlArrayValueIteratorP;

//=======================================================================================
// @bsiclass                                                Ramanujam.Raman      04/16
//=======================================================================================
struct JsECSqlArrayValue : RefCountedBaseWithCreate
    {
    BeSQLite::EC::IECSqlArrayValue const& m_arrayValue;

    JsECSqlArrayValue(BeSQLite::EC::IECSqlArrayValue const& arrayValue) : m_arrayValue(arrayValue) { ; }

    JsECSqlArrayValueIteratorP Begin() { return new JsECSqlArrayValueIterator(m_arrayValue.begin()); }
    bool IsValid(JsECSqlArrayValueIteratorP iter) { return iter && iter->m_iter != m_arrayValue.end(); }
    bool ToNext(JsECSqlArrayValueIteratorP iter) { if (nullptr == iter) return false; ++(iter->m_iter); return IsValid(iter); }

    JsECSqlValueP GetValue(JsECSqlArrayValueIteratorP iter) { return IsValid(iter) ? new JsECSqlValue(*iter->m_iter) : nullptr; }
    };

typedef JsECSqlArrayValue* JsECSqlArrayValueP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsPreparedECSqlStatement : RefCountedBaseWithCreate
{
    BeSQLite::EC::CachedECSqlStatementPtr m_stmt;

    JsPreparedECSqlStatement(BeSQLite::EC::CachedECSqlStatement& stmt) : m_stmt(&stmt) {;}
    bool IsValid() const { return m_stmt.IsValid(); }
    BentleyStatus CheckValueIndexInRange(int32_t idx);

    void BindId(int parameterIndex, JsDgnObjectIdP value);
    void BindText(int parameterIndex, Utf8StringCR value);
    void BindInt(int parameterIndex, int32_t value);
    void BindDouble(int parameterIndex, double value);
    void BindDPoint3d(int parameterIndex, JsDPoint3dP value);
    void BindDRange3d(int parameterIndex, JsDRange3dP value);

    BeSQLiteDbResult Step();
    int32_t GetParameterIndex(Utf8StringCR colName);
    Utf8String GetValueText(int32_t col);
    Utf8String GetValueDateTime(int32_t col);
    double GetValueDouble(int32_t col);
    JsDPoint3dP GetValueDPoint3d(int32_t col);
    JsDRange3dP GetValueDRange3d(int32_t col);
    int32_t GetValueInt(int32_t col);
    JsDgnObjectIdP GetValueId(int32_t col);
    JsECSqlArrayValueP GetValueArray(int32_t col);
};

typedef JsPreparedECSqlStatement* JsPreparedECSqlStatementP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnDb : RefCountedBaseWithCreate
{
    DgnDbPtr m_db;
    explicit JsDgnDb(DgnDbR db) : m_db(&db) {;}
    bool IsValid() const {return m_db.IsValid();}

    JsDgnModelsP GetModels();
    JsECDbSchemaManagerP GetSchemas();
    JsDgnElementsP GetElements();
    JsPreparedECSqlStatementP GetPreparedECSqlSelectStatement(Utf8StringCR ecsql);
    int32_t SaveChanges();

    STUB_OUT_SET_METHOD(Models, JsDgnModelsP)
    STUB_OUT_SET_METHOD(Elements, JsDgnElementsP)
    STUB_OUT_SET_METHOD(Schemas,JsECDbSchemaManagerP)
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnElement : RefCountedBaseWithCreate
{
    static size_t s_count;
    DgnElementPtr m_el;

protected:
    JsDgnElement(){}
public:
    JsDgnElement(DgnElementR el) : m_el(&el) {++s_count;}
    ~JsDgnElement() {--s_count;}

    bool IsValid() const {return m_el.IsValid();}
    JsDgnObjectIdP GetElementId() 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsDgnObjectId(m_el->GetElementId().GetValueUnchecked());
        }
    JsAuthorityIssuedCodeP GetCode() const
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsAuthorityIssuedCode(m_el->GetCode());
        }
    JsDgnModelP GetModel();
    JsECClassP GetElementClass();
    int32_t Insert();
    int32_t Update();
    void SetParent(JsDgnElement* parent) 
        {
        DGNJSAPI_VALIDATE_ARGS_VOID(IsValid());
        if (nullptr == parent)
            m_el->SetParentId(DgnElementId());
        else
            {
            DGNJSAPI_VALIDATE_ARGS_VOID(DGNJSAPI_IS_VALID_JSOBJ(parent));
            m_el->SetParentId(parent->m_el->GetElementId());
            }
        }
    JsECValueP GetProperty(Utf8StringCR);
    int32_t SetProperty(Utf8StringCR, JsECValueP);

    bool ContainsUserProperty(Utf8StringCR name) const;
    JsAdHocJsonPropertyValueP GetUserProperty(Utf8StringCR name) const;
    void RemoveUserProperty(Utf8StringCR name) const;

    virtual JsGeometrySourceP ToGeometrySource();
    virtual JsGeometrySource3dP ToGeometrySource3d();
    virtual JsGeometrySource2dP ToGeometrySource2d();

    static JsDgnElement* Create(JsDgnModelP model, Utf8StringCR elementClassName);

    STUB_OUT_SET_METHOD(Model, JsDgnModelP)
    STUB_OUT_SET_METHOD(ElementId,JsDgnObjectIdP)
    STUB_OUT_SET_METHOD(Code,JsAuthorityIssuedCodeP)
    STUB_OUT_SET_METHOD(ElementClass, JsECClassP)
};

typedef JsDgnElement* JsDgnElementP;

//=======================================================================================
// Base class for projections of geometry-related classes and interfaces.
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsGeometricElementBase : JsDgnElement
    {
protected:
    JsGeometricElementBase(DgnElement& el) : JsDgnElement(el) {}

public:
    JsDgnObjectIdP GetCategoryId() const
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsDgnObjectId(m_el->ToGeometrySource()->GetCategoryId().GetValue());
        }

    int32_t SetCategoryId(JsDgnObjectIdP catid)
        {
        DGNJSAPI_VALIDATE_ARGS(IsValid() && DGNJSAPI_IS_VALID_JSOBJ(catid), -1);
        return (int32_t)m_el->ToGeometrySourceP()->SetCategoryId(DgnCategoryId(catid->m_id));
        }

    JsGeometryCollectionP GetGeometry() const;
    STUB_OUT_SET_METHOD(Geometry, JsGeometryCollectionP)

    JsPlacement3dP GetPlacement() const;
    STUB_OUT_SET_METHOD(Placement, JsPlacement3dP)

    int32_t Transform(JsTransformP transform);

    JsDgnElementP ToDgnElement() { return this; }
};

//=======================================================================================
// This wraps a projection of a C++ interface -- no instance is ever created 
// -- it's always an alias for an instance of a concrete class, such as GeometricElement3d.
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsGeometrySource : JsGeometricElementBase
{
    JsGeometrySource(GeometrySource& gs) : JsGeometricElementBase(*gs.ToElementP()) {}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      04/16
//=======================================================================================
struct JsGeometricElement : JsGeometricElementBase // *** NB: DO NOT USE MULTIPLE INHERITANCE ON A PROJECTED CLASS!!! ***
    {
    JsGeometricElement(GeometricElement& gs) : JsGeometricElementBase(gs) {}
    bool IsValid() const { return m_el.IsValid(); }
    
    // Internal cast helper methods. *** WIP_GeometricElement - remove this when we get a _ToGeometricElement method on DgnElement
    static GeometricElement* ToGeometricElement(DgnElementR el) { return dynamic_cast<GeometricElement*>(&el); }
    GeometricElement* GetGeometricElement() const { return m_el.IsValid() ? ToGeometricElement(*m_el) : nullptr; }
    };

//=======================================================================================
// This wraps a projection of a C++ interface -- no instance is ever created 
// -- it's always an alias for an instance of a concrete class, such as GeometricElement3d
// @bsiclass                                                    Sam.Wilson      04/16
//=======================================================================================
struct JsGeometrySource3d : JsGeometrySource
{
    JsGeometrySource3d(GeometrySource3d& gs) : JsGeometrySource(gs) {}
    JsGeometrySource2dP ToGeometrySource2d() {return nullptr;}
    JsGeometrySource3dP ToGeometrySource3d() {return this;}
};

//=======================================================================================
// This wraps a projection of a C++ interface -- no instance is ever created 
// -- it's always an alias for an instance of a concrete class, such as GeometricElement2d
// @bsiclass                                                    Sam.Wilson      04/16
//=======================================================================================
struct JsGeometrySource2d : JsGeometrySource
    {
    JsGeometrySource2d(GeometrySource2d& gs) : JsGeometrySource(gs) {}
    JsGeometrySource2dP ToGeometrySource2d() { return this; }
    JsGeometrySource3dP ToGeometrySource3d() { return nullptr; }
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      04/16
//=======================================================================================
struct JsGeometricElement3d : JsGeometricElement // *** NB: DO NOT USE MULTIPLE INHERITANCE ON A PROJECTED CLASS!!! ***
    {
    JsGeometricElement3d(GeometricElement3d& gs) : JsGeometricElement(gs) {}

    static JsGeometricElement3d* CreateGeometricElement3d(JsDgnModelP model, JsDgnObjectIdP catid, Utf8StringCR elementClassName);

    // Internal cast helper methods. *** WIP_GeometricElement - remove this when we get a _ToGeometricElement method on DgnElement
    static GeometricElement3d* ToGeometricElement3d(DgnElementR el) { return dynamic_cast<GeometricElement3d*>(&el); } // *** WIP_GeometricElement3d - remove this when we get a _ToGeometricElement3d method on DgnElement
    GeometricElement3d* GetGeometricElement3d() const { return m_el.IsValid() ? ToGeometricElement3d(*m_el) : nullptr; }
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsHitDetail : RefCountedBaseWithCreate
{
    HitDetail m_detail;

    JsHitDetail(HitDetailCR d) : m_detail(d) {}

    JsDPoint3dP GetHitPoint() const { return new JsDPoint3d(m_detail.GetHitPoint()); }
    JsDPoint3dP GetTestPoint() const { return new JsDPoint3d(m_detail.GetTestPoint()); }
    GeometricElement3d* ToGeometricElement3d(DgnElementR el) const {return dynamic_cast<GeometricElement3d*>(&el);}
    JsGeometrySource3dP GetElement() const 
        { 
        DgnElementCPtr el = m_detail.GetElement();
        DGNJSAPI_VALIDATE_ARGS_NULL(el.IsValid());
        auto gel = JsGeometricElement3d::ToGeometricElement3d(const_cast<DgnElementR>(*el));
        DGNJSAPI_VALIDATE_ARGS_NULL(nullptr != gel);
        return (JsGeometrySource3dP)(new JsGeometricElement3d(*gel));
        }
    Utf8String GetHitType() const { return (HitDetailType::Hit == m_detail.GetHitType()) ? "hit" : (HitDetailType::Snap == m_detail.GetHitType()) ? "snap" : "intersection"; }

    STUB_OUT_SET_METHOD(HitPoint, JsDPoint3dP)
    STUB_OUT_SET_METHOD(TestPoint, JsDPoint3dP)
    STUB_OUT_SET_METHOD(HitType, Utf8String)
    STUB_OUT_SET_METHOD(Element, JsGeometrySource3dP)
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnModel : RefCountedBaseWithCreate
{
    DgnModelPtr m_model;

    JsDgnModel(DgnModelR m) : m_model(&m) {;}
    bool IsValid() const {return m_model.IsValid();}

    JsDgnObjectIdP GetModelId() 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsDgnObjectId(m_model->GetModelId().GetValueUnchecked());
        }
    JsAuthorityIssuedCodeP GetCode() const 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsAuthorityIssuedCode(m_model->GetCode());
        }
    JsDgnDbP GetDgnDb() 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsDgnDb(m_model->GetDgnDb());
        }
    static JsAuthorityIssuedCodeP CreateModelCode(Utf8StringCR name) 
        {
        return new JsAuthorityIssuedCode(DgnModel::CreateModelCode(name));
        }

    ComponentModel* ToDgnComponentModel() 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return dynamic_cast<ComponentModel*>(m_model.get());
        }

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

    JsDRange3dP GetElementBox() const { return new JsDRange3d(m_placement.GetElementBox()); }
    JsDRange3dP CalculateRange() const { return new JsDRange3d(m_placement.CalculateRange()); }
    JsTransformP GetTransform() const {return new JsTransform(m_placement.GetTransform()); }

    STUB_OUT_SET_METHOD(ElementBox, JsDRange3dP);
    STUB_OUT_SET_METHOD(Transform, JsTransformP);
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsComponentModel : JsDgnModel
{
    JsComponentModel(ComponentModel& m) : JsDgnModel(m) {;}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsComponentDef : RefCountedBaseWithCreate
{
    ComponentDefPtr m_cdef;

    JsComponentDef(ComponentDef& cd) : m_cdef(&cd) {}
    bool IsValid() const {return m_cdef.IsValid();}

    Utf8String GetName() const {return m_cdef->GetName();}
    JsDgnCategoryP GetCategory() const;
    Utf8String GetCodeAuthority() const {return m_cdef->GetCodeAuthorityName();}
    JsECClassP GetComponentECClass() const;

    //static               FindByName(db: DgnDbP, name: Bentley_Utf8String): ComponentDefP;
    static JsComponentDefP FindByName(JsDgnDbP db, Utf8StringCR name);

    // QueryVariationByName(variationName: Bentley_Utf8String): DgnElementP;
    JsDgnElementP QueryVariationByName(Utf8StringCR variationName);

    // static GetParameters(instance: DgnElementP): ECInstanceP;
    static JsECInstanceP GetParameters(JsDgnElementP instance);

    //! Make an ECInstance whose properties are the input parameters to the ComponentDef's script or solver. 
    //! The caller should then assign values to the properties of the instance.
    //! The caller may then pass the parameters instance to a function such as MakeInstanceOfVariation or MakeUniqueInstance.
    JsECInstanceP MakeParameters();

    //            MakeInstanceOfVariation(targetModel: DgnModelP, variation: DgnElementP, instanceParameters: ECInstanceP, code: DgnCode): DgnElementP;
    JsDgnElementP MakeInstanceOfVariation(JsDgnModelP targetModel, JsDgnElementP variation, JsECInstanceP instanceParameters, JsAuthorityIssuedCodeP code);


    //            MakeUniqueInstance(targetModel: DgnElementP, instanceParameters: ECInstanceP, code: DgnCode): DgnElementP;
    JsDgnElementP MakeUniqueInstance(JsDgnModelP targetModel, JsECInstanceP instanceParameters, JsAuthorityIssuedCodeP code);

    STUB_OUT_SET_METHOD(Name,Utf8String)
    STUB_OUT_SET_METHOD(Category,JsDgnCategoryP)
    STUB_OUT_SET_METHOD(CodeAuthority,Utf8StringCR)
    STUB_OUT_SET_METHOD(ComponentECClass,JsECClassP)
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
struct JsColorDef : RefCountedBaseWithCreate
{
    ColorDef m_value;

    JsColorDef(ColorDef const& v) : m_value(v) {}
    JsColorDef(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) : m_value(red, green, blue, alpha) {}

    VALUE_TYPE_GET_SET(uint8_t, Red)
    VALUE_TYPE_GET_SET(uint8_t, Green)
    VALUE_TYPE_GET_SET(uint8_t, Blue)
    VALUE_TYPE_GET_SET(uint8_t, Alpha)
};

typedef JsColorDef* JsColorDefP;

#ifdef WIP_DGNJSAPI // *** -- need a way to specify both the TS namespace and the C++ namespace
#endif
#ifdef WIP_DGNJSAPI // *** Keep this consistent with Render.h
#endif
// Define a PLACEHOLDER enum in this namespace that just represents the real enum in another C++ namespace. 
BEJAVASCRIPT_EXPORT_CLASS (Bentley.Dgn)
enum class RenderFillDisplay : uint32_t 
{
    Never    = 0, //!< don't fill, even if fill attribute is on for the viewport
    ByView   = 1, //!< fill if the fill attribute is on for the viewport
    Always   = 2, //!< always fill, even if the fill attribute is off for the viewport
    Blanking = 3, //!< always fill, fill will always be behind subsequent geometry
};

// Define a PLACEHOLDER enum in this namespace that just represents the real enum in another C++ namespace. 
BEJAVASCRIPT_EXPORT_CLASS (Bentley.Dgn)
enum class RenderDgnGeometryClass : uint32_t 
{
    Primary      = 0,
    Construction = 1,
    Dimension    = 2,
    Pattern      = 3,
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsRenderGeometryParams : RefCountedBaseWithCreate
{
    Render::GeometryParams m_value;

    JsRenderGeometryParams() {;}
    JsRenderGeometryParams(Render::GeometryParams const& v) : m_value(v) {;}

    JsDgnObjectIdP GetCategoryId() {if (!m_value.GetCategoryId().IsValid()) return nullptr; return new JsDgnObjectId(m_value.GetCategoryId().GetValue());}
    STUB_OUT_SET_METHOD(CategoryId,JsDgnObjectIdP)
    JsDgnObjectIdP GetSubCategoryId() {if (!m_value.GetSubCategoryId().IsValid()) return nullptr; return new JsDgnObjectId(m_value.GetSubCategoryId().GetValue());}
    void SetSubCategoryId(JsDgnObjectIdP v) {m_value.SetSubCategoryId(DgnSubCategoryId(v->m_id));}
    JsColorDefP GetLineColor() {return new JsColorDef(m_value.GetLineColor());}
    void SetLineColor(JsColorDefP v) {m_value.SetLineColor(v->m_value);}
    JsColorDefP GetFillColor() {return new JsColorDef(m_value.GetFillColor());}
    void SetFillColor(JsColorDefP v) {m_value.SetFillColor(v->m_value);}
    void SetFillColorToViewBackground() {m_value.SetFillColorToViewBackground();}
    VALUE_TYPE_GET_SET_CAST(RenderFillDisplay, Render::FillDisplay, FillDisplay)
    VALUE_TYPE_GET_SET_CAST(RenderDgnGeometryClass, Render::DgnGeometryClass, GeometryClass)
    VALUE_TYPE_GET_SET(uint32_t, Weight)
    VALUE_TYPE_GET_SET(double, Transparency)
    VALUE_TYPE_GET_SET(double, FillTransparency)
    VALUE_TYPE_GET_SET(int32_t, DisplayPriority)
    JsDgnObjectIdP GetMaterialId() {if (!m_value.GetMaterialId().IsValid()) return nullptr; return new JsDgnObjectId(m_value.GetMaterialId().GetValue());}
    void SetMaterialId(JsDgnObjectIdP v) {m_value.SetMaterialId(DgnMaterialId(v->m_id));}
};

typedef JsRenderGeometryParams* JsRenderGeometryParamsP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      02/16
//=======================================================================================
struct JsTextString : RefCountedBaseWithCreate
{
    TextStringPtr m_value;

    // *** TBD
};

typedef JsTextString* JsTextStringP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      02/16
//=======================================================================================
struct JsGeometricPrimitive : RefCountedBaseWithCreate
{
    GeometricPrimitivePtr m_value;
    JsGeometricPrimitive(GeometricPrimitive& v) : m_value(&v) {;}
    bool IsValid() const {return m_value.IsValid();}

    JsGeometryP GetGeometry() const;
    JsTextStringP GetTextString() const;

    STUB_OUT_SET_METHOD(TextString, JsTextStringP)
    STUB_OUT_SET_METHOD(Geometry, JsGeometryP)
};

typedef JsGeometricPrimitive* JsGeometricPrimitiveP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      02/16
//=======================================================================================
struct JsDgnGeometryPart : RefCountedBaseWithCreate
{
    DgnGeometryPartPtr m_value;
    JsDgnGeometryPart(DgnGeometryPart& v) : m_value(&v) {;}
    bool IsValid() const {return m_value.IsValid();}

    static JsDgnGeometryPart* Create(JsDgnDbP db) 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(db));
        return new JsDgnGeometryPart(*DgnGeometryPart::Create(*db->m_db));
        }
    BentleyStatus Insert() {return m_value->GetDgnDb().GeometryParts().InsertGeometryPart(*m_value);}
};

typedef JsDgnGeometryPart* JsDgnGeometryPartP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      02/16
//=======================================================================================
struct JsGeometryCollectionIterator : RefCountedBaseWithCreate
    {
    typedef GeometryCollection T_CppColl;
    T_CppColl::Iterator m_iter;

    JsGeometryCollectionIterator(T_CppColl::Iterator it) : m_iter(it) {}
    };

typedef JsGeometryCollectionIterator* JsGeometryCollectionIteratorP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      02/16
//=======================================================================================
struct JsGeometryCollection : RefCountedBaseWithCreate
{
    JS_COLLECTION_IMPL(JsGeometryCollection,JsGeometryCollectionIterator)

    JsGeometryCollection(GeometrySourceCR el) : m_collection(el) {}

    JsGeometricPrimitiveP GetGeometry(JsGeometryCollectionIteratorP iter)
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid(iter));
        auto g = iter->m_iter.GetGeometryPtr();
        return g.IsValid()? new JsGeometricPrimitive(*g): nullptr;
        }
    JsDgnGeometryPartP GetGeometryPart(JsGeometryCollectionIteratorP iter)
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid(iter));
        auto g = iter->m_iter.GetGeometryPartPtr();
        return g.IsValid()? new JsDgnGeometryPart(*g): nullptr;
        }
    JsTransformP GetGeometryToWorld(JsGeometryCollectionIteratorP iter)
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid(iter));
        return new JsTransform(iter->m_iter.GetGeometryToWorld());
        }
    JsRenderGeometryParamsP GetGeometryParams(JsGeometryCollectionIteratorP iter)
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid(iter));
        return new JsRenderGeometryParams(iter->m_iter.GetGeometryParams());
        }

};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsGeometryBuilder : RefCountedBaseWithCreate
{
    GeometryBuilderPtr m_builder;

    JsGeometryBuilder(GeometryBuilderR gb) : m_builder(&gb) {}
    JsGeometryBuilder(JsGeometrySourceP el, JsDPoint3dP o, JsYawPitchRollAnglesP angles);
    JsGeometryBuilder(JsGeometrySourceP el, DPoint3dCR o, YawPitchRollAnglesCR angles);
    ~JsGeometryBuilder() {}
    bool IsValid() const {return m_builder.IsValid();}

    static JsGeometryBuilderP CreateForElement(JsGeometrySourceP el, JsDPoint3dP o, JsYawPitchRollAnglesP angles)
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSELEMENT_PLACEHOLDER(el) && o && angles);
        return new JsGeometryBuilder(el, o, angles);
        }

    static JsGeometryBuilderP CreateForModel(JsDgnModelP model, JsDgnObjectIdP catid, JsDPoint3dP o, JsYawPitchRollAnglesP angles)
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(model) && o && angles);
        return new JsGeometryBuilder(*GeometryBuilder::Create(*model->m_model, DgnCategoryId(catid->m_id), o->Get(), angles->GetYawPitchRollAngles()));
        }


    static JsGeometryBuilderP CreateForElementWithTransform(JsGeometrySourceP el, JsTransformP transform)
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSELEMENT_PLACEHOLDER(el) && transform);
        DPoint3d origin;
        YawPitchRollAngles angles;
        if (!YawPitchRollAngles::TryFromTransform (origin, angles, transform->Get ()))
            return nullptr;
        return new JsGeometryBuilder(el, origin, angles);
        }

    static JsGeometryBuilderP CreateForModelWithTransform(JsDgnModelP model, JsDgnObjectIdP catid, JsTransformP transform)
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(model) && transform);
        DPoint3d origin;
        YawPitchRollAngles angles;
        if (!YawPitchRollAngles::TryFromTransform (origin, angles, transform->Get ()))
            return nullptr;
        return new JsGeometryBuilder(*GeometryBuilder::Create(*model->m_model, DgnCategoryId(catid->m_id), origin, angles));
        }



    static JsGeometryBuilderP CreateGeometryPart(JsDgnDbP db, bool is3d)
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(db));
        return new JsGeometryBuilder(*GeometryBuilder::CreateGeometryPart(*db->m_db, is3d));
        }

    JsRenderGeometryParamsP GetGeometryParams() const 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsRenderGeometryParams(m_builder->GetGeometryParams());
        }

    void AppendRenderGeometryParams(JsRenderGeometryParamsP params)
        {
        DGNJSAPI_VALIDATE_ARGS_VOID(IsValid() && params);
        m_builder->Append(params->m_value);
        }


    void AppendSubCategoryId(JsDgnObjectIdP subcategoryId) 
        {
        DGNJSAPI_VALIDATE_ARGS_VOID(IsValid() && DGNJSAPI_IS_VALID_JSOBJ(subcategoryId));
        m_builder->Append(DgnSubCategoryId(subcategoryId->m_id));
        }

    /*
    void AppendSolidPrimitive(JsSolidPrimitiveP solid) 
        {
        if (solid && solid->GetISolidPrimitivePtr().IsValid())
            m_builder->Append(*solid->GetISolidPrimitivePtr());
        }
    */
    
    void AppendGeometry(JsGeometryP geometry)
        {
        DGNJSAPI_VALIDATE_ARGS_VOID(IsValid() && geometry);
        if (geometry->GetCurveVectorPtr().IsValid())
            m_builder->Append(*geometry->GetCurveVectorPtr());
        else if (geometry->GetICurvePrimitivePtr().IsValid())
            m_builder->Append(*geometry->GetICurvePrimitivePtr());
        else if (geometry->GetISolidPrimitivePtr().IsValid())
            m_builder->Append(*geometry->GetISolidPrimitivePtr());
        }
    void AppendGeometryNode (JsGeometryNodeP node)
        {
        DGNJSAPI_VALIDATE_ARGS_VOID(IsValid() && node);
        auto nativeNode = node->GetGeometryNodePtr ();
        if (nativeNode.IsValid ())
            {
            auto flatNode = node->Flatten ();
            auto nativeFlatNode = flatNode->GetGeometryNodePtr ();
            for (auto &geometry : nativeFlatNode->Geometry ())
                {
                m_builder->Append (*geometry);
                }
            }
        }

    BentleyStatus SetGeometryStreamAndPlacement (JsGeometrySourceP jgs)
        {
        DGNJSAPI_VALIDATE_ARGS_ERROR(IsValid() && DGNJSAPI_IS_VALID_JSELEMENT_PLACEHOLDER(jgs))
        return m_builder->SetGeometryStreamAndPlacement(*jgs->m_el->ToGeometrySourceP());
        }

    BentleyStatus SetGeometryStream (JsDgnGeometryPartP part) {return m_builder->SetGeometryStream(*part->m_value);}

    void AppendCopyOfGeometry(JsGeometryBuilderP builder, JsPlacement3dP relativePlacement);
    void AppendGeometryPart(JsDgnGeometryPartP part, JsPlacement3dP relativePlacement);

    STUB_OUT_SET_METHOD(GeometryParams, JsRenderGeometryParamsP)
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnElements : RefCountedBaseWithCreate
{
    DgnElements& m_elements;

    JsDgnElements(DgnElements& els) : m_elements(els) {}

    JsDgnDbP GetDgnDb() const {return new JsDgnDb(m_elements.GetDgnDb()); }
    JsDgnElementP FindElement(JsDgnObjectIdP id) const;
    JsDgnElementP GetElement(JsDgnObjectIdP id) const;
    JsDgnObjectIdP QueryElementIdByCode(Utf8StringCR codeAuthorityName, Utf8StringCR codeValue, Utf8StringCR nameSpace) const;

    STUB_OUT_SET_METHOD(DgnDb, JsDgnDbP)
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnCategory : RefCountedBaseWithCreate
{
    DgnCategoryCPtr m_category;

    JsDgnCategory(DgnCategoryCR cat) : m_category(&cat) {;}
    bool IsValid() const {return m_category.IsValid();}

    JsDgnDbP GetDgnDb() 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsDgnDb(m_category->GetDgnDb());
        }
    JsDgnObjectIdP GetCategoryId() 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsDgnObjectId(m_category->GetCategoryId().GetValueUnchecked());
        }
    JsDgnObjectIdP GetDefaultSubCategoryId() 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsDgnObjectId(m_category->GetDefaultSubCategoryId().GetValueUnchecked());
        }
    Utf8String GetCategoryName() 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return m_category->GetCategoryName();
        }
    static JsDgnObjectIdP QueryCategoryId(Utf8StringCR name, JsDgnDbP db) 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(db));
        return new JsDgnObjectId(DgnCategory::QueryCategoryId(name, *db->m_db).GetValueUnchecked());
        }
    static JsDgnCategory* QueryCategory(JsDgnObjectIdP id, JsDgnDbP db) 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(db));
        auto cat = DgnCategory::QueryCategory(DgnCategoryId(id->m_id), *db->m_db);
        return cat.IsValid()? new JsDgnCategory(*cat): nullptr;
        }
    static JsDgnObjectIdSetP QueryCategories(JsDgnDbP db)
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(db));
        return new JsDgnObjectIdSet(DgnCategory::QueryCategories(*db->m_db));
        }

    STUB_OUT_SET_METHOD(DgnDb,JsDgnDbP)
    STUB_OUT_SET_METHOD(CategoryId,JsDgnObjectIdP)
    STUB_OUT_SET_METHOD(DefaultSubCategoryId,JsDgnObjectIdP)
    STUB_OUT_SET_METHOD(CategoryName,Utf8String)

};

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
    bool IsValid() const {return nullptr != m_ecClass;}

    JsECClass(ECN::ECClassCR c) : m_ecClass(&c) {;}

    JsDgnObjectIdP GetECClassId() const
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsDgnObjectId(m_ecClass->GetId().GetValue());
        }

    Utf8String GetName() 
        {
        DGNJSAPI_VALIDATE_ARGS(IsValid(), "");
        return m_ecClass->GetName();
        }
    Utf8String GetECSqlName() const
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return m_ecClass->GetECSqlName();
        }
    JsECClassCollectionP GetBaseClasses() const
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsECClassCollection(m_ecClass->GetBaseClasses());
        }
    JsECClassCollectionP GetDerivedClasses() const 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsECClassCollection(m_ecClass->GetDerivedClasses());
        }
    JsECPropertyCollectionP GetProperties() const 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsECPropertyCollection(m_ecClass->GetProperties());
        }

    JsECPropertyP GetProperty(Utf8StringCR name) const;

    JsECInstanceP GetCustomAttribute(Utf8StringCR className) const;

    JsECInstanceP MakeInstance();

    JsECSchemaP GetSchema() const;

    STUB_OUT_SET_METHOD(ECClassId, JsDgnObjectIdP)
    STUB_OUT_SET_METHOD(Name,Utf8String)
    STUB_OUT_SET_METHOD(ECSqlName,Utf8String)
    STUB_OUT_SET_METHOD(Schema,JsECSchemaP)
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
    bool IsValid() const {return nullptr != m_ecSchema;}

    Utf8String GetName() 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return m_ecSchema->GetName();
        }

    JsECClassP GetECClass(Utf8StringCR cls) 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
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

    JsECClassP GetECClassById(JsDgnObjectIdP clsid)
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(DGNJSAPI_IS_VALID_JSOBJ(clsid));
        auto eccls = m_schemas.GetECClass(ECN::ECClassId(clsid->m_id));
        return eccls ? new JsECClass(*eccls) : nullptr;
        }
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECProperty : RefCountedBaseWithCreate
{
    ECN::ECPropertyCP m_property;
    bool IsValid() const {return nullptr != m_property;}

    JsECProperty(ECN::ECPropertyCR prop) : m_property(&prop) {;}

    Utf8String GetName() 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return m_property->GetName();
        }

    JsPrimitiveECPropertyP GetAsPrimitiveProperty() const;
    JsECInstanceP GetCustomAttribute(Utf8StringCR className) const;
    
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

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECValue : RefCountedBaseWithCreate
{
    ECN::ECValue m_value;

    bool IsValid() const {return !m_value.IsNull();}

    static JsECValue* FromDouble(double v) {return new JsECValue(ECN::ECValue(v));}
    static JsECValue* FromInteger(int32_t v) {return new JsECValue(ECN::ECValue(v));}
    static JsECValue* FromString(Utf8StringCR s) {return new JsECValue(ECN::ECValue(s.c_str()));}
    static JsECValue* FromDateTime(Utf8StringCR s) 
        {
        DateTime dt;
        if (BSISUCCESS != DateTime::FromString(dt, s.c_str()))
            return nullptr;
        return new JsECValue(ECN::ECValue(dt));
        }
    static JsECValue* FromPoint3d(JsDPoint3dP s) {DGNJSAPI_VALIDATE_ARGS_NULL(nullptr != s); return new JsECValue(ECN::ECValue(s->GetCR()));}

    JsECValue(ECN::ECValueCR v) : m_value(v) {;}
    bool GetIsPrimitive() const {return m_value.IsPrimitive();}
    bool GetIsNull() const {return m_value.IsNull();}
    ECPropertyPrimitiveType GetPrimitiveType() const {return (ECPropertyPrimitiveType)m_value.GetPrimitiveType();}
    Utf8String GetString() const {return m_value.IsNull()? "": m_value.ToString();}
    JsDPoint3dP GetPoint3d() const {return m_value.IsNull()? nullptr: new JsDPoint3d(m_value.GetPoint3D());}
    Utf8String GetDateTime() const 
        {
        if (m_value.IsNull())
            return "";
        auto dt = m_value.GetDateTime();
        return dt.ToUtf8String();
        }
    int32_t GetInteger() const {return m_value.IsNull()? 0: m_value.GetInteger();}
    double GetDouble() const {return m_value.IsNull()? 0.0: m_value.GetDouble();}

    STUB_OUT_SET_METHOD(PrimitiveType,ECPropertyPrimitiveType)
    STUB_OUT_SET_METHOD(IsPrimitive,bool)
    STUB_OUT_SET_METHOD(IsNull,bool)
    STUB_OUT_SET_METHOD(String,Utf8StringCR)
    STUB_OUT_SET_METHOD(Integer,int32_t)
    STUB_OUT_SET_METHOD(Double,double)
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsAdHocJsonPropertyValue : RefCountedBaseWithCreate
{
    RefCountedPtr<JsDgnElement> m_jsel;   // keep the native and JS element objects alive while I hold a reference to the native element's data
    ECN::AdHocJsonPropertyValue m_prop;

    JsAdHocJsonPropertyValue(JsDgnElement* jsel, ECN::AdHocJsonPropertyValueCR p) : m_jsel(jsel), m_prop(p) {;}

    Utf8String GetUnits() const {Utf8String u; return (ECN::AdHocJsonPropertyValue::GetStatus::Found == m_prop.GetUnits(u))? u: "";}
    void SetUnits(Utf8StringCR units) {m_prop.SetUnits(units.c_str());}

    Utf8String GetExtendedType() const {Utf8String u; return (ECN::AdHocJsonPropertyValue::GetStatus::Found == m_prop.GetExtendedType(u))? u: "";}
    void SetExtendedType(Utf8StringCR ExtendedType) {m_prop.SetExtendedType(ExtendedType.c_str());}

    Utf8String GetCategory() const {Utf8String u; return (ECN::AdHocJsonPropertyValue::GetStatus::Found == m_prop.GetCategory(u))? u: "";}
    void SetCategory(Utf8StringCR Category) {m_prop.SetCategory(Category.c_str());}

    bool GetHidden() const {bool u; return (ECN::AdHocJsonPropertyValue::GetStatus::Found == m_prop.GetHidden(u))? u: false;}
    void SetHidden(bool v) {m_prop.SetHidden(v);}

    bool GetReadOnly() const {bool u; return (ECN::AdHocJsonPropertyValue::GetStatus::Found == m_prop.GetReadOnly(u))? u: false;}
    void SetReadOnly(bool v) {m_prop.SetReadOnly(v);}

    int32_t GetPriority() const {int32_t u; return (ECN::AdHocJsonPropertyValue::GetStatus::Found == m_prop.GetPriority(u))? u: false;}
    void SetPriority(int32_t v) {m_prop.SetPriority(v);}

    JsECValueP GetValueEC() const {ECN::ECValue v = m_prop.GetValueEC(); return v.IsNull()? nullptr: new JsECValue(v);}
    int32_t SetValueEC(JsECValueP value) {DGNJSAPI_VALIDATE_ARGS(DGNJSAPI_IS_VALID_JSOBJ(value), -1); return (int32_t)m_prop.SetValueEC(value->m_value);}
    
    ECPropertyPrimitiveType GetType() const {ECN::PrimitiveType t; return (ECN::AdHocJsonPropertyValue::GetStatus::Found == m_prop.GetType(t))? (ECPropertyPrimitiveType)t: ECPropertyPrimitiveType::Unknown;}
    STUB_OUT_SET_METHOD(Type,ECPropertyPrimitiveType)

};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsECInstance : RefCountedBaseWithCreate
{
    ECN::IECInstancePtr m_instance;

    JsECInstance(ECN::IECInstanceR i) : m_instance(&i) {;}
    bool IsValid() const {return m_instance.IsValid();}

    JsECClassP GetClass() 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        return new JsECClass(m_instance->GetClass());
        } 
    JsECValueP GetValue(Utf8StringCR propertyName) 
        {
        DGNJSAPI_VALIDATE_ARGS_NULL(IsValid());
        ECN::ECValue v;
        if (ECN::ECObjectsStatus::Success != m_instance->GetValueOrAdhoc(v, propertyName.c_str()))
            return nullptr;
        return new JsECValue(v);
        }

    void SetValue(Utf8StringCR propertyName, JsECValueP value) 
        {
        DGNJSAPI_VALIDATE_ARGS_VOID(IsValid() && value);
        m_instance->SetValueOrAdhoc(propertyName.c_str(), value->m_value);
        }

    STUB_OUT_SET_METHOD(Class,JsECClassP)
};

typedef JsECInstance* JsECInstanceP;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsAdhocPropertyQuery : RefCountedBaseWithCreate
    {
    JsECInstanceP m_host;
    ECN::AdhocPropertyQuery m_query;
    
    JsAdhocPropertyQuery(JsECInstanceP host, Utf8StringCR containerAccessString) : m_host(host), m_query(*host->m_instance, containerAccessString.empty()? "Parameters": containerAccessString.c_str()) {;}

    JsECInstanceP      GetHost() const { return m_host; }

    uint32_t           GetPropertyIndex (Utf8StringCR accessString) const {uint32_t i; return m_query.GetPropertyIndex(i,accessString.c_str())? i: UINT32_MAX;}
    uint32_t           GetCount() const {return m_query.GetCount();}

    Utf8String GetName (uint32_t index) const {Utf8String v; m_query.GetName(v, index); return v;}
    Utf8String GetDisplayLabel (uint32_t index) const {Utf8String v; m_query.GetDisplayLabel(v, index); return v;}
    JsECValueP GetValue (uint32_t index) const {ECN::ECValue v; m_query.GetValue(v, index); return new JsECValue(v);}
    ECPropertyPrimitiveType GetPrimitiveType (uint32_t index) const {ECN::PrimitiveType v = (ECN::PrimitiveType)0; m_query.GetPrimitiveType(v, index); return (ECPropertyPrimitiveType)v;}
    Utf8String GetUnitName (uint32_t index) const {Utf8String v; m_query.GetUnitName(v, index); return v;}
    bool IsReadOnly (uint32_t index) const {bool v = false; m_query.IsReadOnly(v, index); return v;}
    bool IsHidden (uint32_t index) const {bool v = false; m_query.IsHidden(v, index); return v;}

    STUB_OUT_SET_METHOD(Host,JsECInstanceP)
    STUB_OUT_SET_METHOD(Count,uint32_t)
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsFile : RefCountedBaseWithCreate
    {
    FILE* m_fp;

    JsFile(FILE* fp) : m_fp(fp) {;}
    ~JsFile() {if (m_fp) fclose(m_fp);}

    bool IsValid() const {return nullptr != m_fp;}

    static JsFile* Fopen(Utf8StringCR name, Utf8StringCR mode);
    void Close() {if (m_fp) fclose(m_fp); m_fp = nullptr;}
    bool Feof();
    Utf8String ReadLine();
    int32_t WriteLine(Utf8StringCR line);
};

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

END_BENTLEY_DGN_NAMESPACE

#endif//ndef _DGN_JS_API_H_

