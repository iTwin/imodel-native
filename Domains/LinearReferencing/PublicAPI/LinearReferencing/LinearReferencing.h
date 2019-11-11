/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/BeAssert.h>
#include <Bentley/Nullable.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECUnits/Units.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>

#ifdef __LINEARREFERENCING_BUILD__
#define LINEARREFERENCING_EXPORT EXPORT_ATTRIBUTE
#else
#define LINEARREFERENCING_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::LinearReferencing %LinearReferencing data types */
#define BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace LinearReferencing {
#define END_BENTLEY_LINEARREFERENCING_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_LINEARREFERENCING        using namespace BENTLEY_NAMESPACE_NAME::LinearReferencing;

// create the Bentley.ConceptCivil namespace
BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

    typedef Nullable<double> NullableDouble;

END_BENTLEY_LINEARREFERENCING_NAMESPACE

//-----------------------------------------------------------------------------------------
// ECSchema name and relative path
//-----------------------------------------------------------------------------------------
#define BLR_SCHEMA_NAME                             "LinearReferencing"
#define BLR_SCHEMA_FILE                             L"LinearReferencing.ecschema.xml"
#define BLR_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BLR_SCHEMA_PATH                             BLR_SCHEMA_LOCATION BLR_SCHEMA_FILE
#define BLR_SCHEMA(name)                            BLR_SCHEMA_NAME "." name
#define BLR_SCHEMA_CODE(name)                       BLR_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------
#define BLR_CLASS_ILinearElement                                    "ILinearElement"
#define BLR_CLASS_ILinearlyLocated                                  "ILinearlyLocated"
#define BLR_CLASS_ILinearlyLocatedAttribution                       "ILinearlyLocatedAttribution"
#define BLR_CLASS_ILinearLocationElement                            "ILinearLocationElement"
#define BLR_CLASS_LinearLocation                                    "LinearLocation"
#define BLR_CLASS_LinearLocationElement                             "LinearLocationElement"
#define BLR_CLASS_LinearlyLocatedAttribution                        "LinearlyLocatedAttribution"
#define BLR_CLASS_LinearlyReferencedAtLocation                      "LinearlyReferencedAtLocation"
#define BLR_CLASS_LinearlyReferencedFromToLocation                  "LinearlyReferencedFromToLocation"
#define BLR_CLASS_LinearlyReferencedLocation                        "LinearlyReferencedLocation"
#define BLR_CLASS_LinearPhysicalElement                             "LinearPhysicalElement"
#define BLR_CLASS_ReferentElement                                   "ReferentElement"
#define BLR_CLASS_Referent                                          "Referent"


//-----------------------------------------------------------------------------------------
// ECRelationship names
//-----------------------------------------------------------------------------------------
#define BLR_REL_GeometricElementDrivesReferent                      "GeometricElementDrivesReferent"
#define BLR_REL_ILinearElementProvidedBySource                      "ILinearElementProvidedBySource"
#define BLR_REL_ILinearLocationLocatesElement                       "ILinearLocationLocatesElement"
#define BLR_REL_ILinearlyLocatedAlongILinearElement                 "ILinearlyLocatedAlongILinearElement"
#define BLR_REL_ILinearlyLocatedAttributesElement                   "ILinearlyLocatedAttributesElement"


//-----------------------------------------------------------------------------------------
// ECProperty names
//-----------------------------------------------------------------------------------------
#define BLR_PROP_ILinearElement_LengthValue                         "LengthValue"
#define BLR_PROP_ILinearElement_Source                              "LinearElementSource"
#define BLR_PROP_ILinearElement_StartValue                          "StartValue"
#define BLR_PROP_ILinearlyLocated_LocatedElement                    "LocatedElement"
#define BLR_PROP_ILinearlyLocatedAttribution_AttributedElement      "AttributedElement"
#define BLR_PROP_IReferent_ReferencedElement                        "ReferencedElement"


//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BLR_SCHEMA_NAME, BLR_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BLR_SCHEMA_NAME, BLR_CLASS_##__name__)); }

