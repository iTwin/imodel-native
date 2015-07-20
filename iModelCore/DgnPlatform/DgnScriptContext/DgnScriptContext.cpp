/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScriptContext/DgnScriptContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeJavaScript/BeJavaScript.h>
#include <DgnPlatform/DgnCore/DgnScriptContext.h>
#include <Bentley/BeFileListIterator.h>
#include <ECObjects/ECObjectsAPI.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>

#define DEBUG_JS_GC

extern Utf8CP dgnScriptContext_GetBootstrappingSource();

BEJAVASCRIPT_EMIT_CUSTOM_TYPESCRIPT_DECLARATION ("export declare class BeJsNativePointer {}")

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
static void deleteThis(void* b) {delete (BeJsNativePointer*)b;}

//---------------------------------------------------------------------------------------
// Type conversion helpers
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
static Utf8CP getUtf8CP(Utf8StringCR s) {return s.c_str();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static RefCountedCPtr<PhysicalElement> createPhysicalElement(DgnModelR model, Utf8CP ecSqlClassName, DgnCategoryId catid, Utf8CP code)//, RefCountedPtr<T> geom)
    {
    Utf8CP dot = strchr(ecSqlClassName, '.');
    if (nullptr == dot)
        return nullptr;
    Utf8String ecschema(ecSqlClassName, dot);
    Utf8String ecclass(dot+1);
    DgnDbR db = model.GetDgnDb();
    DgnClassId pclassId = DgnClassId(db.Schemas().GetECClassId(ecschema.c_str(), ecclass.c_str()));
    PhysicalElementPtr el = PhysicalElement::Create(PhysicalElement::CreateParams(db, model.GetModelId(), pclassId, catid));

    if (nullptr != code)
        el->SetCode(code);

//    DPoint3d start = DPoint3d::FromZero();
//    YawPitchRollAngles angles;

//    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*el, start, angles);

//    builder->Append(*geom);

//    builder->SetGeomStreamAndPlacement(*el);
    return el;
    }

//=======================================================================================
// The first argument to every prototype's Create methods be DgnScriptContextImpl::GetCallContext()
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct DgnJsCallContext
    {
    bvector<BeJsNativePointer*> m_localsToBeDeleted;
  
    void Enter() {BeAssert(m_localsToBeDeleted.empty());}

    void Leave()
        {
        for (BeJsNativePointer* ptr : m_localsToBeDeleted)
            {
            void *nativePtr = ptr->GetValue();
            if (nullptr != nativePtr)
                {
                ptr->SetValue(nullptr);
                deleteThis(nativePtr);
                }
            }
        m_localsToBeDeleted.clear();
        }

    template<typename T>
    T* ScheduleDelete(T* p) {m_localsToBeDeleted.push_back(p); return p;}
    };

//=======================================================================================
// A wrapper class wraps a JsObject around a native value.
// Every wrapper class is-a BeJsNativePointer, and it holds the corresponding native value as its data.
// The lifetime of a wrapper class instance is controlled by the lifetime of the JS object.
// The JS object invokes the supplied BeJsNativePointer::DisposeCallback on the wrapper object when
// the JS object is garbage-collected. Therefore, all wrapper instances must be created using new and must
// not be deleted from the native side. Wrapper instances must not be allocated on the stack!
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS struct JsDgnElement : BeJsNativePointer
{
    // Every wrapper class will hold the native value
    DgnElementPtr m_el;

    BeJsObjectR GetJsObject() {return *this;}

    //  Interface methods - These are specific to each kind of native type

    //! Get the element's DgnElementId
    Utf8String GetElementId();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDgnElement, GetElementId)

    //  The constructor for every wrapper class will be the same
    JsDgnElement(DgnElementR el, BeJsContext& jsctx, BeJsObject& jsprototype) : BeJsNativePointer(jsctx, this, deleteThis, &jsprototype), m_el(&el) {;}
    };

typedef JsDgnElement* JsDgnElementP;

// Implement the instance functions and the corresponding callback marshalling functions
Utf8String JsDgnElement::GetElementId() {return Utf8PrintfString("%lld", (long long)m_el->GetElementId().GetValueUnchecked());}
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsDgnElement, GetElementId, String, getUtf8CP)

//  Every wrapper class will have a Prototype class associated with it.
//  The prototype is paired with a JS object that has properties that are the get/set and other instance functions.
// The first argument to every prototype's Create methods be DgnScriptContextImpl::GetCallContext()
struct JsDgnElementPrototype
{
    BeJsContext& m_ctx;
    BeJsContext& GetJsContext() {return m_ctx;}

