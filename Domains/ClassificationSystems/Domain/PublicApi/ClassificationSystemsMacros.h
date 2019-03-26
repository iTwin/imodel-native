/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/PublicApi/ClassificationSystemsMacros.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

#define BENTLEY_CLASSIFICATIONSYSTEMS_NAMESPACE_NAME BENTLEY_NAMESPACE_NAME::ClassificationSystems
#define BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace ClassificationSystems {
#define END_CLASSIFICATIONSYSTEMS_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_CLASSIFICATIONSYSTEMS using namespace BENTLEY_CLASSIFICATIONSYSTEMS_NAMESPACE_NAME;

#define CLASSIFICATIONSYSTEMS_SCHEMA_NAME                         "ClassificationSystems"

#define CLASSIFICATIONSYSTEMS_SCHEMA(className)                   CLASSIFICATIONSYSTEMS_SCHEMA_NAME "." className

#define CLASSIFICATIONSYSTEMS_SCHEMA_PATH                          L"ECSchemas/Domain/ClassificationSystems.ecschema.xml"

#define CLASSIFICATIONSYSTEMS_CODESPEC_CODE(categoryName)          CLASSIFICATIONSYSTEMS_SCHEMA_NAME "::" categoryName

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_PTR(_name_) \
    END_CLASSIFICATIONSYSTEMS_NAMESPACE

#define DECLARE_CLASSIFICATIONSYSTEMS_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_CLASS_##__name__)); }

#define DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(__name__, __exportstr__) \
    DECLARE_CLASSIFICATIONSYSTEMS_QUERYCLASS_METHODS(__name__) \
    __exportstr__ static __name__##CPtr Get       (Dgn::DgnDbR, Dgn::DgnElementId); \
    __exportstr__ static __name__##Ptr  GetForEdit(Dgn::DgnDbR, Dgn::DgnElementId); \
    __exportstr__        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr); \
    __exportstr__        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr); 

#define DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(__name__) \
    __name__##CPtr __name__::Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    __name__##Ptr  __name__::GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); } \
    __name__##CPtr __name__::Insert    (Dgn::DgnDbStatus* stat)         { return GetDgnDb().Elements().Insert< __name__ >(*this, stat);} \
    __name__##CPtr __name__::Update    (Dgn::DgnDbStatus* stat)         { return GetDgnDb().Elements().Update< __name__ >(*this, stat);}

#if defined (__CLASSIFICATIONSYSTEMSDOMAIN_BUILD__)
#define CLASSIFICATIONSYSTEMSDOMAIN_EXPORT EXPORT_ATTRIBUTE
#else
#define CLASSIFICATIONSYSTEMSDOMAIN_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__CLASSIFICATIONSYSTEMSELEMENTS_BUILD__)
#define CLASSIFICATIONSYSTEMSELEMENTS_EXPORT EXPORT_ATTRIBUTE
#else
#define CLASSIFICATIONSYSTEMSELEMENTS_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__CLASSIFICATIONSYSTEMSHANDLERS_BUILD__)
#define CLASSIFICATIONSYSTEMSHANDLERS_EXPORT EXPORT_ATTRIBUTE
#else
#define CLASSIFICATIONSYSTEMSHANDLERS_EXPORT IMPORT_ATTRIBUTE
#endif

//-----------------------------------------------------------------------------------------
// ECClass names (combine with CLASSIFICATIONSYSTEMS_ECCHEMA_NAME macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define CLASSIFICATIONSYSTEMS_CLASS_ASHRAEClassDefinition                                   "ASHRAEClassDefinition"
#define CLASSIFICATIONSYSTEMS_CLASS_ASHRAE2004ClassDefinition                               "ASHRAE2004ClassDefinition"
#define CLASSIFICATIONSYSTEMS_CLASS_ASHRAE2010ClassDefinition                               "ASHRAE2010ClassDefinition"
#define CLASSIFICATIONSYSTEMS_CLASS_CIBSEClassDefinition                                    "CIBSEClassDefinition"
#define CLASSIFICATIONSYSTEMS_CLASS_Classification                                          "Classification"
#define CLASSIFICATIONSYSTEMS_CLASS_ClassificationGroup                                     "ClassificationGroup"
#define CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem                                    "ClassificationSystem"
#define CLASSIFICATIONSYSTEMS_CLASS_ClassificationTable                                     "ClassificationTable"
#define CLASSIFICATIONSYSTEMS_CLASS_MasterFormatClassDefinition                             "MasterFormatClassDefinition"
#define CLASSIFICATIONSYSTEMS_CLASS_OmniClassClassDefinition                                "OmniClassClassDefinition"
#define CLASSIFICATIONSYSTEMS_CLASS_UniFormatClassDefinition                                "UniFormatClassDefinition"


#define CLASSIFICATIONSYSTEMS_REL_ClassificationGroupGroupsClassifications                         "ClassificationGroupGroupsClassifications"
#define CLASSIFICATIONSYSTEMS_REL_ClassificationSpecializesClassification                          "ClassificationSpecializesClassification"
#define CLASSIFICATIONSYSTEMS_REL_IClassifiedIsClassifiedAs                                        "IClassifiedIsClassifiedAs"
