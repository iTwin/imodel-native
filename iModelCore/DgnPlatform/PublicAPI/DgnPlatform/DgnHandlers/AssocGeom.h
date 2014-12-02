/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/AssocGeom.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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
    byte        type;               // Must be first
    byte        padByte;
    UShort      vertex;
    UShort      numerator;
    UShort      divisor;
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId;           // id of element to compute assoc point from
    UInt64   ___legacyref;
    UShort      nVertex;            // number of vertices on linear element
    UShort      reserved[7];        // padding
    } LinearAssoc;

typedef struct
    {
    byte        type;               // Must be first
    byte        padByte;
    UShort      vertex;
    UShort      nVertex;            // number of vertices on linear element
    UShort      reserved;           // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId;           // id of element to compute assoc point from
    UInt64   ___legacyref;
    double      ratioVal;
    UShort      reserved2[4];       // padding
    } ProjectionAssoc;

typedef struct
    {
    byte        type;               // Must be first
    byte        padByte;
    UShort      keyPoint;
    UShort      reserved[2];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId;           // id of element to compute assoc point from
    UInt64   ___legacyref;
    double      angle;
    UShort      reserved2[4];       // padding
    } ArcAssoc;

typedef struct
    {
    byte        type;               // Must be first
    byte        padByte;
    UShort      option;             // Used for text/text node to override user origin
    UShort      reserved[2];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId;           // id of element to compute assoc point from
    UInt64   ___legacyref;
    UShort      reserved2[8];       // padding
    } OriginAssoc;

typedef struct
    {
    byte        type;               // Must be first
    byte        padByte;
    UShort      reserved[3];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId;           // id of element to compute assoc point from
    UInt64   ___legacyref;
    double      uParam;
    UShort      reserved2[4];       // padding
    } BCurveAssoc;

typedef struct
    {
    byte        type;               // Must be first
    byte        padByte;
    UShort      reserved[3];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId;           // id of element to compute assoc point from
    UInt64   ___legacyref;
    double      uParam;
    double      vParam;
    } BSurfAssoc;

typedef struct
    {
    byte        type;               // Must be first
    byte        padByte;
    UShort      pointNo;
    struct
        {
        UShort  lineNo:8;
        UShort  joint:1;
        UShort  project:1;
        UShort  reserved:6;
        } b;
    UShort      reserved;           // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId;           // id of element to compute assoc point from
    UInt64   ___legacyref;
    double      offsetVal;
    UShort      nVertex;            // number of vertices on linear element
    UShort      reserved2[3];       // padding
    } MlineAssoc;

typedef struct
    {
    byte        type;               // Must be first
    byte        padByte;
    UShort      reserved;           // padding
    UInt32      vertexIndex;
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId;           // id of element to compute assoc point from
    UInt64   ___legacyref;
    double      reservedValue;
    UInt32      nVertex;
    UShort      reserved2[2];       // padding
    } MeshVertexAssoc;

typedef struct
    {
    byte        type;               // Must be first
    byte        padByte;
    UShort      reserved;           // padding
    UInt32      edgeIndex;
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId;           // id of element to compute assoc point from
    UInt64   ___legacyref;
    double      uParam;
    UInt32      nEdge;
    UShort      reserved2[2];       // padding
    } MeshEdgeAssoc;

typedef struct
    {
    byte        type;               // Must be first
    byte        padByte;
    byte        index;
    byte        padByte2;
    UShort      reserved[2];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId1;          // id of first element to compute assoc point from
    UInt64   uniqueId2;          // id of second element to compute assoc point from
    UInt64   ___legacyref1;
    UInt64   ___legacyref2;
    } IntersectAssoc;

typedef struct
    {
    byte        type;               // Must be first
    byte        nSeg1;              // number of segments in first element
    byte        index;
    byte        nSeg2;              // number of segments in second element
    UShort      seg1;               // segment on first element
    UShort      seg2;               // segment on second element
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId1;          // id of first element to compute assoc point from
    UInt64   uniqueId2;          // id of second element to compute assoc point from
    UInt64   ___legacyref1;
    UInt64   ___legacyref2;
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
    byte        type;               // Must be first
    byte        subType;            // 0: call the handler, 1:
    UShort      reserved[3];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   targetElementId;    // id of target element in target model
    UInt64   pathElementId;
    UShort      reserved2[8];       // padding
    } CustomKeypointAssoc;

/* Just a common structure for accessing uniqueIds of single element association types */
typedef struct
    {
    byte        type;               // Must be first
    byte        padByte;
    UShort      reserved[3];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId;           // id of element to compute assoc point from
    UInt64   ___legacyref;
    UShort      reserved2[8];       // padding
    } SingleElmAssoc;

/* Just a common structure for accessing uniqueIds of two element association types */
typedef struct
    {
    byte        type;               // Must be first
    byte        padByte;
    UShort      reserved[3];        // padding
    // *** WIP_V10_ASSOC_POINT - switch to ECRelationship?
    UInt64   uniqueId1;          // id of first element to compute assoc point from
    UInt64   uniqueId2;          // id of second element to compute assoc point from
    UInt64   ___legacyref1;
    UInt64   ___legacyref2;
    } TwoElmAssoc;

union AssocGeom
    {
    byte                type;       // Must be first
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