    BEJAVASCRIPT_DECLARE_JS_CALLBACKS_HOLDER(JsDgnElement)  // this just declares a member variable of type JsObject
    BeJsObject& GetJsObject() {return BEJAVASCRIPT_JS_CALLBACKS_HOLDER(JsDgnElement);}

    JsDgnElementPrototype(BeJsContext& ctx) : m_ctx(ctx),
        BEJAVASCRIPT_INSTANTIATE_JS_CALLBACKS_HOLDER(JsDgnElement)  // this constructs the JsObject
        {
        //  Set the function properties of the JS prototype object, one for each callback method
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_FUNCTION_CALLBACK(JsDgnElement, GetElementId);

        //  Finish the initialization of the JS prototype object.
        BEJAVASCRIPT_INITIALIZE_CLASS("BentleyApi.Dgn", JsDgnElement)
        }

    //  Make an instance
    JsDgnElement* Create(DgnJsCallContext& cc, DgnElementR el) {return cc.ScheduleDelete(new JsDgnElement(el, m_ctx, GetJsObject()));}
};

//=======================================================================================
// A wrapper class wraps a JsObject around a native value.
// Every wrapper class is-a BeJsNativePointer, and it holds the corresponding native value as its data.
// The lifetime of a wrapper class instance is controlled by the lifetime of the JS object.
// The JS object invokes the supplied BeJsNativePointer::DisposeCallback on the wrapper object when
// the JS object is garbage-collected. Therefore, all wrapper instances must be created using new and must
// not be deleted from the native side. Wrapper instances must not be allocated on the stack!
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS struct JsDgnModel : BeJsNativePointer
{
    // Every wrapper class will hold the native value
    DgnModelPtr m_model;

    BeJsObjectR GetJsObject() {return *this;}

    //  Interface methods - These are specific to each kind of native type

    //! Get the element's DgnModelId
    Utf8String GetModelId();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDgnModel, GetModelId)

    JsDgnElement* CreateElement(Utf8StringCR elType, Utf8StringCR categoryName);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDgnModel, CreateElement)

    void InsertElement(JsDgnElementP element);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDgnModel, InsertElement)

    void DeleteAllElements();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDgnModel, DeleteAllElements)

    //  The constructor for every wrapper class will be the same
    JsDgnModel(DgnModelR m, BeJsContext& jsctx, BeJsObject& jsprototype) : BeJsNativePointer(jsctx, this, deleteThis, &jsprototype), m_model(&m) {;}
    };

typedef JsDgnModel* JsDgnModelP;

// Implement the instance functions and the corresponding callback marshalling functions
Utf8String JsDgnModel::GetModelId() {return Utf8PrintfString("%lld", (long long)m_model->GetModelId().GetValueUnchecked());}
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsDgnModel, GetModelId, 
    String, getUtf8CP)

BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_NULLABLEOBJECT_2ARGS(JsDgnModel, CreateElement, 
    JsDgnElementP, 
    Utf8String, String, GetValue, 
    Utf8String, String, GetValue)

void JsDgnModel::InsertElement(JsDgnElementP element)
    {
    if (nullptr != element)
        m_model->GetDgnDb().Elements().Insert(*element->m_el);
    }
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsDgnModel, InsertElement, 
    JsDgnElementP, NativePointer, GetValue)

void JsDgnModel::DeleteAllElements() 
    {
    m_model->FillModel();
    bvector<DgnElementCPtr> elementsInModel;
    for (auto const& emapEntry : *m_model)
        elementsInModel.push_back(emapEntry.second);
    for (auto const& el : elementsInModel)
        el->Delete();
    }
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID(JsDgnModel, DeleteAllElements)

//  Every wrapper class will have a Prototype class associated with it.
//  The prototype is paired with a JS object that has properties that are the get/set and other instance functions.
// The first argument to every prototype's Create methods be DgnScriptContextImpl::GetCallContext()
struct JsDgnModelPrototype
{
    BeJsContext& m_ctx;
    BeJsContext& GetJsContext() {return m_ctx;}

    BEJAVASCRIPT_DECLARE_JS_CALLBACKS_HOLDER(JsDgnModel)  // this just declares a member variable of type JsObject
    BeJsObject& GetJsObject() {return BEJAVASCRIPT_JS_CALLBACKS_HOLDER(JsDgnModel);}