//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_LINEARREFERENCING_ELEMENT_GET_METHODS(__name__, __dgnelementname__) \
    LINEARREFERENCING_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { return new __name__(*db.Elements().Get< __dgnelementname__ >(id)); } \
    LINEARREFERENCING_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return new __name__(*db.Elements().GetForEdit< __dgnelementname__ >(id)); }

//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_LINEARREFERENCING_LINEARLYLOCATED_SET_METHODS(__name__, __dgnelementname__) \
    __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { \
        __name__##CPtr retCPtr = new __name__(*getP()->GetDgnDb().Elements().Insert< __dgnelementname__ >(*getP(), stat)); Dgn::DgnDbStatus status = Dgn::DgnDbStatus::Success; \
        if (retCPtr.IsNull() || Dgn::DgnDbStatus::Success != (status = _InsertLinearElementRelationship())) { if (stat) *stat = status; return nullptr; } return retCPtr; } \
    __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { \
        __name__##CPtr retCPtr = new __name__(*getP()->GetDgnDb().Elements().Update< __dgnelementname__ >(*getP(), stat)); Dgn::DgnDbStatus status = Dgn::DgnDbStatus::Success; \
        if (retCPtr.IsNull() || Dgn::DgnDbStatus::Success != (status = _UpdateLinearElementRelationship())) { if (stat) *stat = status; return nullptr; } return retCPtr; }


//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the Conceptual namespace
//-----------------------------------------------------------------------------------------
#define LINEARREFERENCING_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_LINEARREFERENCING_NAMESPACE


//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define LINEARREFERENCING_REFCOUNTED_PTR(_name_) \
    BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_BENTLEY_LINEARREFERENCING_NAMESPACE 


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the LinearReferencing namespace
//-----------------------------------------------------------------------------------------
LINEARREFERENCING_TYPEDEFS(DistanceExpression)
LINEARREFERENCING_TYPEDEFS(ILinearElement)
LINEARREFERENCING_TYPEDEFS(ILinearElementSource)
LINEARREFERENCING_TYPEDEFS(ILinearlyLocated)
LINEARREFERENCING_TYPEDEFS(ILinearLocationElement)
LINEARREFERENCING_TYPEDEFS(IReferent)
LINEARREFERENCING_TYPEDEFS(ISegmentableLinearElement)
LINEARREFERENCING_TYPEDEFS(ISpatialLinearElement)
LINEARREFERENCING_TYPEDEFS(LinearLocation)
LINEARREFERENCING_TYPEDEFS(LinearLocationElement)
LINEARREFERENCING_TYPEDEFS(LinearLocationReference)
LINEARREFERENCING_TYPEDEFS(LinearlyLocatedAttribution)
LINEARREFERENCING_TYPEDEFS(LinearlyReferencedLocation)
LINEARREFERENCING_TYPEDEFS(LinearlyReferencedAtLocation)
LINEARREFERENCING_TYPEDEFS(LinearlyReferencedFromToLocation)
LINEARREFERENCING_TYPEDEFS(LinearPhysicalElement)
LINEARREFERENCING_TYPEDEFS(Referent)
LINEARREFERENCING_TYPEDEFS(ReferentElement)

LINEARREFERENCING_REFCOUNTED_PTR(LinearlyReferencedAtLocation)
LINEARREFERENCING_REFCOUNTED_PTR(LinearlyReferencedFromToLocation)
LINEARREFERENCING_REFCOUNTED_PTR(LinearLocation)
LINEARREFERENCING_REFCOUNTED_PTR(Referent)

#define DGNELEMENTWRAPPER_DECLARE_MEMBERS(__superclass__, __dgnelementclass__) \
    private: typedef __superclass__ T_Super; \
    protected: static RefCountedPtr<__dgnelementclass__> Create(Dgn::DgnDbR dgnDb, __dgnelementclass__::CreateParams const& params) { \
        auto ecInstancePtr = dgnDb.Schemas().GetClass(params.m_classId)->GetDefaultStandaloneEnabler()->CreateInstance(); \
        ParamsToInstance(dgnDb, params, *ecInstancePtr); \
        auto newElementPtr = dgnDb.Elements().Create<__dgnelementclass__>(*ecInstancePtr); \
        if (params.m_parentId.IsValid()) \
            newElementPtr->SetParentId(params.m_parentId, params.m_parentRelClassId); \
        return newElementPtr; }

