/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/StrokeSymbol.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define LCPOINT_ANYVERTEX   (LCPOINT_LINEORG | LCPOINT_LINEEND | LCPOINT_LINEVERT)

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

//---------------------------------------------------------------------------------------
// calculate the "maximum offset from the origin" for this XGraphics container
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
static double getGeometryPartMaxOffset (LsSymbolComponentCR symbol, double angle)
    {
    //  NEEDSWORK_LINESTYLES  It would be better to draw this with the transform instead of transforming the range
    Transform transform;
    transform.InitFromPrincipleAxisRotations(Transform::FromIdentity(), 0.0, 0.0, angle);
    DRange3d        range;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    symbol._GetRange(range);
#endif
    transform.Multiply(range.low);
    transform.Multiply(range.high);

    double      maxWidth = fabs (range.low.y);
    double      test;

    if ((test = fabs (range.high.y)) > maxWidth)
        maxWidth = test;

    if ((test = fabs (range.low.z)) > maxWidth)
        maxWidth = test;

    if ((test = fabs (range.high.z)) > maxWidth)
        maxWidth = test;

    return maxWidth;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
double LsSymbolReference::_GetMaxWidth (DgnModelP dgnModel) const
    {
    if (NULL == m_symbol.get ())
        return 0.0;

    double offset   = m_offset.Magnitude ();
    double maxWidth = getGeometryPartMaxOffset(*m_symbol, m_angle)/m_symbol->GetMuDef();

    return  (offset + maxWidth) * 2.0;
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

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    ClipPlaneSet clips (convexClip);
    context->DrawSymbol (m_symbol.get (), &transform, &clips);
#endif

    return  SUCCESS;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void LsSymbolComponent::_Draw (ViewContextR context)
    {
    DgnGeometryPartPtr geomPart = GetGeometryPart();
    if (!geomPart.IsValid())
        return;

    GeometryStreamIO::Collection collection(geomPart->GetGeometryStream().GetData(), geomPart->GetGeometryStream().GetSize());
    collection.Draw(context, context.GetCurrentGeometryParams().GetCategoryId(), context.GetViewFlags()); 
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
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsSymbolComponent::LsSymbolComponent(LsSymbolComponentCR src) : LsComponent(&src)
    {
    m_isModified = true;
    m_geomPartId = src.m_geomPartId;
    m_storedScale = src.m_storedScale;
    m_muDef = src.m_muDef;
    m_symSize = src.m_symSize;
    m_symBase = src.m_symBase;;
    m_symFlags = src.m_symFlags;
    m_lineColor = src.m_lineColor;
    m_fillColor = src.m_fillColor;
    m_weight = src.m_weight;
    m_lineColorByLevel = src.m_lineColorByLevel;
    m_postProcessed = false;
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
    m_lineColorByLevel = false;

    memset (&m_symSize, 0, sizeof(m_symSize));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolComponent::~LsSymbolComponent ()
    {
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
        //  NEEDSWORK_LINESTYLE_UNITS
        double muDef = 1.0;  //  /modelRef->GetProperties().GetMillimetersPerMaster();

        SetMuDef (muDef);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolComponent::_ClearPostProcess ()
    {
    m_postProcessed = false;
    //  Assume we need to regenerate the XGraphics
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2015
//---------------------------------------------------------------------------------------
void LsSymbolComponent::SetColors(bool colorByLevel, ColorDef lineColor, ColorDef fillColor)
    {
    m_lineColorByLevel = colorByLevel;
    m_lineColor = lineColor;
    m_fillColor = fillColor;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2015
//---------------------------------------------------------------------------------------
void LsSymbolComponent::SetWeight(uint32_t weight) { m_weight = weight; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolComponent* LsSymbolComponent::LoadPointSym (LsComponentReader* reader)
    {
    Json::Value     jsonValue;
    reader->GetJsonValue(jsonValue);
    LsSymbolComponentP symbolComponent;
    LsSymbolComponent::CreateFromJson(&symbolComponent, jsonValue, reader->GetSource());
    return symbolComponent;
    }

double              LsSymbolComponent::GetStoredUnitScale () const { return m_storedScale; }
void                LsSymbolComponent::SetStoredUnitScale (double storedScale) { m_muDef = m_storedScale = storedScale; }
double              LsSymbolComponent::GetUnitScale () const { return m_muDef; }
bool                LsSymbolComponent::IsNoScale () const { return IsNotScaled (); }
void                LsSymbolComponent::SetIsNoScale (bool value) { m_symFlags = (m_symFlags & ~LSSYM_NOSCALE) | (value ? LSSYM_NOSCALE : 0); }
bool                LsSymbolComponent::Is3d ()   const { return (m_symFlags & LSSYM_3D) != 0; }
void                LsSymbolComponent::GetRange (DRange3dR range) const 
    { 
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    _GetRange (range);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    07/2015
//---------------------------------------------------------------------------------------
DgnGeometryPartCPtr LsSymbolComponent::GetGeometryPart() const
    {
    if (m_geomPart.IsValid())
        return m_geomPart;

    m_geomPart = GetDgnDbP()->Elements().Get<DgnGeometryPart>(m_geomPartId);
    return m_geomPart;
    }