    JsDgnModelPrototype(BeJsContext& ctx) : m_ctx(ctx),
        BEJAVASCRIPT_INSTANTIATE_JS_CALLBACKS_HOLDER(JsDgnModel)  // this constructs the JsObject
        {
        //  Set the function properties of the JS prototype object, one for each callback method
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_FUNCTION_CALLBACK(JsDgnModel, GetModelId);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_FUNCTION_CALLBACK(JsDgnModel, CreateElement);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_FUNCTION_CALLBACK(JsDgnModel, InsertElement);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_FUNCTION_CALLBACK(JsDgnModel, DeleteAllElements);

        //  Finish the initialization of the JS prototype object.
        BEJAVASCRIPT_INITIALIZE_CLASS("BentleyApi.Dgn", JsDgnModel)
        }

    //  Make an instance
    JsDgnModel* Create(DgnJsCallContext& cc, DgnModelR m) {return cc.ScheduleDelete(new JsDgnModel(m, m_ctx, GetJsObject()));}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS struct JsDPoint3d : BeJsNativePointer
{
    DPoint3d m_pt;

    BeJsObjectR GetJsObject() {return *this;}

    //! Get the X coordinate
    double GetX();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, GetX)
    //! Get the Y coordinate
    double GetY();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, GetY)
    //! Get the Z coordinate
    double GetZ();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, GetZ)
    //! Set the X coordinate
    void SetX(double meters);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, SetX)
    //! Set the Y coordinate
    void SetY(double meters);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, SetY)
    //! Set the Z coordinate
    void SetZ(double meters);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, SetZ)

    //! Create a JsDPoint3d object (JavaScript callers use this)
    static JsDPoint3d* Create(double x, double y, double z);
    BEJAVASCRIPT_DECLARE_STATIC_CALLBACK(JsDPoint3d, Create)

    JsDPoint3d(DPoint3dCR pt, BeJsContext& jsctx, BeJsObject& jsprototype) : BeJsNativePointer(jsctx, this, deleteThis, &jsprototype), m_pt(pt) {;}
    JsDPoint3d(double x, double y, double z);
};

typedef JsDPoint3d* JsDPoint3dP;

BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsDPoint3d, GetX, Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsDPoint3d, GetY, Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsDPoint3d, GetZ, Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsDPoint3d, SetX, double, Number, GetValue)
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsDPoint3d, SetY, double, Number, GetValue)
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsDPoint3d, SetZ, double, Number, GetValue)
BEJAVASCRIPT_DEFINE_STATIC_CALLBACK_NULLABLEOBJECT_3ARGS(JsDPoint3d, Create,
    JsDPoint3dP,
    double, Number, GetValue,
    double, Number, GetValue,
    double, Number, GetValue)

double JsDPoint3d::GetX() {return m_pt.x;}
double JsDPoint3d::GetY() {return m_pt.y;}
double JsDPoint3d::GetZ() {return m_pt.z;}
void JsDPoint3d::SetX(double v) {m_pt.x = v;}
void JsDPoint3d::SetY(double v) {m_pt.y = v;}
void JsDPoint3d::SetZ(double v) {m_pt.z = v;}

