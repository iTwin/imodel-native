/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define LCPOINT_ANYVERTEX   (LCPOINT_LINEORG | LCPOINT_LINEEND | LCPOINT_LINEVERT)

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
static double getGeometryPartMaxOffset (LsSymbolComponentCR symbol, double angle)
    {
    DRange3d range;
    symbol.GetRange(range);

    Transform transform;
    transform.InitFromPrincipleAxisRotations(Transform::FromIdentity(), 0.0, 0.0, angle);
    transform.Multiply(range.low);
    transform.Multiply(range.high);

    double maxWidth = fabs (range.low.y);
    double test;

    if ((test = fabs (range.high.y)) > maxWidth)
        maxWidth = test;

    if ((test = fabs (range.low.z)) > maxWidth)
        maxWidth = test;

    if ((test = fabs (range.high.z)) > maxWidth)
        maxWidth = test;

    return maxWidth;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
double LsSymbolReference::_GetMaxWidth () const
    {
    if (NULL == m_symbol.get ())
        return 0.0;

    double offset   = m_offset.Magnitude ();
    double maxWidth = getGeometryPartMaxOffset(*m_symbol, m_angle) / (m_symbol->IsNotScaled() ? 1.0 : m_symbol->GetMuDef());

    return  (offset + maxWidth) * 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void addClipPlane (ConvexClipPlaneSetR clipPlanes, DPoint3dCP pt, DPoint3dCP dir)
    {
    clipPlanes.push_back (ClipPlane (DVec3d::From (-dir->x, -dir->y, -dir->z), *pt));
    }

/*---------------------------------------------------------------------------------**//**
* Output this symbol ref at a specific location and direction, optionally clipping the origin and/or end.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LsSymbolReference::Output(LineStyleContextR lineStyleContext, LineStyleSymbCP modifiers, DPoint3dCP orgIn, DPoint3dCP dir, double const* xScale, DPoint3dCP clipOrg, DPoint3dCP clipEnd) const
    {
    if (NULL == m_symbol.get())
        return ERROR;

    Transform transform;
    RotMatrix planeByRows;
    modifiers->GetPlaneAsMatrixRows(planeByRows);
    DVec3d xVector, yVector, zVector;
    planeByRows.GetRows(xVector, yVector, zVector);

    DVec3d uVector, vVector, wVector;
    uVector = *(DVec3dCP)dir;
    wVector = zVector;
    vVector.NormalizedCrossProduct(wVector, uVector);
    transform.InitFromOriginAndVectors(*orgIn, uVector, vVector, wVector);

    double      scale = (GetSymbolComponentCP()->IsNotScaled() ? 1.0 : modifiers->GetScale());
    transform.TranslateInLocalCoordinates(transform, m_offset.x * scale, m_offset.y * scale, 0.0);

    // Add rotation modifiers. Rotation is about the Z axis of the transform.
    switch (GetRotationMode())
        {
        case ROTATE_Relative:
            {
            // Can't call initFromPrincipleAxisRotations directly since it will multiply matrices in wrong order.
            RotMatrix rotation, product, baseMatrix;
            rotation.InitFromPrincipleAxisRotations (RotMatrix::FromIdentity(), 0.0, 0.0, m_angle);
            transform.GetMatrix(baseMatrix);
            product.InitProduct(baseMatrix, rotation);
            transform.SetMatrix(product);
            break;
            }

        case ROTATE_Absolute:
            {
            RotMatrix planeByColumns, rotatedMatrix;
            planeByColumns.InitFromColumnVectors(xVector, yVector, zVector);
            rotatedMatrix.InitFromPrincipleAxisRotations(planeByColumns, 0.0, 0.0, m_angle);
            transform.SetMatrix(rotatedMatrix);
            break;
            }

        case ROTATE_Adjusted:
            {
            // Adjust so that the X direction is left to right with vertical adjusted to read "up hill" (-y to +y)
            DVec3d xDir, yDir, zDir;
            DPoint3d org;
            transform.GetOriginAndVectors(org, xDir, yDir, zDir);
            DVec3d xDirTemp = xDir;
            planeByRows.Multiply(xDirTemp);

            if (xDirTemp.x < 0.0 || fabs(xDirTemp.y + 1.0) < .0001)
                {
                xDir.Negate ();
                yDir.Negate ();
                }

            transform.InitFromOriginAndVectors(org, xDir, yDir, zDir);
            RotMatrix rotation, product, baseMatrix;
            rotation.InitFromPrincipleAxisRotations(RotMatrix::FromIdentity (), 0.0, 0.0, m_angle);
            transform.GetMatrix(baseMatrix);
            product.InitProduct(baseMatrix, rotation);
            transform.SetMatrix(product);
            break;
            }
        }

    scale = (GetSymbolComponentCP()->IsNotScaled() ? 1.0 : modifiers->GetScale() / m_symbol->GetMuDef());
    DPoint3d scaleVec;
    scaleVec.Init(scale, scale, scale);
    if (xScale)
        scaleVec.x *= *xScale;

    transform.ScaleMatrixColumns(transform, scaleVec.x, scaleVec.y, scaleVec.z);

    // if there is clip at either the beginning or the end of the symbol, set up the clip planes
    if (clipOrg || clipEnd)
        {
        ConvexClipPlaneSet convexPlanes;

        if (clipOrg)
            {
            DPoint3d revDir = *dir;

            revDir.Scale(-1.0);
            addClipPlane(convexPlanes, clipOrg, &revDir);
            }

        if (clipEnd)
            {
            addClipPlane(convexPlanes, clipEnd, dir);
            }

        ClipPlaneSet  planes(convexPlanes);
        ClipVectorPtr clip = ClipVector::Create();

        if (SUCCESS == ClipVector::AppendPlanes(clip, planes))
            {
            m_symbol->Draw(lineStyleContext, transform, clip.get(), GetUseElementColor(), GetUseElementWeight());

            return SUCCESS;
            }
        }

    m_symbol->Draw(lineStyleContext, transform, nullptr, GetUseElementColor(), GetUseElementWeight());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LsSymbolComponent::Draw(LineStyleContextR context, TransformCR transform, ClipVectorCP clip, bool ignoreColor, bool ignoreWeight)
    {
    // NOTE: Unfortunately we can't just call ViewContext::AddSubGraphic for symbols since it won't support things like ignoreColor, ignoreWeight...
    DgnGeometryPartCPtr geomPart = GetGeometryPart();

    if (!geomPart.IsValid())
        return;

    Render::GraphicBuilderR mainGraphic = context.GetGraphicR();
    ViewContextR viewContext = context.GetViewContext();
    Transform partToWorld = Transform::FromProduct(mainGraphic.GetLocalToWorldTransform(), transform);
    ElementAlignedBox3d range = geomPart->GetBoundingBox();

    partToWorld.Multiply(range, range);

    if (!viewContext.IsRangeVisible(range))
        return; // Part range doesn't overlap pick/fence...

    Render::GeometryParamsCR baseParams = context.GetGeometryParams();
    bool cookParams = baseParams.GetCategoryId().IsValid(); // NOTE: LineStyleRangeCollector doesn't care about base symbology...
    bool creatingTexture = context.GetCreatingTexture();
    Render::GraphicBuilderPtr symbolGraphic = mainGraphic.CreateSubGraphic(transform, clip);

    GeometryCollection collection(geomPart->GetGeometryStream(), *GetDgnDbP(), &baseParams);

    for (auto iter : collection)
        {
        GeometricPrimitivePtr geometry = iter.GetGeometryPtr();

        if (!geometry.IsValid())
            continue;

        if (cookParams)
            {
            // NOTE: Symbol geometry can have line codes and other symbology changes.
            Render::GeometryParams symbParams(iter.GetGeometryParams());

            if (ignoreColor)
                {
                symbParams.SetLineColor(baseParams.GetLineColor()); // Should transparency also come from baseParams???
                symbParams.SetFillColor(baseParams.GetLineColor()); // BaseParams doesn't have seperate fill color - synch fill with color.
                }
            else if (creatingTexture)
                {
                context.SetHasTextureColors();
                }
            
            if (ignoreWeight || creatingTexture) // TextureParams cache doesn't support weight changes on symbol components...
                symbParams.SetWeight(baseParams.GetWeight());

            if (nullptr != symbParams.GetLineStyle() && !symbParams.GetLineStyle()->GetLineStyleSymb().UseLinePixels())
                symbParams.SetLineStyle(nullptr);

            viewContext.CookGeometryParams(symbParams, *symbolGraphic);

            // NOTE: Symbols need to be independent of ViewFlags, treat FillDisplay::ByView as FillDisplay::Always...
            if (FillDisplay::Never != symbParams.GetFillDisplay() && GeometricPrimitive::GeometryType::CurveVector == geometry->GetGeometryType())
                {
                CurveVectorCR curveVector = *geometry->GetAsCurveVector();

                symbolGraphic->AddCurveVector(curveVector, curveVector.IsAnyRegionType());
                continue;
                }
            }

        geometry->AddToGraphic(*symbolGraphic);
        }

    Render::GraphicParams graphicParams;

    if (cookParams)
        {
        Render::GeometryParams tmpGeomParams(baseParams);
        viewContext.CookGeometryParams(tmpGeomParams, graphicParams);
        }

    mainGraphic.AddSubGraphic(*symbolGraphic->Finish(), transform, graphicParams, clip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LsSymbolComponent::GetRange (DRange3dR range) const
    {
    range.low = m_symBase;  

    range.high.x = m_symBase.x + m_symSize.x;
    range.high.y = m_symBase.y + m_symSize.y;
    range.high.z = m_symBase.z + m_symSize.z;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
    m_postProcessed = false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolComponent::LsSymbolComponent (LsLocation const *pLocation) : LsComponent (pLocation)
    {
    m_muDef       = 0.0;
    m_storedScale = 0.0;
    m_symFlags    = 0;
    m_postProcessed = false;

    memset (&m_symSize, 0, sizeof(m_symSize));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolComponent::~LsSymbolComponent ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsSymbolComponent::_DoStroke (LineStyleContextR context, DPoint3dCP inPoints, int nPoints, LineStyleSymbR modifiers) const
    {
    BeAssert (0);  // symbol components should never be drawn this way
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LsSymbolComponent::_PostProcessLoad ()
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolComponent::_ClearPostProcess ()
    {
    m_postProcessed = false;
    //  Assume we need to regenerate the XGraphics
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnGeometryPartCPtr LsSymbolComponent::GetGeometryPart() const
    {
    return GetDgnDbP()->Elements().Get<DgnGeometryPart>(m_geomPartId);
    }
