/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct Dgn::TransformClip
{
Transform                       m_transform;                // The "top of stack" transform - product of all transforms in stack.
Transform                       m_inverse;                  // Inverse of m_transform.
bool                            m_transformed;              // True if m_transform is not identity.
size_t                          m_drawGeomPopCount;         // Number of calls to make to IDrawGeom::PopTransClip
bool                            m_isViewIndependent;        // True if view independent.
ClipVectorPtr                   m_allClips;                 // Clips for all stack entries.
ClipVectorPtr                   m_thisClip;                 // Clips for this stack entry.
ClipVectorPtr                   m_allDrawGeomClips;         // All clips pushed to drawGeom.

ClipVectorCP    GetClip () const            { return m_allClips.get(); }
ClipVectorCP    GetDrawGeomClip () const    { return m_allDrawGeomClips.get(); }
TransformCP     GetTransformCP() const      { return m_transformed ?  &m_transform : NULL; }

~TransformClip () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TransformClip (TransformClip* top, TransformCP transform, ClipVectorCP clip)
    {
    Init (top);

    if (NULL != clip)
        {
        m_thisClip = ClipVector::CreateCopy (*clip); 

        if (m_allClips.IsValid())
            m_allClips->AppendCopy (*clip);
        else
            m_allClips = ClipVector::CreateCopy (*clip);
        }

    TransformInPlace (transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GetTransform (TransformR transform)
    {
    transform = m_transform;
    return m_transformed ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double  GetTransformScale ()
    {
    if (! m_transformed)
        return 1.0;

    double  determinant =  RotMatrix::From (m_transform).Determinant();

    return pow (fabs (determinant), 1.0 / 3.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GetInverse (TransformR inverse)
    {
    inverse = m_inverse;
    return m_transformed ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void  Init (TransformClip* top)
    {
    m_drawGeomPopCount = 0;

    if (NULL != top)
        {
        m_transform         = top->m_transform;
        m_inverse           = top->m_inverse;
        m_transformed       = top->m_transformed;
        m_isViewIndependent = top->m_isViewIndependent;

        if (top->m_allClips.IsValid())
            m_allClips = ClipVector::CreateCopy (*top->m_allClips);

        if (top->m_allDrawGeomClips.IsValid())
            m_allDrawGeomClips = ClipVector::CreateCopy (*top->m_allDrawGeomClips);
        }                   
    else
        {
        m_transform.InitIdentity ();
        m_inverse = m_transform;
        m_isViewIndependent = false;
        m_transformed = false;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    TransformInPlace (TransformCP transform)
    {
    if (NULL != transform && !transform->IsIdentity())
        {
        m_transformed = true;
        m_transform = Transform::FromProduct (m_transform, *transform);
        
        Transform       inverse;

        if (!inverse.InverseOf (*transform))
            {
            BeDataAssert (false);
            return;
            }

        m_inverse = Transform::FromProduct (inverse, m_inverse);

        if (m_allClips.IsValid())
            m_allClips->TransformInPlace (inverse);

        if (m_thisClip.IsValid())
            m_thisClip->TransformInPlace (inverse);
            
        if (m_allDrawGeomClips.IsValid())
            m_allDrawGeomClips->TransformInPlace (inverse);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IncrementPushedToDrawGeom ()
    {
    if (0 == m_drawGeomPopCount++ && m_thisClip.IsValid())
        {
        if (m_allDrawGeomClips.IsValid())
            m_allDrawGeomClips->AppendCopy (*m_thisClip);
        else
            m_allDrawGeomClips = ClipVector::CreateCopy (*m_thisClip);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+-------- ------+---------------+---------------+------*/
bool    TestRayIntersect (DPoint3dCR point, DVec3dCR direction) const
    {
    if (m_allClips.IsValid())
        {
        for (ClipPrimitivePtr const& primitive: *m_allClips)
            {
            ClipPlaneSetCP  planeSet = primitive->GetClipPlanes();

            if (NULL != planeSet &&
                !planeSet->IsPointInside (point) &&
                !planeSet->TestRayIntersect (point, direction))
                return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+--------------+---------------+---------------+------*/
ClipPlaneContainment    ClassifyPoints (DPoint3dCP points, size_t nPoints, bool ignoreMasks) const
    {
    bool            allStronglyInside = true;

    if (m_allClips.IsValid())
        {
        for (ClipPrimitivePtr const& primitive: *m_allClips)
            {
            ClipPlaneContainment    thisStatus = primitive->ClassifyPointContainment (points, nPoints, ignoreMasks);

            if (ClipPlaneContainment_StronglyOutside == thisStatus)
                return thisStatus;

            if (ClipPlaneContainment_StronglyInside != thisStatus)
                allStronglyInside = false;
                
            }
        }
    
    return allStronglyInside ? ClipPlaneContainment_StronglyInside : ClipPlaneContainment_Ambiguous;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+--------------+---------------+---------------+------*/
bool    TestPoint (DPoint3dCR point) const 
    {
    if (m_allClips.IsValid())
        {
        for (ClipPrimitivePtr const& primitive: *m_allClips)
            {
            if (ClipPlaneContainment_StronglyOutside == primitive->ClassifyPointContainment (&point, 1, false))
                return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+--------------+---------------+---------------+------*/
void Pop (ViewContextR context)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    for (size_t i=0; i < m_drawGeomPopCount; i++)
        context.GetCurrentGraphicR().PopTransClip();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GetRayIntersection (double& maxIntersect, DPoint3dCR point, DVec3dCR direction) const
    {
    maxIntersect = -fc_hugeVal;

    if (m_allClips.IsValid())
        {
        for (ClipPrimitivePtr const& primitive: *m_allClips)
            {
            double                  tNear;
            ClipPlaneSetCP          planeSet = primitive->GetClipPlanes();

            if (NULL != planeSet &&
                planeSet->GetRayIntersection (tNear, point, direction))
                maxIntersect = MAX (tNear, maxIntersect);
            else
                return false;
            }
        }

    return maxIntersect > -fc_hugeVal;
    }

}; // TransformClip

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TransformClipStack::IsEmpty () const
    {
    return m_transformClips.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    TransformClipStack::PushTransform (TransformCR trans)
    {
    m_transformClips.push_back (new TransformClip (GetTop(), &trans, NULL));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    TransformClipStack::PushIdentity ()
    {
    m_transformClips.push_back (new TransformClip (NULL, NULL, NULL));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    TransformClipStack::PushClip (ClipVectorCR clip)
    {
    m_transformClips.push_back (new TransformClip (GetTop(), NULL, &clip));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    TransformClipStack::PushClipPlaneSets (ClipPlaneSetCR planeSet)
    {
    PushClip (*new ClipVector(ClipPrimitive::CreateFromClipPlanes(planeSet).get()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    TransformClipStack::PushClipPlanes (ClipPlaneCP planes, size_t nPlanes)
    {
    PushClipPlaneSets (ClipPlaneSet (planes, nPlanes));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    TransformClipStack::TestRay (DPoint3dCR point, DVec3dCR direction) const
    {
    if (TestPoint (point))
        return true;

    return m_transformClips.back()->TestRayIntersect (point, direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment TransformClipStack::ClassifyPoints (DPoint3dCP points, size_t nPoints, bool ignoreMasks) const
    {
    return IsEmpty() ? ClipPlaneContainment_StronglyInside : m_transformClips.back()->ClassifyPoints (points, nPoints, ignoreMasks); 
    } 

/*---------------------------------------------------------------------------------**//**
*   Specialized routine. Does not set the distance unless it is greater than intial distance. 
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    TransformClipStack::GetRayIntersection (double& intersectDistance, DPoint3dCR point, DVec3dCR direction) const
    {
    return IsEmpty() ? false : m_transformClips.back()->GetRayIntersection (intersectDistance, point, direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    TransformClipStack::TestPoint (DPoint3dCR point) const 
    {
    return IsEmpty () ? true : m_transformClips.back()->TestPoint (point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    TransformClipStack::GetTransform (TransformR transform) const
    {
    if (m_transformClips.empty())
        {
        transform.InitIdentity ();
        return ERROR;
        }

    return m_transformClips.back()->GetTransform(transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    TransformClipStack::GetInverseTransform (TransformR inverse) const
    {
    if (m_transformClips.empty())
        {
        inverse.InitIdentity ();
        return ERROR;
        }

    return m_transformClips.back()->GetInverse (inverse);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    TransformClipStack::GetTransformFromIndex (TransformR transform, size_t index) const
    {
    if (index >= m_transformClips.size ())
        {
        transform.InitIdentity ();
        return ERROR;
        }

    transform = m_transformClips.at (index)->m_transform;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    TransformClipStack::GetTransformFromTopToIndex (TransformR transform, size_t index) const
    {                               
    transform.InitIdentity();

    if (SUCCESS != GetTransform (transform))
        return ERROR;

    if (0 == index)
        return SUCCESS;

    if (index > m_transformClips.size () ||
        m_transformClips.at (index-1)->m_transform.IsEqual (transform))
        {
        transform.InitIdentity ();
        return ERROR;
        }
    transform = Transform::FromProduct (m_transformClips.at (index-1)->m_inverse, transform);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransformClipStack::SetViewIndependent () 
    {
    if (m_transformClips.empty())
        {
        BeAssert (false);
        return;
        }
    m_transformClips.back()->m_isViewIndependent = true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransformClipStack::IncrementPushedToDrawGeom ()
    {
    if (m_transformClips.empty())
        {
        BeAssert (false);
        return;
        }
    m_transformClips.back()->IncrementPushedToDrawGeom();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransformClipStack::Pop (ViewContextR viewContext)
    {
    if (m_transformClips.empty())
        {
        BeAssert (false);
        return;
        }
    m_transformClips.back()->Pop (viewContext);
    delete m_transformClips.back();
    m_transformClips.pop_back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransformClipStack::PopAll (ViewContextR viewContext)
    {
    while (!IsEmpty())
        Pop (viewContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef COMMENT_OUT //*** graphite does not use clip "elements"
void            TransformClipStack::AddClipElement(ElementHandleCR eh, bool setVisited)
    {
    if (!m_transformClips.empty())
        m_transformClips.back()->AddClipElement (eh, setVisited);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            TransformClipStack::DeferVisit (DgnElementP element) 
    {
    return IsEmpty() ? false : m_transformClips.back()->DeferVisit (element);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void        TransformClipStack::Clear ()           
    { 
    for (TransformClip* transformClip: m_transformClips)
        delete transformClip;

    m_transformClips.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TransformClipStack::~TransformClipStack ()
    {
    for (TransformClip* transformClip: m_transformClips)
        delete transformClip;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+--------------+---------------+---------------+------*/
ClipPlaneContainment    TransformClipStack::ClassifyRange (DRange3dCR range, bool ignoreMasks) const  
    {
    DPoint3d        corners[8];

    range.Get8Corners (corners);
    return ClassifyPoints (corners, 8, ignoreMasks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+--------------+---------------+---------------+------*/
ClipPlaneContainment    TransformClipStack::ClassifyElementRange(DRange3dCR elRange, bool is3d, bool ignoreMasks) const  
    {
    DRange3d range = elRange;
    if (!is3d)
        range.low.z = range.high.z = 0.0;
    
    return ClassifyRange (range, ignoreMasks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          TransformClipStack::GetSize () const                { return m_transformClips.size(); }
bool            TransformClipStack::IsViewIndependent () const      { return m_transformClips.empty() ? false : m_transformClips.back()->m_isViewIndependent; }
TransformCP     TransformClipStack::GetTransformCP() const          { return m_transformClips.empty() ? NULL  : m_transformClips.back()->GetTransformCP(); }
double          TransformClipStack::GetTransformScale() const       { return m_transformClips.empty() ? 1.0   : m_transformClips.back()->GetTransformScale(); }
TransformClip*  TransformClipStack::GetTop ()                       { return m_transformClips.empty() ? NULL  : m_transformClips.back(); }
ClipVectorCP    TransformClipStack::GetClip () const                { return m_transformClips.empty() ? NULL  : m_transformClips.back()->GetClip(); }
ClipVectorCP    TransformClipStack::GetDrawGeomClip () const        { return m_transformClips.empty() ? NULL  : m_transformClips.back()->GetDrawGeomClip(); }




 
