//  Every wrapper class will have a Prototype class associated with it.
//  The prototype is paired with a JS object that has properties that are the get/set and other instance functions.
// The first argument to every prototype's Create methods be DgnScriptContextImpl::GetCallContext()
struct JsDPoint3dPrototype
    {
    BeJsContext& m_ctx;
    BeJsContext& GetJsContext() {return m_ctx;}

    BEJAVASCRIPT_DECLARE_JS_CALLBACKS_HOLDER(JsDPoint3d)  // this just declares a member variable of type JsObject
    BeJsObject& GetJsObject() {return BEJAVASCRIPT_JS_CALLBACKS_HOLDER(JsDPoint3d);}

    JsDPoint3dPrototype(BeJsContext& ctx) : m_ctx(ctx),
        BEJAVASCRIPT_INSTANTIATE_JS_CALLBACKS_HOLDER(JsDPoint3d)  // this constructs the JsObject
        {
        //  Set the function properties of the JS prototype object, one for each callback method
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsDPoint3d, X);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsDPoint3d, Y);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsDPoint3d, Z);
        BEJAVASCRIPT_INSTANTIATE_STATIC_FUNCTION_CALLBACK(JsDPoint3d, Create);

        //  Finish the initialization of the JS prototype object.
        BEJAVASCRIPT_INITIALIZE_CLASS("BentleyApi.Dgn", JsDPoint3d)
        }

    JsDPoint3d* Create(DgnJsCallContext& cc, DPoint3dCR pt) {return cc.ScheduleDelete(new JsDPoint3d(pt, m_ctx, GetJsObject()));}
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS struct JsYawPitchRollAngles : BeJsNativePointer
{
    YawPitchRollAngles m_angles;

    BeJsObjectR GetJsObject() {return *this;}

    //! Get the Yaw angle
    double GetYaw();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetYaw)
    //! Get the Pitch angle
    double GetPitch();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetPitch)
    //! Get the Roll angle
    double GetRoll();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetRoll)
    //! Set the Yaw angle
    void SetYaw(double degrees);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, SetYaw)
    //! Set the Pitch angle
    void SetPitch(double degrees);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, SetPitch)
    //! Set the Roll angle
    void SetRoll(double degrees);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, SetRoll)

    //! Create a JsYawPitchRollAngles object (JavaScript callers use this)
    static JsYawPitchRollAngles* Create(double yaw, double pitch, double roll);
    BEJAVASCRIPT_DECLARE_STATIC_CALLBACK(JsYawPitchRollAngles, Create)

    JsYawPitchRollAngles(YawPitchRollAnglesCR angles, BeJsContext& jsctx, BeJsObject& jsprototype) : BeJsNativePointer(jsctx, this, deleteThis, &jsprototype), m_angles(angles) {;}
};

typedef JsYawPitchRollAngles* JsYawPitchRollAnglesP;

BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetYaw  , Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetPitch, Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetRoll , Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsYawPitchRollAngles, SetYaw  , double, Number, GetValue)
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsYawPitchRollAngles, SetPitch, double, Number, GetValue)
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsYawPitchRollAngles, SetRoll , double, Number, GetValue)
BEJAVASCRIPT_DEFINE_STATIC_CALLBACK_NULLABLEOBJECT_3ARGS(JsYawPitchRollAngles, Create,
    JsYawPitchRollAnglesP,
    double, Number, GetValue,
    double, Number, GetValue,
    double, Number, GetValue)

double JsYawPitchRollAngles::GetYaw  () {return m_angles.GetYaw().Degrees();}
double JsYawPitchRollAngles::GetPitch() {return m_angles.GetPitch().Degrees();}
double JsYawPitchRollAngles::GetRoll () {return m_angles.GetRoll().Degrees();}
void JsYawPitchRollAngles::SetYaw  (double v) {m_angles.FromDegrees(v, GetPitch(), GetRoll());}
void JsYawPitchRollAngles::SetPitch(double v) {m_angles.FromDegrees(GetYaw(), v, GetRoll());}
void JsYawPitchRollAngles::SetRoll (double v) {m_angles.FromDegrees(GetYaw(), GetPitch(), v);}


