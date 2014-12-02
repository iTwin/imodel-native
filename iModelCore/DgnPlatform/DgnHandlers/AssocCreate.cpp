/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/AssocCreate.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssociativePoint::SetRoot
(
AssocPoint&     assocPoint,
ElementId       elemId,
int             rootIndex
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    switch (assoc.type)
        {
        case LINEAR_ASSOC:
        case PROJECTION_ASSOC:
        case ARC_ASSOC:
        case MLINE_ASSOC:
        case ORIGIN_ASSOC:
        case BSURF_ASSOC:
        case BCURVE_ASSOC:
        case MESH_VERTEX_ASSOC:
        case MESH_EDGE_ASSOC:
        case CUSTOM_ASSOC:
            {
            if (0 != rootIndex)
                return ERROR;

            assoc.singleElm.uniqueId        = elemId.GetValue();
            assoc.singleElm.___legacyref = 0;

            return SUCCESS;
            }

        case INTERSECT_ASSOC:
        case INTERSECT2_ASSOC:
            {
            switch (rootIndex)
                {
                case 0:
                    assoc.twoElm.uniqueId1        = elemId.GetValue();
                    assoc.twoElm.___legacyref1 = 0;
                    break;

                case 1:
                    assoc.twoElm.uniqueId2        = elemId.GetValue();
                    assoc.twoElm.___legacyref2 = 0;
                    break;

                default:
                    return ERROR;
                }

            return SUCCESS;
            }

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssociativePoint::SetRoot
(
AssocPoint&     assocPoint,
DisplayPathCP   path,
DgnModelP    parentModel_unused,
bool            allowFarElm_unused,
int             rootIndex
)
    {
    ElementId   elemId;

//    if (!path || SUCCESS != assoc_createFarReference (NULL, &elemId, &refAttId, path, allowFarElm, NULL, NULL, parentModel))
    int         currIndex = path->GetCursorIndex ();
    ElementRefP cursorElm = path->GetPathElem (currIndex);
    if (NULL == cursorElm)
        return ERROR;

    elemId = cursorElm->GetElementId();

    return AssociativePoint::SetRoot (assocPoint, elemId, rootIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssociativePoint::IsValid
(
AssocPoint&     assocPoint,
DgnModelP    pathRoot,
DgnModelP    parentModel_unused
)
    {
    return AssociativePoint::GetPoint (NULL, assocPoint, pathRoot);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssociativePoint::InitKeypoint
(
AssocPoint&     assocPoint,
UShort          vertex,    
UShort          nVertex,   
UShort          numerator, 
UShort          divisor
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    memset (&assoc, 0, sizeof (assoc));

    assoc.line.type         = LINEAR_ASSOC;
    assoc.line.vertex       = vertex;
    assoc.line.numerator    = numerator;
    assoc.line.divisor      = divisor;
    assoc.line.nVertex      = nVertex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssociativePoint::InitOrigin
(
AssocPoint&     assocPoint,
UShort          option
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    memset (&assoc, 0, sizeof (assoc));

    assoc.origin.type = ORIGIN_ASSOC;

    /*-------------------------------------------------------------------
    Currently the option parameter is used only for text and text nodes
    to make it possible to associate to any of the justification points
    rather than associating to the user origin.

    Text control points are numbered as follows:    2 - 6 - 3
                                                    5 - 9 - 7
                                                    1 - 8 - 4
    -------------------------------------------------------------------*/
    assoc.origin.option = option;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssociativePoint::InitArc
(
AssocPoint&     assocPoint,
ArcLocation     keyPoint,
double          angle
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    memset (&assoc, 0, sizeof (assoc));

    assoc.arc.type      = ARC_ASSOC;
    assoc.arc.keyPoint  = keyPoint;

    if (ARC_ANGLE == keyPoint)
        {
        angle = fmod (angle, msGeomConst_2pi);

        if (angle < 0.0)
            angle += msGeomConst_2pi;

        assoc.arc.angle = angle;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssociativePoint::InitProjection
(
AssocPoint&     assocPoint,
UShort          vertex,     
UShort          nVertex,    
double          ratio
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    memset (&assoc, 0, sizeof (assoc));

    assoc.projection.type       = PROJECTION_ASSOC;
    assoc.projection.vertex     = vertex;
    assoc.projection.ratioVal   = ratio;
    assoc.projection.nVertex    = nVertex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssociativePoint::InitMline
(
AssocPoint&     assocPoint,
UShort          vertex,    
UShort          nVertex,   
UShort          lineNo,    
double          offset,    
bool            joint
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    memset (&assoc, 0, sizeof (assoc));

    assoc.mline.type        = MLINE_ASSOC;
    assoc.mline.pointNo     = vertex;
    assoc.mline.nVertex     = nVertex;
    assoc.mline.b.lineNo    = lineNo;

    if (joint)
        {
        assoc.mline.b.joint = true;
        }
    else
        {
        assoc.mline.b.project = true;
        assoc.mline.offsetVal = offset;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssociativePoint::InitBCurve
(
AssocPoint&     assocPoint,
double          uParam
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    memset (&assoc, 0, sizeof (assoc));

    assoc.bCurve.type   = BCURVE_ASSOC;
    assoc.bCurve.uParam = uParam;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssociativePoint::InitBSurface
(
AssocPoint&     assocPoint,
double          uParam,     
double          vParam     
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    memset (&assoc, 0, sizeof (assoc));

    assoc.bSurf.type    = BSURF_ASSOC;
    assoc.bSurf.uParam  = uParam;
    assoc.bSurf.vParam  = vParam;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssociativePoint::InitMeshEdge
(
AssocPoint&     assocPoint,
int             edgeIndex, 
int             nEdge,     
double          uParam
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    memset (&assoc, 0, sizeof (assoc));

    assoc.meshEdge.type         = MESH_EDGE_ASSOC;
    assoc.meshEdge.uParam       = uParam;
    assoc.meshEdge.edgeIndex    = edgeIndex;
    assoc.meshEdge.nEdge        = nEdge;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssociativePoint::InitMeshVertex
(
AssocPoint&     assocPoint,
int             vertexIndex,
int             nVertex    
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    memset (&assoc, 0, sizeof (assoc));

    assoc.meshVertex.type           = MESH_VERTEX_ASSOC;
    assoc.meshVertex.vertexIndex    = vertexIndex;
    assoc.meshVertex.nVertex        = nVertex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssociativePoint::InitIntersection
(
AssocPoint&     assocPoint,
byte            index,      
UShort          seg1,       
UShort          seg2,       
int             nSeg1,      
int             nSeg2
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    memset (&assoc, 0, sizeof (assoc));

    assoc.intersect2.type       = INTERSECT2_ASSOC;
    assoc.intersect2.index      = index;
    assoc.intersect2.seg1       = seg1;
    assoc.intersect2.seg2       = seg2;

    if (nSeg1 <= 255)
        assoc.intersect2.nSeg1  = nSeg1 & 0xff;

    if (nSeg2 <= 255)
        assoc.intersect2.nSeg2  = nSeg2 & 0xff;
    }

