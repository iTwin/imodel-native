/*--------------------------------------------------------------------------------------+
|
|     $Source: ConstraintModel/PublicApi/ConstraintModelMacros.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

#define BEGIN_CONSTRAINTMODEL_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace ConstraintModel {
#define END_CONSTRAINTMODEL_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_CONSTRAINTMODEL using namespace BentleyApi::ConstraintModel;

#define DECLARE_CONSTRAINTMODEL_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetECClassId(CONSTRAINTMODEL_SCHEMA_NAME, CONSTRAINTMODEL_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetECClass(CONSTRAINTMODEL_SCHEMA_NAME, CONSTRAINTMODEL_CLASS_##__name__)); }

#define DECLARE_CONSTRAINTMODEL_ELEMENT_BASE_METHODS(__name__, __exportstr__) \
    DECLARE_CONSTRAINTMODEL_QUERYCLASS_METHODS(__name__) \
    __exportstr__ static __name__##CPtr Get       (Dgn::DgnDbR, Dgn::DgnElementId); \
    __exportstr__ static __name__##Ptr  GetForEdit(Dgn::DgnDbR, Dgn::DgnElementId); \
    __exportstr__        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr); \
    __exportstr__        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr);  

#if defined (__CONSTRAINTMODEL_BUILD__)
#define CONSTRAINTMODEL_EXPORT EXPORT_ATTRIBUTE
#else
#define CONSTRAINTMODEL_EXPORT IMPORT_ATTRIBUTE
#endif

#define DEFINE_CONSTRAINTMODEL_ELEMENT_BASE_METHODS(__name__) \
    __name__##CPtr __name__::Get       (DgnDbR db, DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    __name__##Ptr  __name__::GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); } \
    __name__##CPtr __name__::Insert    (DgnDbStatus* stat)         { return GetDgnDb().Elements().Insert< __name__ >(*this, stat);} \
    __name__##CPtr __name__::Update    (DgnDbStatus* stat)         { return GetDgnDb().Elements().Update< __name__ >(*this, stat);}

#define CONSTRAINTMODEL_SCHEMA_NAME                                     "ConstraintModel"
#define CONSTRAINTMODEL_SCHEMA_PATH                                     L"ECSchemas/Domain/ConstraintModel.01.00.ecschema.xml"

#define CONSTRAINTMODEL_SCHEMA(className)                               CONSTRAINTMODEL_SCHEMA_NAME "." className
#define CONSTRAINTMODEL_SCHEMA_CODE(categoryName)                       CONSTRAINTMODEL_SCHEMA_NAME "_" categoryName

#define CONSTRAINTMODEL_REL_ElementConstrainsElement                   "ElementConstrainsElement"
#define CONSTRAINTMODEL_REL_ElementOffsetsElement                      "ElementOffsetsElement"
#define CONSTRAINTMODEL_REL_ElementCoincidesElement                    "ElementCoincidesElement"

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define CONSTRAINTMODEL_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_CONSTRAINTMODEL_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_PTR(_name_) \
    END_CONSTRAINTMODEL_NAMESPACE

    
    