//  Every wrapper class will have a Prototype class associated with it.
//  The prototype is paired with a JS object that has properties that are the get/set and other instance functions.
// The first argument to every prototype's Create methods be DgnScriptContextImpl::GetCallContext()
struct JsYawPitchRollAnglesPrototype
    {
    BeJsContext& m_ctx;
    BeJsContext& GetJsContext() {return m_ctx;}

    BEJAVASCRIPT_DECLARE_JS_CALLBACKS_HOLDER(JsYawPitchRollAngles)  // this just declares a member variable of type JsObject
    BeJsObject& GetJsObject() {return BEJAVASCRIPT_JS_CALLBACKS_HOLDER(JsYawPitchRollAngles);}

    JsYawPitchRollAnglesPrototype(BeJsContext& ctx) : m_ctx(ctx),
        BEJAVASCRIPT_INSTANTIATE_JS_CALLBACKS_HOLDER(JsYawPitchRollAngles)  // this constructs the JsObject
        {
        //  Set the function properties of the JS prototype object, one for each callback method
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsYawPitchRollAngles, Yaw);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsYawPitchRollAngles, Pitch);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsYawPitchRollAngles, Roll);
        BEJAVASCRIPT_INSTANTIATE_STATIC_FUNCTION_CALLBACK(JsYawPitchRollAngles, Create);

        //  Finish the initialization of the JS prototype object.
        BEJAVASCRIPT_INITIALIZE_CLASS("BentleyApi.Dgn", JsYawPitchRollAngles)
        }

    JsYawPitchRollAngles* Create(DgnJsCallContext& cc, YawPitchRollAnglesCR a) {return cc.ScheduleDelete(new JsYawPitchRollAngles(a, m_ctx, GetJsObject()));}
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS struct JsElementGeometryBuilder : BeJsNativePointer
{
#ifdef DEBUG_JS_GC
    static size_t s_count;
    static void IncrementCount() {++s_count;}
    static void DecrementCount() {BeAssert(s_count > 0); --s_count;;}
#else
    static void IncrementCount() {;}
    static void DecrementCount() {;}
#endif
    ElementGeometryBuilderPtr m_builder;

    JsElementGeometryBuilder(DgnElement3dR el, DPoint3dCR o, YawPitchRollAnglesCR angles, BeJsContext& jsctx, BeJsObject& jsprototype) 
        : BeJsNativePointer(jsctx, this, deleteThis, &jsprototype),
        m_builder (ElementGeometryBuilder::Create(el, o, angles)) 
        {
        IncrementCount();
        }

    JsElementGeometryBuilder(DgnElement2dR el, DPoint2dCR o, AngleInDegrees angle, BeJsContext& jsctx, BeJsObject& jsprototype)
        : BeJsNativePointer(jsctx, this, deleteThis, &jsprototype),
        m_builder (ElementGeometryBuilder::Create(el, o, angle)) 
        {
        IncrementCount();
        }

    ~JsElementGeometryBuilder()
        {
        DecrementCount();
        }

    BeJsObjectR GetJsObject() {return *this;}

    static JsElementGeometryBuilder* Create(JsDgnElementP el, JsDPoint3dP o, JsYawPitchRollAnglesP angles);
    BEJAVASCRIPT_DECLARE_STATIC_CALLBACK(JsElementGeometryBuilder, Create)

    void AppendBox(double x, double y, double z);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsElementGeometryBuilder, AppendBox);

    BentleyStatus SetGeomStreamAndPlacement (JsDgnElementP element);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsElementGeometryBuilder, SetGeomStreamAndPlacement);
};
typedef JsElementGeometryBuilder* JsElementGeometryBuilderP;

#ifdef DEBUG_JS_GC
size_t JsElementGeometryBuilder::s_count;
#endif

BEJAVASCRIPT_DEFINE_STATIC_CALLBACK_NULLABLEOBJECT_3ARGS(JsElementGeometryBuilder, Create,
    JsElementGeometryBuilderP,
    JsDgnElementP, NativePointer, GetValue,
    JsDPoint3dP, NativePointer, GetValue,
    JsYawPitchRollAnglesP, NativePointer, GetValue)

BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_3ARGS(JsElementGeometryBuilder, AppendBox, 
    double, Number, GetValue, 
    double, Number, GetValue, 
    double, Number, GetValue)

BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_1ARG(JsElementGeometryBuilder, SetGeomStreamAndPlacement,
    Number, BentleyStatus,
    JsDgnElementP, NativePointer, GetValue)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
BentleyStatus JsElementGeometryBuilder::SetGeomStreamAndPlacement(JsDgnElementP el) {return m_builder->SetGeomStreamAndPlacement(*el->m_el->ToGeometricElementP());}

//---------------------------------------------------------------------------------------
// *** TEMPORARY METHOD *** 
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
void JsElementGeometryBuilder::AppendBox(double x, double y, double z)
    {
    // *** TEMPORARY METHOD *** 
    DPoint3d localOrigin;
    localOrigin.x = 0.0;
    localOrigin.y = 0.0;
    localOrigin.z = 0.0;

    DPoint3d localTop (localOrigin);
    localTop.z = z;

    DVec3d localX = DVec3d::From(1,0,0);
    DVec3d localY = DVec3d::From(0,1,0);

    DgnBoxDetail boxd(localOrigin, localTop, localX, localY, x, y, x, y, true);
    ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnBox(boxd);

    m_builder->Append(*solid);
    }

