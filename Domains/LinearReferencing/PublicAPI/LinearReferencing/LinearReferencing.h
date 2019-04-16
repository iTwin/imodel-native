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
#define BLR_CLASS_GeometricElementAsReferent                        "GeometricElementAsReferent"
#define BLR_CLASS_ILinearElement                                    "ILinearElement"
#define BLR_CLASS_ILinearlyLocated                                  "ILinearlyLocated"
#define BLR_CLASS_ILinearlyLocatedAttribution                       "ILinearlyLocatedAttribution"
#define BLR_CLASS_ILinearlyLocatedElement                           "ILinearlyLocatedElement"
#define BLR_CLASS_LinearlyReferencedAtLocation                      "LinearlyReferencedAtLocation"
#define BLR_CLASS_LinearlyReferencedFromToLocation                  "LinearlyReferencedFromToLocation"
#define BLR_CLASS_LinearlyReferencedLocation                        "LinearlyReferencedLocation"


//-----------------------------------------------------------------------------------------
// ECCustomAttribute names
//-----------------------------------------------------------------------------------------
#define BLR_CA_ILinearlyLocatedSegmentationHints                    "ILinearlyLocatedSegmentationHints"
#define BLR_CAPROP_SupportedLinearlyReferencedLocationTypes         "SupportedLinearlyReferencedLocationTypes"

//-----------------------------------------------------------------------------------------
// ECRelationship names
//-----------------------------------------------------------------------------------------
#define BLR_REL_GeometricElementDrivesReferent                      "GeometricElementDrivesReferent"
#define BLR_REL_ILinearlyLocatedAlongILinearElement                 "ILinearlyLocatedAlongILinearElement"
#define BLR_REL_ILinearElementSourceProvidesILinearElements         "ILinearElementSourceProvidesILinearElements"


//-----------------------------------------------------------------------------------------
// ECProperty names
//-----------------------------------------------------------------------------------------
#define BLR_PROP_ILinearlyLocated_ILinearElement                    "ILinearElement"


//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_LINEARREFERENCING_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BLR_SCHEMA_NAME, BLR_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BLR_SCHEMA_NAME, BLR_CLASS_##__name__)); }


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
LINEARREFERENCING_TYPEDEFS(GeometricElementAsReferent)
LINEARREFERENCING_TYPEDEFS(ICascadeLinearLocationChangesAlgorithm)
LINEARREFERENCING_TYPEDEFS(ILinearElement)
LINEARREFERENCING_TYPEDEFS(ILinearElementSource)
LINEARREFERENCING_TYPEDEFS(ILinearlyLocated)
LINEARREFERENCING_TYPEDEFS(ILinearlyLocatedElement)
LINEARREFERENCING_TYPEDEFS(IReferent)
LINEARREFERENCING_TYPEDEFS(ISegmentableLinearElement)
LINEARREFERENCING_TYPEDEFS(ISpatialLinearElement)
LINEARREFERENCING_TYPEDEFS(LinearlyReferencedLocation)
LINEARREFERENCING_TYPEDEFS(LinearlyReferencedAtLocation)
LINEARREFERENCING_TYPEDEFS(LinearlyReferencedFromToLocation)
LINEARREFERENCING_TYPEDEFS(LinearLocation)

LINEARREFERENCING_REFCOUNTED_PTR(GeometricElementAsReferent)
LINEARREFERENCING_REFCOUNTED_PTR(LinearlyReferencedAtLocation)
LINEARREFERENCING_REFCOUNTED_PTR(LinearlyReferencedFromToLocation)
