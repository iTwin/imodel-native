/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/StrokeSymbol.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define LCPOINT_ANYVERTEX   (LCPOINT_LINEORG | LCPOINT_LINEEND | LCPOINT_LINEVERT)

typedef PointSymRsc*        PointSymRscP;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReference::RotationMode LsSymbolReference::GetRotationMode () const
    {
    if (m_mod1 & LCPOINT_ADJROT)
        return  ROTATE_Adjusted;

    if (m_mod1 & LCPOINT_ABSROT)
        return  ROTATE_Absolute;

    return  ROTATE_Relative;
    }

#if defined (NEEDSWORK_DGNITEM)
    /*---------------------------------------------------------------------------------**//**
    * calculate the "maximum offset from the origin" for this element descriptor chain.
    * @bsimethod                                                    Keith.Bentley   03/03
    +---------------+---------------+---------------+---------------+---------------+------*/
    static double getDescrMaxOffset (DgnElementPtrVec const& elems, DgnModelP model, double angle)
        {
        // This is wrong...needs to be based on the curve vector, not the element!
        double      maxWidth = 0, test;

        Transform transform;
    transform.InitFromPrincipleAxisRotations (RotMatrix::FromIdentity (), 0.0, 0.0, angle);

        for (auto descr: elems)
            {

            EditElementHandle  tmpElHandle (descr.get(), false);
            ElementHandlerR    handler = tmpElHandle.GetElementHandler();

            DRange3d    range;

            if (SUCCESS == handler.CalcElementRange (tmpElHandle, range, &transform))
                {
                // only consider the y component of the range for offset
                if ((test = fabs (range.low.y)) > maxWidth)
                    maxWidth = test;

                if ((test = fabs (range.high.y)) > maxWidth)
                    maxWidth = test;

                if ((test = fabs (range.low.z)) > maxWidth)
                    maxWidth = test;

                if ((test = fabs (range.high.z)) > maxWidth)
                    maxWidth = test;
                }
            }

        return maxWidth;
        }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