//  Every wrapper class will have a Prototype class associated with it.
//  The prototype is paired with a JS object that has properties that are the get/set and other instance functions.
// The first argument to every prototype's Create methods be DgnScriptContextImpl::GetCallContext()
struct JsElementGeometryBuilderPrototype
    {
    BeJsContext& m_ctx;
    BeJsContext& GetJsContext() {return m_ctx;}

    BEJAVASCRIPT_DECLARE_JS_CALLBACKS_HOLDER(JsElementGeometryBuilder)  // this just declares a member variable of type JsObject
    BeJsObject& GetJsObject() {return BEJAVASCRIPT_JS_CALLBACKS_HOLDER(JsElementGeometryBuilder);}

    JsElementGeometryBuilderPrototype(BeJsContext& ctx) : m_ctx(ctx),
        BEJAVASCRIPT_INSTANTIATE_JS_CALLBACKS_HOLDER(JsElementGeometryBuilder)  // this constructs the JsObject
        {
        //  Set the function properties of the JS prototype object, one for each callback method
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_FUNCTION_CALLBACK(JsElementGeometryBuilder, AppendBox);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_FUNCTION_CALLBACK(JsElementGeometryBuilder, SetGeomStreamAndPlacement);
        BEJAVASCRIPT_INSTANTIATE_STATIC_FUNCTION_CALLBACK(JsElementGeometryBuilder, Create);
        
        //  Finish the initialization of the JS prototype object.
        BEJAVASCRIPT_INITIALIZE_CLASS("BentleyApi.Dgn", JsElementGeometryBuilder)
        }

    JsElementGeometryBuilder* Create(DgnJsCallContext& cc, DgnElement3dR el, DPoint3dCR o, YawPitchRollAnglesCR angles) {return cc.ScheduleDelete(new JsElementGeometryBuilder(el, o, angles, m_ctx, GetJsObject()));}
    JsElementGeometryBuilder* Create(DgnJsCallContext& cc, DgnElement2dR el, DPoint2dCR o, AngleInDegrees angle) {return cc.ScheduleDelete(new JsElementGeometryBuilder(el, o, angle, m_ctx, GetJsObject()));}
    };

