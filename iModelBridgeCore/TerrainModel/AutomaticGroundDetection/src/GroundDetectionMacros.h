/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/GroundDetectionMacros.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma  once
#define BEGIN_GROUND_DETECTION_NAMESPACE                BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE namespace GroundDetection {
#define END_GROUND_DETECTION_NAMESPACE                  END_BENTLEY_TERRAINMODEL_NAMESPACE }
#define USING_NAMESPACE_GROUND_DETECTION                using namespace BENTLEY_NAMESPACE_NAME::TerrainModel::GroundDetection;

#define GROUND_DETECTION_TYPEDEF(_sname_) \
    BEGIN_GROUND_DETECTION_NAMESPACE \
        struct _sname_; \
        typedef RefCountedPtr<_sname_> _sname_##Ptr; \
    END_GROUND_DETECTION_NAMESPACE

#if defined (__GROUND_DETECTION_BUILD__)
#    define GROUND_DETECTION_EXPORT           EXPORT_ATTRIBUTE
#else
#    define GROUND_DETECTION_EXPORT           IMPORT_ATTRIBUTE
#endif

//GDZERO
#define EPS 1.0e-9
#define DOUBLE_EQUAL(v1,v2)  ((v1 <= (v2+EPS)) && (v1 >= (v2-EPS)))
#define DOUBLE_EQUAL_TOL(v1,v2,tol)  ((v1 <= (v2+tol)) && (v1 >= (v2-tol)))
   