double LsSymbolReference::_GetMaxWidth (DgnModelP dgnModel) const
    {
#if defined(NOTNOW)
    if (NULL == GetElements())
        return  0.0;

    double maxWidth = getDescrMaxOffset (*GetElements(), dgnModel, m_angle) / m_symbol->GetMuDef();
    double offset   = m_offset.Magnitude ();

    return  (offset + maxWidth) * 2.0;
#endif
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void addClipPlane (ConvexClipPlaneSetR clipPlanes, DPoint3dCP pt, DPoint3dCP dir)
    {
    clipPlanes.push_back (ClipPlane (DVec3d::From (-dir->x, -dir->y, -dir->z), *pt));
    }

/*---------------------------------------------------------------------------------**//**
* Output this symbol ref at a specific location and direction, optionally clipping the origin and/or end.
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LsSymbolReference::Output (ViewContextP context, LineStyleSymbCP modifiers, DPoint3dCP org, DPoint3dCP dir, double const* xScale, DPoint3dCP clipOrg, DPoint3dCP clipEnd) const
    {
    if (NULL == m_symbol.get ())
        return ERROR;

    Transform  transform;
    RotMatrix   planeByRows;
    modifiers->GetPlaneAsMatrixRows (planeByRows);
    DVec3d xVector, yVector, zVector;
    planeByRows.GetRows(xVector, yVector, zVector);

    DVec3d uVector, vVector, wVector;
    uVector = *(DVec3d*)dir;
    wVector = zVector;
    vVector.NormalizedCrossProduct (wVector, uVector);
    transform.InitFromOriginAndVectors (*org, uVector, vVector, wVector);

    double      scale = (GetSymbolComponentCP()->IsNotScaled() ? 1.0 : modifiers->GetScale());
    transform.TranslateInLocalCoordinates (transform, m_offset.x * scale, m_offset.y * scale, 0.0);

    // Add rotation modifiers. Rotation is about the Z axis of the transform.
    switch (GetRotationMode())
        {
        case ROTATE_Relative:
            {
            // Can't call initFromPrincipleAxisRotations directly since it will multiply matrices in wrong order.
            RotMatrix rotation, product, baseMatrix;
            rotation.InitFromPrincipleAxisRotations (RotMatrix::FromIdentity (), 0.0, 0.0, m_angle);
            transform.GetMatrix (baseMatrix);
            product.InitProduct (baseMatrix, rotation);
            transform.SetMatrix (product);
            break;
            }

        case ROTATE_Absolute:
            {
            RotMatrix planeByColumns, rotatedMatrix;
            planeByColumns.InitFromColumnVectors (xVector, yVector, zVector);
            rotatedMatrix.InitFromPrincipleAxisRotations (planeByColumns, 0.0, 0.0, m_angle);
            transform.SetMatrix (rotatedMatrix);
            }
            break;

        case ROTATE_Adjusted:
            {
            // Adjust so that the X direction is left to right with vertical adjusted to read "up hill" (-y to +y)
            DVec3d xDir, yDir, zDir;
            DPoint3d org;
            transform.GetOriginAndVectors (org, xDir, yDir, zDir);
            DVec3d xDirTemp = xDir;
            planeByRows.Multiply(xDirTemp);
            if (xDirTemp.x < 0.0 || fabs (xDirTemp.y + 1.0) < .0001)
                {
                xDir.Negate ();
                yDir.Negate ();
                }
            transform.InitFromOriginAndVectors (org, xDir, yDir, zDir);

            RotMatrix rotation, product, baseMatrix;
            rotation.InitFromPrincipleAxisRotations (RotMatrix::FromIdentity (), 0.0, 0.0, m_angle);
            transform.GetMatrix (baseMatrix);
            product.InitProduct (baseMatrix, rotation);
            transform.SetMatrix (product);
            }
            break;
        }


    scale = (GetSymbolComponentCP()->IsNotScaled() ? 1.0 : modifiers->GetScale() / m_symbol->GetMuDef());
    DPoint3d    scaleVec;
    scaleVec.Init (scale, scale, scale);
    if (xScale)
        scaleVec.x *= *xScale;

    transform.ScaleMatrixColumns (transform, scaleVec.x, scaleVec.y, scaleVec.z);
 
    ConvexClipPlaneSet  convexClip;
    // if there is clip at either the beginning or the end of the symbol, set up the clip planes
    if (clipOrg || clipEnd)
        {
        if (clipOrg)
            {
            DPoint3d revDir = *dir;
            revDir.Scale (-1.0);
            addClipPlane (convexClip, clipOrg, &revDir);
            }

        if (clipEnd)
            addClipPlane (convexClip, clipEnd, dir);
        }

    ClipPlaneSet clips (convexClip);
    context->DrawSymbol (m_symbol.get (), &transform, &clips, !GetUseColor(), !GetUseWeight());

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void LsSymbolComponent::_Draw (ViewContextR context)
    {
#if defined (NEEDS_WORK_DGNITEM)
    DgnModelP   dgnCacheP = NULL;
    DgnViewportP   vp = context.GetViewport();

    if (NULL != vp)
        dgnCacheP = vp->GetViewController ().GetTargetModel ();

    XGraphicsContainer::DrawXGraphicsFromMemory (context, GetXGraphicsData (),
                                                static_cast <int32_t> (GetXGraphicsSize ()), 
                                                dgnCacheP, XGraphicsContainer::DRAW_OPTION_None);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LsSymbolComponent::_GetRange (DRange3dR range) const
    {
    range.low = m_symBase;  

    range.high.x = m_symBase.x + m_symSize.x;
    range.high.y = m_symBase.y + m_symSize.y;
    range.high.z = m_symBase.z + m_symSize.z;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolComponent::LsSymbolComponent (LsLocation const *pLocation) : LsComponent (pLocation)
    {
    m_muDef       = 0.0;
    m_storedScale = 0.0;
    m_symFlags    = 0;
    m_postProcessed = false;
    m_xGraphicsSize = 0; 
    m_xGraphicsData = NULL;

    memset (&m_symSize, 0, sizeof(m_symSize));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolComponent::~LsSymbolComponent ()
    {
    FreeGraphics ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void LsSymbolComponent::SetXGraphics (Byte const *data, size_t dataSize)
    {
    if (NULL != m_xGraphicsData)
        bentleyAllocator_free (const_cast <Byte*> (m_xGraphicsData));

    m_xGraphicsSize = dataSize;
    m_xGraphicsData = (Byte*)bentleyAllocator_malloc(dataSize);
    memcpy (const_cast <Byte*> (m_xGraphicsData), data, dataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsSymbolComponent::_DoStroke (ViewContextP context, DPoint3dCP inPoints, int nPoints, LineStyleSymbCP modifiers) const
    {
    BeAssert (0);  // symbol components should never be drawn this way
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void LsSymbolComponent::_PostProcessLoad (DgnModelP modelRef)
    {
    if (m_postProcessed)
        return;
        
    m_postProcessed = true;

    //   If the symbol was stored with no master unit definition (scale) use the working units from the master file.
    if (m_storedScale > 0.0)
        {
        SetMuDef (m_storedScale);
        }
    else
        {
        double muDef = 1.0/modelRef->GetProperties().GetMillimetersPerMaster();

        SetMuDef (muDef);
        }

    if (GetXGraphicsData() != NULL)
        return;    //  if the component was saved as XGraphics then the data is already loaded and there is no DgnElementDescr

#if defined (NEEDS_WORK_DGNITEM)
    XGraphicsRecorder   recorder (modelRef);

    recorder.EnableInitialWeightMatSym ();
    recorder.EnableInitialColorMatSym ();
    recorder.EnableInitialLineCodeMatSym();

    for (auto& descr : m_elements)
        {
        descr->SetDgnModel(*modelRef); // Probably no longer necessary...should have been set when elements were created...
        ElementHandle handle(descr.get(), false);
        
        recorder.GetContext ()->OutputElement (handle);
        }
        
    XGraphicsContainerR    xGraphicsContainer = const_cast <XGraphicsContainerR> (recorder.GetContainer ());
    
    size_t                  dataSize = xGraphicsContainer.GetDataSize ();
    Byte const*             data = xGraphicsContainer.GetData ();
    
    SetXGraphics (data, dataSize);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolComponent::_ClearPostProcess ()
    {
    m_postProcessed = false;
    //  Assume we need to regenerate the XGraphics
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolComponent* LsSymbolComponent::LoadPointSym (LsComponentReader* reader)
    {
    PointSymRsc*    symRsc = (PointSymRsc*) reader->GetRsc();

    if (NULL == symRsc)
        return  NULL;

    LsSymbolComponent* symbComp = new LsSymbolComponent (reader->GetSource());
    symbComp->SetDescription (Utf8String(symRsc->header.descr).c_str());

    symbComp->SetXGraphics (symRsc->symBuf, symRsc->nBytes);

    symbComp->SetFlags (symRsc->symFlags);

    DPoint3d symSize, symBase = symRsc->header.range.low;
    symSize.x = symRsc->header.range.high.x - symRsc->header.range.low.x;
    symSize.y = symRsc->header.range.high.y - symRsc->header.range.low.y;

    if (symRsc->symFlags & LSSYM_3D)
        {
        symSize.z = symRsc->header.range.high.z - symRsc->header.range.low.z;
        }
    else
        {
        symSize.z = 0.0;
        symBase.z = 0.0;
        }

    symbComp->SetSymSize (&symSize);
    symbComp->SetSymBase (&symBase);
    symbComp->SetStoredUnitScale (symRsc->header.scale);
    return symbComp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LsSymbolComponent::FreeGraphics ()
    {
    if (m_xGraphicsData)
        {
        m_xGraphicsSize = 0;
        bentleyAllocator_free(const_cast <Byte*> (m_xGraphicsData));
        m_xGraphicsData = nullptr;
        }
    }

double              LsSymbolComponent::GetStoredUnitScale () const { return m_storedScale; }
void                LsSymbolComponent::SetStoredUnitScale (double storedScale) { m_muDef = m_storedScale = storedScale; }
double              LsSymbolComponent::GetUnitScale () const { return m_muDef; }
bool                LsSymbolComponent::IsNoScale () const { return IsNotScaled (); }
void                LsSymbolComponent::SetIsNoScale (bool value) { m_symFlags = (m_symFlags & ~LSSYM_NOSCALE) | (value ? LSSYM_NOSCALE : 0); }
bool                LsSymbolComponent::Is3d ()   const { return (m_symFlags & LSSYM_3D) != 0; }
void                LsSymbolComponent::GetRange (DRange3dR range) const 
    { 
    _GetRange (range);
    }
