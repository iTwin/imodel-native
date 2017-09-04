/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Domain/PublicApi/GridsMacros.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

#define GRIDS_NAMESPACE_NAME  Grids
#define BEGIN_GRIDS_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace GRIDS_NAMESPACE_NAME {
#define END_GRIDS_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_GRIDS using namespace BENTLEY_NAMESPACE_NAME::GRIDS_NAMESPACE_NAME;

#define DECLARE_GRIDS_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(GRIDS_SCHEMA_NAME, GRIDS_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(GRIDS_SCHEMA_NAME, GRIDS_CLASS_##__name__)); }

#define DECLARE_GRIDS_ELEMENT_BASE_METHODS(__name__, __exportstr__) \
    DECLARE_GRIDS_QUERYCLASS_METHODS(__name__) \
    __exportstr__ static __name__##CPtr Get       (Dgn::DgnDbR, Dgn::DgnElementId); \
    __exportstr__ static __name__##Ptr  GetForEdit(Dgn::DgnDbR, Dgn::DgnElementId); \
    __exportstr__        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr); \
    __exportstr__        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr);  

#if defined (__GRIDS_BUILD__)
#define GRIDS_EXPORT EXPORT_ATTRIBUTE
#else
#define GRIDS_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__GRIDELEMENTS_BUILD__)
#define GRIDELEMENTS_EXPORT EXPORT_ATTRIBUTE
#else
#define GRIDELEMENTS_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__GRIDHANDLERS_BUILD__)
#define GRIDHANDLERS_EXPORT EXPORT_ATTRIBUTE
#else
#define GRIDHANDLERS_EXPORT IMPORT_ATTRIBUTE
#endif

#if defined (__GRIDSDOMAIN_BUILD__)
#define GRIDSDOMAIN_EXPORT EXPORT_ATTRIBUTE
#else
#define GRIDSDOMAIN_EXPORT IMPORT_ATTRIBUTE
#endif

#define DEFINE_GRIDS_ELEMENT_BASE_METHODS(__name__) \
    __name__##CPtr __name__::Get       (DgnDbR db, DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    __name__##Ptr  __name__::GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); } \
    __name__##CPtr __name__::Insert    (DgnDbStatus* stat)         { return GetDgnDb().Elements().Insert< __name__ >(*this, stat);} \
    __name__##CPtr __name__::Update    (DgnDbStatus* stat)         { return GetDgnDb().Elements().Update< __name__ >(*this, stat);}

#define GRIDS_SCHEMA_NAME                                     "Grids"
#define GRIDS_SCHEMA_PATH                                     L"ECSchemas/Domain/Grids.ecschema.xml"

#define GRIDS_SCHEMA(className)                               GRIDS_SCHEMA_NAME "." className
#define GRIDS_SCHEMA_CODE(categoryName)                       GRIDS_SCHEMA_NAME "_" categoryName
#define GRIDS_CODESPEC_CODE(categoryName)                     GRIDS_SCHEMA_NAME "::" categoryName

#define GRIDS_REL_GridSurfaceCreatesGridCurve                   "GridSurfaceCreatesGridCurve"

#define GRIDS_CLASS_SurfaceSet                                  "SurfaceSet"
#define GRIDS_CLASS_GridPortion                                 "GridPortion"
#define GRIDS_CLASS_OrthogonalGridPortion                       "OrthogonalGridPortion"
#define GRIDS_CLASS_RadialGridPortion                           "RadialGridPortion"
#define GRIDS_CLASS_SketchGridPortion                           "SketchGridPortion"

#define GRIDS_CLASS_IntersectionCurve                           "IntersectionCurve"
#define GRIDS_CLASS_GridCurve                                   "GridCurve"
#define GRIDS_CLASS_GridLine                                    "GridLine"
#define GRIDS_CLASS_GridSpline                                  "GridSpline"
#define GRIDS_CLASS_GridArc                                     "GridArc"

#define GRIDS_CLASS_DrivingSurface                              "DrivingSurface"
#define GRIDS_CLASS_GridSurface                                 "GridSurface"
#define GRIDS_CLASS_GridPlaneSurface                            "GridPlaneSurface"
#define GRIDS_CLASS_GridArcSurface                              "GridArcSurface"

//Categories
#define GRIDS_CATEGORY_CODE_IntersectionCurve                   "IntersectionCurve"
#define GRIDS_CATEGORY_CODE_GridCurve                           "GridCurve"

#define GRIDS_CATEGORY_CODE_DrivingSurface                      "DrivingSurface"
#define GRIDS_CATEGORY_CODE_GridSurface                         "GridSurface"

//Authorities
#define GRIDS_AUTHORITY_GridCurve                       GRIDS_CODESPEC_CODE(GRIDS_CLASS_GridCurve)
#define GRIDS_AUTHORITY_SurfaceSet                      GRIDS_CODESPEC_CODE(GRIDS_CLASS_SurfaceSet)
#define GRIDS_AUTHORITY_GridPortion                     GRIDS_CODESPEC_CODE(GRIDS_CLASS_GridPortion)
#define GRIDS_AUTHORITY_OrthogonalGridPortion           GRIDS_CODESPEC_CODE(GRIDS_CLASS_OrthogonalGridPortion)
#define GRIDS_AUTHORITY_RadialGridPortion               GRIDS_CODESPEC_CODE(GRIDS_CLASS_RadialGridPortion)
#define GRIDS_AUTHORITY_SketchGridPortion               GRIDS_CODESPEC_CODE(GRIDS_CLASS_SketchGridPortion)

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_GRIDS_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_PTR(_name_) \
    END_GRIDS_NAMESPACE

    
    