//=======================================================================================
// This class holds the one and only BeJsEnvironment and BeJsContext for the session.
// It defines all of the DgnScript Object Model proxies and registers them in Script.
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct DgnScriptContextImpl : BeJsContext
{
    struct Prototypes
        {
        JsDgnElementPrototype         m_dgnElement;
        JsDPoint3dPrototype           m_dpoint3d;
        JsYawPitchRollAnglesPrototype m_angles;
        JsElementGeometryBuilderPrototype m_builder;
        JsDgnModelPrototype           m_dgnModel;

        Prototypes(BeJsContext& ctx) : m_dgnElement(ctx), m_dpoint3d(ctx), m_angles(ctx), m_builder(ctx), m_dgnModel(ctx)
            {;}
        };

    // -------------------------------
    // Member variables
    // -------------------------------
    BeJsObject m_egaRegistry;
    BeJsObject m_modelSolverRegistry;
    bset<Utf8String> m_jsScriptsExecuted;
    Prototypes m_prototypes;
    DgnJsCallContext m_callcontext;

    // -------------------------------
    // Member functions
    // -------------------------------
    DgnScriptContextImpl(BeJsEnvironmentR);
    ~DgnScriptContextImpl();
    DgnDbStatus LoadProgram(Dgn::DgnDbR db, Utf8CP tsFunctionSpec);
    DgnDbStatus ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms);
    DgnDbStatus ExecuteModelSolver(int& functionReturnStatus, Dgn::DgnModelR model, Utf8CP jsFunctionName, Json::Value const& parms);

    DgnJsCallContext& GetCallContext() {return m_callcontext;}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsElementGeometryBuilder* JsElementGeometryBuilder::Create(JsDgnElementP e, JsDPoint3dP o, JsYawPitchRollAnglesP a)
    {
    DgnScriptContextImpl* holder = static_cast<DgnScriptContextImpl*>(const_cast<BeJsContext*>(&e->GetContext()));

    DgnElement3dP e3d = dynamic_cast<DgnElement3dP>(e->m_el.get());
    if (nullptr != e3d)
        return holder->m_prototypes.m_builder.Create(holder->GetCallContext(), *e3d, o->m_pt, a->m_angles);

    DgnElement2dP e2d = dynamic_cast<DgnElement2dP>(e->m_el.get());
    if (nullptr != e2d)
        return holder->m_prototypes.m_builder.Create(holder->GetCallContext(), *e2d, DPoint2d::From(o->GetX(), o->GetY()), AngleInDegrees::FromDegrees(a->GetYaw()));

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsDPoint3d* JsDPoint3d::Create(double x, double y, double z)
    {
    DgnScriptContextImpl* holder = dynamic_cast<DgnScriptContext*>(&T_HOST.GetScriptingAdmin().GetDgnScriptContext())->GetImpl();
    DPoint3d pt = DPoint3d::From(x, y, z);
    return holder->m_prototypes.m_dpoint3d.Create(holder->GetCallContext(), pt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsYawPitchRollAngles* JsYawPitchRollAngles::Create(double yaw, double pitch, double roll)
    {
    DgnScriptContextImpl* holder = dynamic_cast<DgnScriptContext*>(&T_HOST.GetScriptingAdmin().GetDgnScriptContext())->GetImpl();
    YawPitchRollAngles angles = YawPitchRollAngles::FromDegrees(yaw, pitch, roll);
    return holder->m_prototypes.m_angles.Create(holder->GetCallContext(), angles);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsDgnElement* JsDgnModel::CreateElement(Utf8StringCR ecSqlClassName, Utf8StringCR categoryName)
    {
    DgnScriptContextImpl* holder = dynamic_cast<DgnScriptContext*>(&T_HOST.GetScriptingAdmin().GetDgnScriptContext())->GetImpl();
    DgnCategoryId catid = m_model->GetDgnDb().Categories().QueryCategoryId(categoryName.c_str());
    RefCountedCPtr<PhysicalElement> el = createPhysicalElement(*m_model, ecSqlClassName.c_str(), catid, nullptr);
    return holder->m_prototypes.m_dgnElement.Create(holder->GetCallContext(), *const_cast<PhysicalElementP>(el.get()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContextImpl::DgnScriptContextImpl(BeJsEnvironmentR jsenv)
    : 
    BeJsContext(jsenv, "DgnScriptContext", dgnScriptContext_GetBootstrappingSource()),
    m_egaRegistry(EvaluateScript("BentleyApi.Dgn.GetEgaRegistry()")),
    m_modelSolverRegistry(EvaluateScript("BentleyApi.Dgn.GetModelSolverRegistry()")),
    m_prototypes(*this)
    {
    BeAssert(!m_egaRegistry.IsUndefined() && m_egaRegistry.IsObject());
    BeAssert(!m_modelSolverRegistry.IsUndefined() && m_modelSolverRegistry.IsObject());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContextImpl::~DgnScriptContextImpl()
    {
#ifdef DEBUG_JS_GC
    BeAssert(0 == JsElementGeometryBuilder::s_count);
#endif
    BeAssert(m_callcontext.m_localsToBeDeleted.empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContextImpl::LoadProgram(Dgn::DgnDbR db, Utf8CP jsFunctionSpec)
    {
    Utf8String jsProgramName;
    Utf8CP dot = strrchr(jsFunctionSpec, '.');
    if (nullptr == dot)
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] is an illegal Script function spec. Must be of the form program.function", jsFunctionSpec);
        BeAssert(false && "illegal Script function spec");
        return DgnDbStatus::BadArg;
        }

    jsProgramName.assign(jsFunctionSpec, dot);

    if (m_jsScriptsExecuted.find(jsProgramName) != m_jsScriptsExecuted.end())
        return DgnDbStatus::Success;

    DgnScriptType sTypePreferred = DgnScriptType::JavaScript;
    DgnScriptType sTypeFound;
    Utf8String jsprog;
    DgnDbStatus status = T_HOST.GetScriptingAdmin()._FetchScript(jsprog, sTypeFound, db, jsProgramName.c_str(), sTypePreferred);
    if (DgnDbStatus::Success != status)
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->infov ("Script program %s is not registered in the script library", jsProgramName.c_str());
        return status;
        }

    if (sTypeFound != DgnScriptType::JavaScript)
        {
        switch (sTypeFound)
            {
            case DgnScriptType::TypeScript:
                BeAssert(false && "*** TBD: compile script type on the fly");
                return DgnDbStatus::NotEnabled;
                break;
            default:
                BeAssert(false && "Unsupported script type");
                return DgnDbStatus::NotEnabled;
                break;
            }
        }

    m_jsScriptsExecuted.insert(jsProgramName);

    Utf8String fileUrl("file:///"); // This does not really identify a file. It is something tricky that is needed to get JS to accept the script that we pass in jsprog.
    fileUrl.append(jsProgramName);
    fileUrl.append(".js");

    NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->tracev ("Evaluating %s", jsProgramName.c_str());

    EvaluateScript(jsprog.c_str(), fileUrl.c_str());   // evaluate the whole script, allowing it to define objecjs and their properties. 
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContextImpl::ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms)
    {
    functionReturnStatus = -1;

    DgnDbStatus status = LoadProgram(el.GetDgnDb(), jsEgaFunctionName);
    if (DgnDbStatus::Success != status)
        return status;

    BeJsFunction jsfunc = m_egaRegistry.GetFunctionProperty(jsEgaFunctionName);
    if (jsfunc.IsUndefined() || !jsfunc.IsFunction())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] is not registered as an EGA", jsEgaFunctionName);
        BeAssert(false && "EGA not registered");
        return DgnDbStatus::NotEnabled;
        }

    BeJsObject parmsObj = EvaluateJson(parms);
    GetCallContext().Enter();
    JsDgnElement* jsel = m_prototypes.m_dgnElement.Create(GetCallContext(), el);
    JsDPoint3d* jsorigin = m_prototypes.m_dpoint3d.Create(GetCallContext(), origin);
    JsYawPitchRollAngles* jsangles = m_prototypes.m_angles.Create(GetCallContext(), angles);
    BeJsValue retval = jsfunc(*jsel, *jsorigin, *jsangles, parmsObj);
    GetCallContext().Leave();

    if (!retval.IsNumber())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] does not have the correct signature for an EGA - must return an int", jsEgaFunctionName);
        BeAssert(false && "EGA has incorrect return type");
        return DgnDbStatus::NotEnabled;
        }

    BeJsNumber num(retval);
    functionReturnStatus = num.GetIntegerValue();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContextImpl::ExecuteModelSolver(int& functionReturnStatus, Dgn::DgnModelR model, Utf8CP jsFunctionName, Json::Value const& parms)
    {
    functionReturnStatus = -1;

    DgnDbStatus status = LoadProgram(model.GetDgnDb(), jsFunctionName);
    if (DgnDbStatus::Success != status)
        return status;

    BeJsFunction jsfunc = m_modelSolverRegistry.GetFunctionProperty(jsFunctionName);
    if (jsfunc.IsUndefined() || !jsfunc.IsFunction())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] is not registered as a model solver", jsFunctionName);
        BeAssert(false && "model solver not registered");
        return DgnDbStatus::NotEnabled;
        }

    BeJsObject parmsObj = EvaluateJson(parms);
    GetCallContext().Enter();
    JsDgnModel* jsModel = m_prototypes.m_dgnModel.Create(GetCallContext(), model);
    BeJsValue retval = jsfunc(*jsModel, parmsObj);
    GetCallContext().Leave();

    if (!retval.IsNumber())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] does not have the correct signature for a model solver - must return an int", jsFunctionName);
        BeAssert(false && "model solver has incorrect return type");
        return DgnDbStatus::NotEnabled;
        }

    BeJsNumber num(retval);
    functionReturnStatus = num.GetIntegerValue();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContext::DgnScriptContext(BeJsEnvironmentR jsenv)
    {
    m_pimpl = new DgnScriptContextImpl(jsenv);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContext::~DgnScriptContext()
    {
    delete m_pimpl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms)
    {
    return m_pimpl->ExecuteEga(functionReturnStatus, el, jsEgaFunctionName, origin, angles, parms);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::ExecuteModelSolver(int& functionReturnStatus, Dgn::DgnModelR model, Utf8CP jsFunctionName, Json::Value const& parms)
    {
    return m_pimpl->ExecuteModelSolver(functionReturnStatus, model, jsFunctionName, parms);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptingAdmin::ScriptingAdmin()
    {
    m_jsenv = nullptr;
    m_dgnContext = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptingAdmin::~ScriptingAdmin()
    {
    if (nullptr != m_dgnContext)
        delete m_dgnContext;
    if (nullptr != m_jsenv)
        delete m_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsEnvironmentR DgnPlatformLib::Host::ScriptingAdmin::GetBeJsEnvironment()
    {
    if (nullptr == m_jsenv)
        m_jsenv = new BeJsEnvironment;
    return *m_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnScriptContextR DgnPlatformLib::Host::ScriptingAdmin::GetDgnScriptContext()
    {
    if (nullptr == m_dgnContext)
        m_dgnContext = new DgnScriptContext(GetBeJsEnvironment());
    return *m_dgnContext;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::ScriptingAdmin::_FetchScript(Utf8StringR sText, DgnScriptType& stypeFound, DgnDbR db, Utf8CP sName, DgnScriptType stypePreferred)
    {
    DgnScriptLibrary jslib(db);
    return jslib.QueryScript(sText, stypeFound, sName, stypePreferred);
    }
