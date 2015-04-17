/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/AssocGeom.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#define BAD_ASSOCIATION         20
#define ASSOC_TOPO_CHANGE       21

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/* association types */
enum AssocPointType
    {
    LINEAR_ASSOC        = 1,
    INTERSECT_ASSOC     = 2,
    ARC_ASSOC           = 3,
    MLINE_ASSOC         = 4,
    BCURVE_ASSOC        = 5,
    PROJECTION_ASSOC    = 6,
    ORIGIN_ASSOC        = 7,
    INTERSECT2_ASSOC    = 8,
    MESH_VERTEX_ASSOC   = 10,
    MESH_EDGE_ASSOC     = 11,
    BSURF_ASSOC         = 13,
    CUSTOM_ASSOC        = 14,
    BLOB_ASSOC          = 15,   // internal -- used only by PersistentSnapPath

    };

#define UNASSOC_DIM_WEIGHT      3
#define UNASSOC_DIM_STYLE       2

typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    unsigned short vertex;
    unsigned short numerator;
    unsigned short divisor;
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId;           // id of element to compute assoc point from
    uint64_t ___legacyref;
    unsigned short nVertex;            // number of vertices on linear element
    unsigned short reserved[7];        // padding
    } LinearAssoc;

typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    unsigned short vertex;
    unsigned short nVertex;            // number of vertices on linear element
    unsigned short reserved;           // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId;           // id of element to compute assoc point from
    uint64_t ___legacyref;
    double      ratioVal;
    unsigned short reserved2[4];       // padding
    } ProjectionAssoc;

typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    unsigned short keyPoint;
    unsigned short reserved[2];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId;           // id of element to compute assoc point from
    uint64_t ___legacyref;
    double      angle;
    unsigned short reserved2[4];       // padding
    } ArcAssoc;

typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    unsigned short option;             // Used for text/text node to override user origin
    unsigned short reserved[2];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId;           // id of element to compute assoc point from
    uint64_t ___legacyref;
    unsigned short reserved2[8];       // padding
    } OriginAssoc;

typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    unsigned short reserved[3];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId;           // id of element to compute assoc point from
    uint64_t ___legacyref;
    double      uParam;
    unsigned short reserved2[4];       // padding
    } BCurveAssoc;

typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    unsigned short reserved[3];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId;           // id of element to compute assoc point from
    uint64_t ___legacyref;
    double      uParam;
    double      vParam;
    } BSurfAssoc;

typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    unsigned short pointNo;
    struct
        {
        unsigned short lineNo:8;
        unsigned short joint:1;
        unsigned short project:1;
        unsigned short reserved:6;
        } b;
    unsigned short reserved;           // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId;           // id of element to compute assoc point from
    uint64_t ___legacyref;
    double      offsetVal;
    unsigned short nVertex;            // number of vertices on linear element
    unsigned short reserved2[3];       // padding
    } MlineAssoc;

typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    unsigned short reserved;           // padding
    uint32_t    vertexIndex;
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId;           // id of element to compute assoc point from
    uint64_t ___legacyref;
    double      reservedValue;
    uint32_t    nVertex;
    unsigned short reserved2[2];       // padding
    } MeshVertexAssoc;

typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    unsigned short reserved;           // padding
    uint32_t    edgeIndex;
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId;           // id of element to compute assoc point from
    uint64_t ___legacyref;
    double      uParam;
    uint32_t    nEdge;
    unsigned short reserved2[2];       // padding
    } MeshEdgeAssoc;

typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    Byte index;
    Byte padByte2;
    unsigned short reserved[2];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId1;          // id of first element to compute assoc point from
    uint64_t uniqueId2;          // id of second element to compute assoc point from
    uint64_t ___legacyref1;
    uint64_t ___legacyref2;
    } IntersectAssoc;

typedef struct
    {
    Byte type;               // Must be first
    Byte nSeg1;              // number of segments in first element
    Byte index;
    Byte nSeg2;              // number of segments in second element
    unsigned short seg1;               // segment on first element
    unsigned short seg2;               // segment on second element
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId1;          // id of first element to compute assoc point from
    uint64_t uniqueId2;          // id of second element to compute assoc point from
    uint64_t ___legacyref1;
    uint64_t ___legacyref2;
    } Intersect2Assoc;

/*  Refers to an ICustomKeypoint, which is stored on a far path element.
    This is set up just like any other single-point assoc that uses a path instead of a reference attachment:
    -- The first element ID refers to the target elemetn (relative to the target model).
    -- The second element ID refers to the far path element (in the dependent's model), which stands
        in for the reference attachment ID.
    The only unusual feature of a CustomKeypointAssoc is that the pathElementId will *always* be
    defined, even if target element is in the local file or a directly attached reference.
*/
typedef struct
    {
    Byte type;               // Must be first
    Byte subType;            // 0: call the handler, 1:
    unsigned short reserved[3];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t targetElementId;    // id of target element in target model
    uint64_t pathElementId;
    unsigned short reserved2[8];       // padding
    } CustomKeypointAssoc;

/* Just a common structure for accessing uniqueIds of single element association types */
typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    unsigned short reserved[3];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId;           // id of element to compute assoc point from
    uint64_t ___legacyref;
    unsigned short reserved2[8];       // padding
    } SingleElmAssoc;

/* Just a common structure for accessing uniqueIds of two element association types */
typedef struct
    {
    Byte type;               // Must be first
    Byte padByte;
    unsigned short reserved[3];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    uint64_t uniqueId1;          // id of first element to compute assoc point from
    uint64_t uniqueId2;          // id of second element to compute assoc point from
    uint64_t ___legacyref1;
    uint64_t ___legacyref2;
    } TwoElmAssoc;

union AssocGeom
    {
    Byte type;       // Must be first
    SingleElmAssoc      singleElm;
    TwoElmAssoc         twoElm;
    LinearAssoc         line;
    ProjectionAssoc     projection;
    BCurveAssoc         bCurve;
    BSurfAssoc          bSurf;
    ArcAssoc            arc;
    OriginAssoc         origin;
    MeshVertexAssoc     meshVertex;
    MeshEdgeAssoc       meshEdge;
    MlineAssoc          mline;
    CustomKeypointAssoc customKeypoint;
    IntersectAssoc      intersect;
    Intersect2Assoc     intersect2;
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