template<typename T>
struct DgnElementWrapper: BentleyApi::RefCountedBase, BentleyApi::NonCopyableClass
{    
private:
    BentleyApi::RefCountedCPtr<T> m_wrappedCPtr;
    BentleyApi::RefCountedPtr<T> m_wrappedPtr;

protected:
    virtual ~DgnElementWrapper() {}
    explicit DgnElementWrapper(T const& dgnElementCR) : m_wrappedCPtr(&dgnElementCR) { BeAssert(m_wrappedCPtr.IsValid()); }
    explicit DgnElementWrapper(T& dgnElementR) : m_wrappedPtr(&dgnElementR) { BeAssert(m_wrappedPtr.IsValid()); }

    static void ParamsToInstance(BentleyApi::Dgn::DgnDbR dgnDb, typename T::CreateParams const& params, BentleyApi::ECN::IECInstanceR ecInstance)
        {
        ecInstance.SetValue("Model", BentleyApi::ECN::ECValue(params.m_modelId));
        ecInstance.SetValue("CodeSpec", BentleyApi::ECN::ECValue(params.m_code.GetCodeSpecId()));
        ecInstance.SetValue("CodeScope", BentleyApi::ECN::ECValue(params.m_code.GetScopeElementId(dgnDb)));
        ecInstance.SetValue("CodeValue", BentleyApi::ECN::ECValue(params.m_code.GetValueUtf8CP()));
        }

public:
    T const* get() const { return m_wrappedPtr.IsValid() ? m_wrappedPtr.get() : (m_wrappedCPtr.IsValid() ? m_wrappedCPtr.get() : m_wrappedPtr.get()); }
    T* getP() { return m_wrappedPtr.get(); }

    BentleyApi::Dgn::DgnElementId GetElementId() const { return get()->GetElementId(); }
    BentleyApi::Dgn::DgnDbR GetDgnDb() const { return get()->GetDgnDb(); }
    BentleyApi::Dgn::DgnModelId GetModelId() const { return get()->GetModelId(); }
    BentleyApi::Dgn::DgnModelId GetSubModelId() const { return get()->GetSubModelId(); }
    BentleyApi::Dgn::DgnModelPtr GetModel() const { return get()->GetModel(); }
    BentleyApi::Dgn::DgnModelPtr GetSubModel() const { return get()->GetSubModel(); }
    BentleyApi::Dgn::DgnCodeCR GetCode() const { return get()->GetCode(); }
    void SetCode(BentleyApi::Dgn::DgnCode code) { getP()->SetCode(code); }
    BentleyApi::Utf8CP GetUserLabel() const { return get()->GetUserLabel(); }
    void SetUserLabel(BentleyApi::Utf8CP val) { getP()->SetUserLabel(val); }
}; // DgnElementWrapper

template <typename T>
struct GeometricElementWrapper : DgnElementWrapper<T>
{
protected:
    explicit GeometricElementWrapper(T const& element) : DgnElementWrapper<T>(element) {}
    explicit GeometricElementWrapper(T& element) : DgnElementWrapper<T>(element) {}

    static void ParamsToInstance(BentleyApi::Dgn::DgnDbR dgnDb, typename T::CreateParams const& params, BentleyApi::ECN::IECInstanceR ecInstance)
        {
        DgnElementWrapper<T>::ParamsToInstance(dgnDb, params, ecInstance);
        ecInstance.SetValue("Category", BentleyApi::ECN::ECValue(params.m_category));
        }

public:
    BentleyApi::Dgn::DgnCategoryId GetCategoryId() const { return this->get()->GetCategoryId(); }
}; // GeometricElementWrapper
