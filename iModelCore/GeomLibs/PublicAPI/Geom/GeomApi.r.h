/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <Bentley/Bentley.r.h>
#include <Bentley/Bentley.h>

#if defined (mdl_resource_compiler) || defined (mdl_type_resource_generator)

struct DPoint2d  {T_Adouble x,y;};
struct DPoint3d  {T_Adouble x,y,z;};
struct RotMatrix {T_Adouble form3d[3][3];};
struct Transform {T_Adouble form3d[3][4];};
struct DRange3d  {DPoint3d low, high;};

struct BsplineParam
    {
    int32_t             order;
    int32_t             closed;
    int32_t             numPoles;
    int32_t             numKnots;
    int32_t             numRules;
    };

struct BsplineDisplay
    {
    int32_t             polygonDisplay;
    int32_t             curveDisplay;
    int32_t             rulesByLength;
    };

struct MSBsplineCurve
    {
    // Classic microstation bspline curve data members .....
    int32_t             type;
    int32_t             rational;
    BsplineDisplay      display;
    BsplineParam        params;
    DPoint3d            *poles;         /* In homogeneous coordinates (weighted) */
    T_Adouble              *knots;         /* Full knot vector */
    T_Adouble              *weights;       /* Weights only if rational */
    };

#endif // defined (mdl_resource_compiler) || defined (mdl_type_resource_generator)

#include "msgeomstructs_typedefs.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! {nodeId, entityId} pair for solid topology references.
typedef struct faceId
    {
    uint32_t nodeId;
    uint32_t entityId;

#ifdef __cplusplus
       bool operator <  (struct faceId const& rhs) const { return (rhs.nodeId == nodeId) ? (entityId < rhs.entityId)  : (nodeId < rhs.nodeId); }
#endif
    } FaceId;

//! face pair for solid topology references.
typedef struct edgeId
    {
    FaceId      faces[2];
    } EdgeId;

//! vertex triple for solid topology references.
typedef struct vertexId
    {
    FaceId      faces[3];
    } VertexId;

//! Red, green, blue color data as floats.
typedef struct FloatRgb {float red, green, blue;} FloatRgb;

//! Red, green, blue color data as doubles.
struct RgbFactor
{
T_Adouble red, green, blue;
#ifdef __cplusplus
//! Extract bytes from an integer color into double colors.
static GEOMDLLIMPEXP RgbFactor FromIntColor (int32_t intColor);
//! Copy from point (or DVec3d!!!)
static GEOMDLLIMPEXP RgbFactor From (DPoint3dCR data);
static GEOMDLLIMPEXP RgbFactor From (double r, double g, double b);
void GEOMDLLIMPEXP AddInPlace (RgbFactor const &other);
void GEOMDLLIMPEXP ScaleInPlace (double a);
//! Exact equality test -- beware of last bit, EqualsInt is probably mor appropriate
bool GEOMDLLIMPEXP Equals (RgbFactor const &other) const;
//! Return true if the int equivalents are equal.
bool GEOMDLLIMPEXP EqualsInt (RgbFactor const &other) const;
//! Pack double colors into int bytes.
GEOMDLLIMPEXP int32_t ToIntColor () const;
#endif
};

END_BENTLEY_GEOMETRY_NAMESPACE
