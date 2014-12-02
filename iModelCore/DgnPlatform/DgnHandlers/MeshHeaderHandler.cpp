/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/MeshHeaderHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            MeshHeaderHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_MESH_HEADER_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MeshHeaderHandler::_OnTransform
(
EditElementHandleR  eeh,
TransformInfoCR     trans
)
    {
    if (SUCCESS != T_Super::_OnTransform (eeh, trans))
        return ERROR;

    PolyfaceHeaderPtr meshData;

    if (SUCCESS != _GetMeshData (eeh, meshData))
        return ERROR;

    meshData->Transform (*trans.GetTransform (), true);
    
    return _SetMeshData (eeh, *meshData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MeshHeaderHandler::_FenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR eh,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    // Don't want optimized clip handled by base class...
    return _OnFenceClip (inside, outside, eh, fp, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      01/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MeshHeaderHandler::_OnFenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR eh,
FenceParamsP    fenceParams,
FenceClipFlags  options
)
    {
    ClipVectorPtr        clip;
    ClipPlaneSetCP       clipPlanes;

    if (NULL == fenceParams || ! (clip = fenceParams->GetClipVector ()).IsValid() || clip->empty())
        return ERROR;

    BeAssert (1 == clip->size());
    
    ClipPrimitivePtr        clipPrimitive = clip->front();
    BeAssert (clipPrimitive.IsValid());

    if (NULL == (clipPlanes = clipPrimitive->GetClipPlanes ()))
        return ERROR;

    PolyfaceHeaderPtr inputMesh;

    if (SUCCESS != MeshHeaderHandler::PolyfaceFromElement (inputMesh, eh))
        return ERROR;

    Transform   clipToWorld, worldToClip;

    if (NULL == fenceParams->GetTransform ())
        worldToClip.InitIdentity();
    else
        worldToClip = *fenceParams->GetTransform();
    
    clipToWorld.InverseOf (worldToClip);

    PolyfaceHeaderPtr        insideMesh, outsideMesh;
    PolyfaceCoordinateMapPtr insideMap, outsideMap;

    if (NULL != inside)
        {
        insideMesh = PolyfaceHeader::New ();
        insideMesh->ActivateVectorsForIndexing (*inputMesh);
        insideMap = PolyfaceCoordinateMap::New (*insideMesh);
        }

    if (NULL != outside)
        {
        outsideMesh = PolyfaceHeader::New ();
        outsideMesh->ActivateVectorsForIndexing (*inputMesh);
        outsideMap = PolyfaceCoordinateMap::New (*outsideMesh);
        }

    bool    cutFaceFailures;

    inputMesh->Transform (worldToClip, false);
    PolyfaceCoordinateMap::AddClippedPolyface (*inputMesh, insideMap.get(), outsideMap.get (), cutFaceFailures, (ClipPlaneSetP) clipPlanes, inputMesh->IsClosedByEdgePairing ());

    if (NULL != inside)
        {
        insideMesh->Transform (clipToWorld, false);

        EditElementHandle   eeh;

        if (SUCCESS == MeshHeaderHandler::CreateMeshElement (eeh, &eh, *insideMesh, eh.GetElementCP ()->Is3d(), *eh.GetDgnModelP ()))
            inside->Insert (eeh);
        }

    if (NULL != outside)
        {
        outsideMesh->Transform (clipToWorld, false);

        EditElementHandle   eeh;

        if (SUCCESS == MeshHeaderHandler::CreateMeshElement (eeh, &eh, *outsideMesh, eh.GetElementCP ()->Is3d(), *eh.GetDgnModelP ()))
            outside->Insert (eeh);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MeshHeaderHandler::_OnFenceStretch
(
EditElementHandleR  eeh,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    PolyfaceHeaderPtr meshData;

    if (SUCCESS != _GetMeshData (eeh, meshData))
        return ERROR;

    BlockedVector<DPoint3d> &point = meshData->Point ();

    if (!point.Active ())
        return ERROR;

    size_t  numPoint = point.size ();

    for (size_t i = 0; i < numPoint; i++)
        {
        if (fp->PointInside (point[i]))
            transform.GetTransform ()->Multiply (point[i]);
        }

    return _SetMeshData (eeh, *meshData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MeshHeaderHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Solids & geometry.GetOptions ()))
        return ERROR;

    PolyfaceHeaderPtr meshData;

    if (SUCCESS != _GetMeshData (eh, meshData))
        return ERROR;

    if (DropGeometry::SOLID_Wireframe == geometry.GetSolidsOptions ())
        {
        bvector<DSegment3d> segments;

        meshData->CollectSegments (segments, true);

        if (0 == segments.size ())
            return ERROR;

        for (DSegment3d segment : segments)
            {
            EditElementHandle   edgeEeh;

            if (SUCCESS != LineHandler::CreateLineElement (edgeEeh, NULL, segment, eh.GetElementCP ()->Is3d(), *eh.GetDgnModelP ()))
                continue;

            ElementPropertiesSetter::ApplyTemplate (edgeEeh, eh);

            dropGeom.Insert (edgeEeh);
            }

        return SUCCESS;
        }

    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*meshData.get (), true);

    visitor->SetNumWrap (1);

    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
        {
        EditElementHandle   faceEeh;

        if (SUCCESS != ShapeHandler::CreateShapeElement (faceEeh, NULL, &visitor->Point ().front (), visitor->Point ().size (), eh.GetElementCP ()->Is3d(), *eh.GetDgnModelP ()))
            continue;

        ElementPropertiesSetter::ApplyTemplate (faceEeh, eh);

        bool            singleColor = true;
        UInt32          elemColor = INVALID_COLOR;
        DgnColorMapCP   colorMap = DgnColorMap::Get (*eh.GetDgnProject());

        bvector<RgbColorDef> vertexRGBs;

        if (visitor->ColorTable ().Active () && 0 != visitor->ColorTable ().size ())
            {
            elemColor = visitor->ColorTable ().front ();

            for (UInt32 const& thisColor : visitor->ColorTable ())
                {
                if (thisColor == elemColor)
                    continue;

                singleColor = false;
                break;
                }

            if (!singleColor)
                {
                // NOTE: COLOR_BYCELL/COLOR_BYLEVEL not supported per-vertex since it's stored as RGB...
                for (UInt32 const& thisColor : visitor->ColorTable ())
                    vertexRGBs.push_back (colorMap->GetColor (thisColor).m_rgb);
                }
            }
        else if (visitor->IntColor ().Active () && 0 != visitor->IntColor ().size ())
            {
            UInt32  intColor = visitor->IntColor ().front ();

            for (UInt32 const& thisColor : visitor->IntColor ())
                {
                if (thisColor == intColor)
                    continue;

                singleColor = false;
                break;
                }

            if (!singleColor)
                {
                for (UInt32 const& thisColor : visitor->IntColor ())
                    vertexRGBs.push_back (IntColorDef (thisColor).m_rgb);
                }
            else
                {
                // NOTE: Could create extended color...old code used closest match...
                elemColor = colorMap->FindClosestMatch (IntColorDef (intColor));
                }
            }
        else if (visitor->DoubleColor ().Active () && 0 != visitor->DoubleColor ().size ())
            {
            RgbFactor   rgbColor = visitor->DoubleColor ().front ();

            for (RgbFactor const& thisColor : visitor->DoubleColor ())
                {
                if (thisColor.red == rgbColor.red && thisColor.green == rgbColor.green && thisColor.blue == rgbColor.blue)
                    continue;

                singleColor = false;
                break;
                }

            if (!singleColor)
                {
                for (RgbFactor const& thisColor : visitor->DoubleColor ())
                    {
                    RgbColorDef colorDef;

                    ColorUtil::ConvertRgbFactorToRgbColorDef (colorDef, thisColor);
                    vertexRGBs.push_back (colorDef);
                    }
                }
            else
                {
                RgbColorDef colorDef;

                ColorUtil::ConvertRgbFactorToRgbColorDef (colorDef, rgbColor);

                // NOTE: Could create extended color...old code used closest match...
                elemColor = colorMap->FindClosestMatch (IntColorDef (colorDef));
                }
            }

        DVec3dCP      normals = NULL;
        DPoint2dCP    parameters = NULL;
        RgbColorDefCP colors = NULL;

        if (visitor->Normal ().Active () && 0 != visitor->Normal ().size ())
            normals = &visitor->Normal ().front ();

        if (visitor->Param ().Active () && 0 != visitor->Param ().size ())
            parameters = &visitor->Param ().front ();

        if (singleColor && INVALID_COLOR != elemColor)
            {
            ElementPropertiesSetter remapper;

            remapper.SetColor (elemColor);
            remapper.Apply (faceEeh);
            }
        else if (0 != vertexRGBs.size ())
            {
            colors = &vertexRGBs.front ();
            }

        // Add any optional vertex data to shape...
        ShapeHandler::AppendVertexData (faceEeh, normals, parameters, colors);

        dropGeom.Insert (faceEeh);
        }

    return SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass                                                     EarlinLutz      01/05
+===============+===============+===============+===============+===============+======*/
struct          MeshStroker : IStrokeForCache
{
private:

static const int    s_faceSizeLimit = INT_MAX; // currently, no limit
bool                m_invertWhite;
bool                m_nonSnappable;

public:

MeshStroker (bool nonSnappable) {m_nonSnappable = nonSnappable; m_invertWhite = false;}

virtual DrawExpense _GetDrawExpense () override                     { return DrawExpense::High; }
virtual bool        _WantLocateByStroker () override {return !m_nonSnappable;} // Optimize locate for non-snappable meshes, don't call _StrokeForCache to output edge geometry...
virtual void        _StrokeForCache (CachedDrawHandleCR, ViewContextR, double pixelSize) override;

bool ComputeInvertWhite (ElementHandleCR, ViewContextR);
bool GetMinimalLoad (ViewContextR);
bool LoadAndPrepPolyfaceHeader (MeshStroker*, PolyfaceHeaderPtr&, ElementHandleCR, ViewContextR);
void TranslateAuxiliaryColors (PolyfaceHeaderPtr, ElementHandleCR, ViewContextR, bool invertWhite);

}; // MeshStroker

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      07/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MeshStroker::GetMinimalLoad (ViewContextR context)
    {
    // TR #194719: cached mesh should never be minimal; it should always have convex planar faces and aux data
    if (context.CheckICachedDraw ())
        return false;

    // minimal load is for when we don't care about convexity, planarity, auxiliary data
    DrawPurpose drawPurpose = context.GetDrawPurpose();

    return DrawPurpose::Pick             == drawPurpose ||
           DrawPurpose::RangeCalculation == drawPurpose ||
           DrawPurpose::FitView          == drawPurpose ||
           DrawPurpose::FenceAccept      == drawPurpose;
    }

/*---------------------------------------------------------------------------------**//**
* @description Populate PolyfaceVectors, prep for ViewContextP
* @return true if mesh data extracted successfully
* @bsimethod                                                    DavidAssaf      02/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MeshStroker::LoadAndPrepPolyfaceHeader
(
MeshStroker*        stroker,
PolyfaceHeaderPtr&  meshData,
ElementHandleCR     eh,
ViewContextR        context
)
    {
    meshData = NULL;

    // NEEDS WORK: Optimize for minimal load?
    if (SUCCESS != MeshHeaderHandler::PolyfaceFromElement (meshData, eh))
        return false;

    UInt32      numPerFace = meshData->GetNumPerFace ();

    if (DgnPlatformLib::Host::GraphicsAdmin::CONTROLPOLY_DISPLAY_Always == T_HOST.GetGraphicsAdmin()._GetControlPolyDisplay ())
        {
        BlockedVectorIntR indexArray = meshData->PointIndex ();
        size_t            numIndex = indexArray.size ();

        for (size_t i = 0; i < numIndex; i++)
            indexArray[i] = abs (indexArray[i]);
        }

    // Triangulate index arrays if output is QVision and:
    // * a face is too large (potential problems downstream?), or
    // * a face is nonplanar (resolve any ambiguity), or
    // * a face is nonconvex (QVision triangulation assumes convexity)
    if (!context.GetIViewDraw().IsOutputQuickVision ())
        return true;

    size_t  minLoop, maxLoop, numFace;
    bool    bHasNonPlanarFace, bHasNonConvexFace;

    meshData->InspectFaces (numFace, minLoop, maxLoop, bHasNonPlanarFace, bHasNonConvexFace);

    if ((maxLoop > s_faceSizeLimit) || bHasNonPlanarFace || bHasNonConvexFace)
        meshData->Triangulate ();

    // Define texture for illuminated mesh...
    meshData->SetTextureId (context.GetIViewDraw().DefineQVTexture (meshData->GetIlluminationNameCP (), &context.GetDgnProject()));

    // QVision convention for variable-length face blocking
    if (numPerFace == 1)
        meshData->SetNumPerFace (0);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Translate mesh auxiliary colors to RGB float triples.
*
* @param pArrays        IN      polyface arrays
* @param elemHandle     IN      mesh element handle
* @param context        IN      view context for table color translation
* @param invertWhite    IN      true to apply viewport color logic to invert colors.
* @bsimethod                                                    DavidAssaf      02/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            MeshStroker::TranslateAuxiliaryColors
(
PolyfaceHeaderPtr   meshData,
ElementHandleCR     eh,
ViewContextR        context,
bool                invertWhite
)
    {
    BlockedVectorUInt32R    intColorVector   = meshData->IntColor ();
    BlockedVectorUInt32R    colorTableVector = meshData->ColorTable ();
    BlockedVectorRgbFactorR rgbColorVector   = meshData->DoubleColor ();
    BlockedVectorFloatRgbR  floatRgbVector   = meshData->FloatColor ();

    size_t      nColors = 0;

    // Skip of no color data ...
    if (intColorVector.Active ())
        nColors = intColorVector.size ();
    else if (colorTableVector.Active ())
        nColors = colorTableVector.size ();
    else if (rgbColorVector.Active ())
        nColors = rgbColorVector.size ();

    if (nColors <= 0)
        return;

    floatRgbVector.clear ();

    if (intColorVector.Active ())
        {
        for (size_t i = 0; i < nColors; i++)
            {
            FPoint3d    floatColor;

            ColorUtil::ConvertIntToFloatRgb (floatColor, intColorVector[i]);
            floatRgbVector.push_back (*(BentleyApi::FloatRgb*) &floatColor);            
            }
        }
    else if (rgbColorVector.Active ())
        {
        for (size_t i = 0; i < nColors; i++)
            {
            FPoint3d    floatColor;

            bsiFPoint3d_initFromDPoint3d (&floatColor, reinterpret_cast <DPoint3d*>(&rgbColorVector[i]));
            floatRgbVector.push_back (*(BentleyApi::FloatRgb*)&floatColor);            
            }
        }
    else if (colorTableVector.Active ())
        {
        UInt32      white = invertWhite ? context.GetViewport()->GetMenuColor (WHITE_MENU_COLOR_INDEX) : 0;
        UInt32      black = invertWhite ? context.GetViewport()->GetMenuColor (BLACK_MENU_COLOR_INDEX) : 0;
        DgnProjectR project = context.GetDgnProject();

        for (size_t i = 0; i < nColors; i++)
            {
            IntColorDef mappedColor (255, 255, 255); // Initialized to white...

            // Allegedly COLOR_BYCELL/BYLEVEL are acceptable values...NOTE: Problem w/caching as fixed rgb...
            if (COLOR_BYCELL == colorTableVector[i])
                {
                ElemHeaderOverridesCP ovr = context.GetHeaderOvr ();

                if (ovr)
                    project.Colors().Extract (&mappedColor, NULL, NULL, NULL, NULL, ovr->GetColor ());
                }
            else if (COLOR_BYLEVEL == colorTableVector[i])
                {
                auto appear = context.GetViewport()->GetViewController().GetSubLevelAppearance(context.GetCurrentDisplayParams ()->GetLevelSubLevelId());
                project.Colors().Extract(&mappedColor, NULL, NULL, NULL, NULL, appear.GetColor());
                }
            else
                {
                project.Colors().Extract (&mappedColor, NULL, NULL, NULL, NULL, colorTableVector[i]);
                }

            if (invertWhite && white == (mappedColor.m_int & 0x00ffffff))
                mappedColor.m_int = black;

            FPoint3d    floatColor;

            ColorUtil::ConvertIntToFloatRgb (floatColor, mappedColor.m_int);
            floatRgbVector.push_back (*(BentleyApi::FloatRgb*)&floatColor);
            }
        }

    floatRgbVector.SetActive (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MeshStroker::ComputeInvertWhite (ElementHandleCR eh, ViewContextR context)
    {
    m_invertWhite = false;

    // Minimal loads don't care about colors
    if (GetMinimalLoad (context))
        return false;

    ViewportP  viewport = context.GetViewport();

    if (!viewport)
        return false;

    UInt32      white = viewport->GetMenuColor (WHITE_MENU_COLOR_INDEX);

    // to invert, the background must be white
    if (viewport->GetBackgroundColor() != white)
        return false;

    // to invert, the view must be unshaded
    ViewFlagsCP viewFlags = context.GetViewFlags();

    if ( (NULL == viewFlags) || (viewFlags->renderMode > static_cast<UInt32>(MSRenderMode::Wireframe)))
        return false;

    DgnProjectR project = context.GetDgnProject();

    for (ChildElemIter childEh (eh, ExposeChildrenReason::Count); childEh.IsValid (); childEh = childEh.ToNext ())
        {
        if (MATRIX_HEADER_ELM != childEh.GetLegacyType())
            continue;

        if (MESH_ELM_TAG_TABLE_COLOR != childEh.GetElementCP()->ToMatrix_header().tag)
            continue;

        for (ChildElemIter dataEh (childEh, ExposeChildrenReason::Count); dataEh.IsValid (); dataEh = dataEh.ToNext ())
            {
            if (MATRIX_INT_DATA_ELM != dataEh.GetLegacyType())
                continue;

            DgnElementCP dataElm = dataEh.GetElementCP ();

            for (UInt32 iColor=0; iColor < dataElm->ToMatrix_int_data().numValue; iColor++)
                {
                IntColorDef mappedColor;

                if (SUCCESS != project.Colors().Extract (&mappedColor, NULL, NULL, NULL, NULL, dataElm->ToMatrix_int_data().data[iColor]))
                    continue;

                if (white != (mappedColor.m_int & 0x00ffffff))
                    continue;

                m_invertWhite = true;

                return true;
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      07/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            MeshStroker::_StrokeForCache(CachedDrawHandleCR dh,ViewContextR context, double pixelSize)
    {
    BeAssert(NULL != dh.GetElementHandleCP());
    ElementHandleCR eh = *dh.GetElementHandleCP();
    PolyfaceHeaderPtr meshData;

    if (!LoadAndPrepPolyfaceHeader (this, meshData, eh, context) || 
        !meshData->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ())
        return;

    TranslateAuxiliaryColors (meshData, eh, context, m_invertWhite);

    // In 8.11, there are some default setups here:
    // color indices may be present in pIIndex or p3DIndex
    // 811 code int*        pRGBIndices = arrays.pIIndex ? jmdlEmbeddedIntArray_getPtr (arrays.pIIndex, 0) : jmdlEmbeddedIntArray_getPtr (arrays.p3DIndex, 0);
    // 811 code
    // 811 code // in non-illuminated meshes, only one of RGB/UV can be indexed by pRGBUVIndexBuffer: prefer RGB
    // 811 code int*        pRGBUVIndices = pRGB ? pRGBIndices : jmdlEmbeddedIntArray_getPtr (arrays.pUVIndex, 0);
    // 811 code DPoint2d*   pUV           = pRGB ? NULL        : jmdlEmbeddedDPoint2dArray_getPtr (arrays.pUV, 0);
    // TR307839 Mesh with both uv and color data.   suppress the uv so explicit color wins.  But the other defaults are still not done.
    if (context.GetIViewDraw().IsOutputQuickVision ()   // EDL 02/13 ony QV has issue with uv+color, don't do this to anyone else.
        && meshData->Param ().Active () && meshData->FloatColor ().Active())
        {
        meshData->Param ().SetActive (false);
        meshData->Param ().clear ();
        }
    
    context.GetIDrawGeom().DrawPolyface (*meshData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      07/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     MeshHeaderHandler::_Draw (ElementHandleCR eh, ViewContextR context)
    {
    MeshStroker stroker (DrawPurpose::Pick == context.GetDrawPurpose () && eh.GetElementCP ()->IsSnappable());
    bool        invertWhite = stroker.ComputeInvertWhite (eh, context); // TR#187606: Invert white color table entries on white bg...

    context.DrawCached (eh, stroker, invertWhite ? 1 : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      MeshHeaderHandler::_OnSnap (SnapContextP context, int snapPathIndex)
    {
    if (!context->IsSnappableElement (snapPathIndex))
        return SnapStatus::NotSnappable;

    if (SnapMode::Origin == context->GetSnapMode ())
        {
        DPoint3d        hitPoint;
        SnapPathP       snap = context->GetSnapPath ();
        ElementHandle   elHandle (snap->GetPathElem (snapPathIndex));

        GetRangeCenter (elHandle, hitPoint);

        context->ElmLocalToWorld (hitPoint);
        context->SetSnapInfo (snapPathIndex, SnapMode::Origin, context->GetSnapSprite (SnapMode::Origin), hitPoint, true, false);

        return SnapStatus::Success;
        }

    return context->DoDefaultDisplayableSnap (snapPathIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            MeshHeaderHandler::_GetOrientation (ElementHandleCR thisElm, RotMatrixR rMatrix)
    {
    // Need something quick/efficient (no edP/allocated arrays, etc)...
    for (ChildElemIter tmpIter (thisElm, ExposeChildrenReason::Count); tmpIter.IsValid (); tmpIter = tmpIter.ToNext ())
        {
        if (MATRIX_HEADER_ELM != tmpIter.GetLegacyType() || MESH_ELM_TAG_VERTEX_COORDINATES != tmpIter.GetElementCP ()->ToMatrix_header().tag)
            continue;

        // NOTE: Can't use ChildElemIter to get to data element...no handler for matrix elm!
        EditElementHandle  dataIter;

        if (tmpIter.PeekElementDescrCP ())
            {
            dataIter.SetElementDescr (tmpIter.PeekElementDescrCP()->Components().begin()->get(), false);
            }
        else
            {
            ElementRefP child = NULL;
            SubElementRefVecP subElements = tmpIter.GetElementRef()->GetSubElements();
            if (NULL != subElements && subElements->size() > 0)
                child = subElements->at(0);

            dataIter.SetElementRef (child);
            }

        if (!dataIter.IsValid () || MATRIX_DOUBLE_DATA_ELM != dataIter.GetElementCP ()->GetLegacyType() || dataIter.GetElementCP ()->ToMatrix_double_data().numValue < 9)
            break;

        DPoint3d    pts[3];

        memcpy (pts, dataIter.GetElementCP ()->ToMatrix_double_data().data, sizeof (pts));

        DVec3d      xVector, yVector;

        xVector.differenceOf (&pts[1], &pts[0]);
        yVector.differenceOf (&pts[2], &pts[0]);

        rMatrix.initFrom2Vectors (&xVector, &yVector);
        rMatrix.squareAndNormalizeColumns (&rMatrix, 0, 1);

        return;
        }

    T_Super::_GetOrientation (thisElm, rMatrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            MeshHeaderHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // Pre-transform to "flatten" element and known linkages...
    TransformInfo   tInfo (flattenTrans);

    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     MinimalValidate (PolyfaceHeaderPtr meshData)
    {
    BlockedVector <DPoint3d> &point = meshData->Point ();
    BlockedVector <int>      &pointIndex = meshData->PointIndex ();
    size_t numPoint = point.size ();
    size_t numIndex = pointIndex.size ();

    if (!point.Active ())
        return false;

    if (pointIndex.Active ())
        {
        // Assert.. all (signed, one based !!!) indice are within range.
        for (size_t i = 0; i < numIndex; i++)
            {
            int k = pointIndex[i];
            if (abs (k) > static_cast<int>(numPoint))
                return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MeshHeaderHandler::PolyfaceFromElement (PolyfaceHeaderPtr &meshData, ElementHandleCR eh)
    {
    UInt32      meshStyle = 0;

    meshData = NULL;

    if (!MeshHeaderUtils::Read (eh, meshStyle))
        return ERROR;

    PolyfaceHeaderPtr   myMesh = PolyfaceHeader::New();
    BlockedVectorInt faceToColorIndex;
    BlockedVectorInt vertexToColorIndex;

    myMesh->SetTwoSided (true);

    myMesh->SetMeshStyle (meshStyle);
    if (meshStyle == MESH_ELM_STYLE_QUAD_GRID
        || meshStyle == MESH_ELM_STYLE_COORDINATE_QUADS)
        myMesh->SetNumPerFace (4);
    else if (meshStyle == MESH_ELM_STYLE_TRIANGLE_GRID
        || meshStyle == MESH_ELM_STYLE_COORDINATE_TRIANGLES)
        myMesh->SetNumPerFace (3);
    else
        myMesh->SetNumPerFace (0);  // Can be changed if point index shows blocking.
        
    WChar illuminatedTexture[MAXFILELENGTH];

    if (SUCCESS == LinkageUtil::ExtractNamedStringLinkageByIndex (illuminatedTexture, MAXFILELENGTH, STRING_LINKAGE_KEY_IlluminatedMesh, 0, eh.GetElementCP ()))
        myMesh->SetIlluminationName (illuminatedTexture);

    for (ChildElemIter childEh (eh, ExposeChildrenReason::Count); childEh.IsValid(); childEh = childEh.ToNext())
        {
        UInt32  numPerStruct, numPerRow, tag, indexFamily, indexedBy;

        if (MatrixHeaderUtils::Read (childEh, numPerStruct, numPerRow, tag, indexFamily, indexedBy))
            {
            if (tag == MESH_ELM_TAG_VERTEX_COORDINATES && 
                numPerStruct == 3 && 
                MatrixHeaderUtils::Read (childEh, myMesh->Point()))
                {
                myMesh->Point().SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                myMesh->SetNumPerRow (numPerRow);
                }
            else if (tag == MESH_ELM_TAG_NORMAL_COORDINATES && 
                     numPerStruct == 3 && 
                     MatrixHeaderUtils::Read (childEh, myMesh->Normal()))
                {
                myMesh->Normal().SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                }
            else if (tag == MESH_ELM_TAG_UV_PARAMETERS && 
                     numPerStruct == 2 && 
                     MatrixHeaderUtils::Read (childEh, myMesh->Param()))
                {
                myMesh->Param().SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                }
            else if (tag == MESH_ELM_TAG_INT_COLOR && 
                     numPerStruct == 3 && 
                     MatrixHeaderUtils::Read (childEh, myMesh->IntColor()))
                {
                myMesh->IntColor().SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                }
            else if (tag == MESH_ELM_TAG_DOUBLE_COLOR && 
                     numPerStruct == 3 && 
                     MatrixHeaderUtils::Read (childEh, myMesh->DoubleColor()))
                {
                myMesh->DoubleColor().SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                }
            else if (tag == MESH_ELM_TAG_TABLE_COLOR && 
                     MatrixHeaderUtils::Read (childEh, myMesh->ColorTable()))
                {
                myMesh->ColorTable().SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                }
            else if (tag == MESH_ELM_TAG_FACE_LOOP_TO_VERTEX_INDICES && 
                     numPerStruct == 1 && 
                     MatrixHeaderUtils::Read (childEh, myMesh->PointIndex()))
                {
                myMesh->PointIndex().SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                myMesh->SetNumPerFace (numPerRow);
                }
            else if (tag == MESH_ELM_TAG_FACE_LOOP_TO_NORMAL_INDICES && 
                     numPerStruct == 1 && 
                     MatrixHeaderUtils::Read (childEh, myMesh->NormalIndex()))
                {
                myMesh->NormalIndex().SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                }
            else if (tag == MESH_ELM_TAG_FACE_LOOP_TO_UV_PARAMETER_INDICES && 
                     numPerStruct == 1 && 
                     MatrixHeaderUtils::Read (childEh, myMesh->ParamIndex()))
                {
                myMesh->ParamIndex().SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                }
            else if ((tag == MESH_ELM_TAG_FACE_LOOP_TO_TABLE_COLOR_INDICES ||
                      tag == MESH_ELM_TAG_FACE_LOOP_TO_DOUBLE_COLOR_INDICES ||
                      tag == MESH_ELM_TAG_FACE_LOOP_TO_INT_COLOR_INDICES) &&
                     numPerStruct == 1 && 
                     MatrixHeaderUtils::Read (childEh, myMesh->ColorIndex()))
                {
                myMesh->ColorIndex().SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                }
            else if (tag == MESH_ELM_TAG_FACE_TO_TABLE_COLOR_INDICES && 
                     numPerStruct == 1 && 
                     MatrixHeaderUtils::Read (childEh, faceToColorIndex))
                {
                faceToColorIndex.SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                }
            else if (tag == MESH_ELM_TAG_VERTEX_TO_TABLE_COLOR_INDICES && 
                     numPerStruct == 1 && 
                     MatrixHeaderUtils::Read (childEh, vertexToColorIndex))
                {
                vertexToColorIndex.SetTags (numPerStruct, numPerRow, tag, indexFamily, indexedBy, true);
                }
            }
        }

    if (faceToColorIndex.Active ())
        myMesh->ConvertTableColorToColorIndices (&faceToColorIndex, NULL);
    if (vertexToColorIndex.Active ())
        myMesh->ConvertTableColorToColorIndices (&vertexToColorIndex, NULL);

    if (!MinimalValidate (myMesh))
        return ERROR;

    meshData = myMesh;

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MeshHeaderHandler::CreateMeshElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh,
PolyfaceQueryCR     meshData,
bool                is3d,
DgnModelR        modelRef
)
    {
    DgnV8ElementBlank   element;

    // PolyfaceQuery is always indexed !?!?
    UInt32 numPerFace = meshData.GetNumPerFace ();
    int     numPerRow = (int)meshData.GetNumPerRow();
    int     meshStyle = (int)meshData.GetMeshStyle ();
    int structsPerRow = 1;
    if (meshData.GetPointIndexCount () > 0)
        {
        meshStyle = MESH_ELM_STYLE_INDEXED_FACE_LOOPS;
        }
    else if (meshStyle == MESH_ELM_STYLE_QUAD_GRID
             || meshStyle == MESH_ELM_STYLE_TRIANGLE_GRID
             )
        {
        structsPerRow = numPerRow;
        numPerFace =
            MESH_ELM_STYLE_QUAD_GRID == meshStyle ? 4 : 3;
        }
    else
        {
        if (numPerFace == 3)
            meshStyle = MESH_ELM_STYLE_COORDINATE_TRIANGLES;
        else if (numPerFace == 4)
            meshStyle = MESH_ELM_STYLE_COORDINATE_QUADS;
        else
            return ERROR;
        }
    MeshHeaderUtils::CreateMeshHeader (element, templateEh, meshStyle, is3d, modelRef);
    
    MSElementDescrPtr header = MSElementDescr::Allocate (element, modelRef);
    MSElementDescrPtr matrixDescr;

    UInt32  effectiveNumPerFace;
    int     blockType;

    if (numPerFace <= 1)
        {
        blockType = MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_VARIABLE;
        effectiveNumPerFace = 1;
        }
    else
        {
        blockType = MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_FIXED;
        effectiveNumPerFace = numPerFace;
        }

    int pointTransformType, normalTransformType, paramTransformType, colorTransformType;

    MatrixHeaderUtils::EncodeTransformType (&pointTransformType,  MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_POINT,    3, false);
    MatrixHeaderUtils::EncodeTransformType (&normalTransformType, MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_COVECTOR, 3, false);
    MatrixHeaderUtils::EncodeTransformType (&colorTransformType,  MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE,     3, false);
    MatrixHeaderUtils::EncodeTransformType (&paramTransformType,  MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE,     2, false);

    int pointIndexType, normalIndexType, paramIndexType, colorIndexType;

    MatrixHeaderUtils::EncodeIndexType (&pointIndexType,  blockType, 1, true, true);
    MatrixHeaderUtils::EncodeIndexType (&normalIndexType, blockType, 1, true, true);
    MatrixHeaderUtils::EncodeIndexType (&paramIndexType,  blockType, 1, true, true);
    MatrixHeaderUtils::EncodeIndexType (&colorIndexType,  blockType, 1, true, true);

    size_t numIndex = meshData.GetPointIndexCount ();  // This is the count for ALL active indices -- point, normal, uv, color.
    if (meshData.GetPointCount () > 0)
        {
        matrixDescr = MatrixHeaderUtils::CreateDoubleDataChain (templateEh,
                        (double*)meshData.GetPointCP (),
                        meshData.GetPointCount (), // numStructs
                        3,  // numPerStruct
                        structsPerRow,      // effectiveNumPerFace,  // structsPerRow
                        MESH_ELM_TAG_VERTEX_COORDINATES,    // tag
                        MESH_ELM_INDEX_FAMILY_NONE,  // index type
                        MESH_ELM_TAG_FACE_LOOP_TO_VERTEX_INDICES, // indexed by
                        pointTransformType,
                        modelRef);
        header->AddComponent(*matrixDescr.get());
        }

    if (meshData.GetPointIndexCP () != NULL &&
        numIndex > 0)
        {
        matrixDescr = MatrixHeaderUtils::CreateIntDataChain (templateEh,
                        meshData.GetPointIndexCP (),
                        meshData.GetPointIndexCount (), // numStructs
                        1,  // numPerStruct
                        effectiveNumPerFace,  // structsPerRow
                        MESH_ELM_TAG_FACE_LOOP_TO_VERTEX_INDICES,    // tag
                        MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP,  // index type
                        MESH_ELM_INDEX_FAMILY_NONE, // indexed by
                        pointIndexType,
                        modelRef);
        header->AddComponent(*matrixDescr.get());
        }

    if (meshData.GetNormalCount () > 0)
        {
        matrixDescr = MatrixHeaderUtils::CreateDoubleDataChain (templateEh,
                        (double*)meshData.GetNormalCP (),
                        meshData.GetNormalCount (), // numStructs
                        3,  // numPerStruct
                        structsPerRow,
                        MESH_ELM_TAG_NORMAL_COORDINATES,    // tag
                        MESH_ELM_INDEX_FAMILY_NONE,  // index type
                        MESH_ELM_TAG_FACE_LOOP_TO_NORMAL_INDICES, // indexed by
                        normalTransformType,
                        modelRef);
        header->AddComponent(*matrixDescr.get());
        }

    if (meshData.GetNormalIndexCP () != NULL && numIndex > 0)
        {
        matrixDescr = MatrixHeaderUtils::CreateIntDataChain (templateEh,
                        meshData.GetNormalIndexCP (),
                        numIndex, // numStructs
                        1,  // numPerStruct
                        effectiveNumPerFace,  // structsPerRow
                        MESH_ELM_TAG_FACE_LOOP_TO_NORMAL_INDICES,    // tag
                        MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP,  // index type
                        MESH_ELM_INDEX_FAMILY_NONE, // indexed by
                        pointIndexType,
                        modelRef);
        header->AddComponent(*matrixDescr.get());
        }

    if (meshData.GetParamCP () != NULL &&
        meshData.GetParamCount () > 0)
        {
        matrixDescr = MatrixHeaderUtils::CreateDoubleDataChain (templateEh,
                        (double*)meshData.GetParamCP (),
                        meshData.GetParamCount (), // numStructs
                        2,  // numPerStruct
                        structsPerRow,
                        MESH_ELM_TAG_UV_PARAMETERS,    // tag
                        MESH_ELM_INDEX_FAMILY_NONE,  // index type
                        MESH_ELM_TAG_FACE_LOOP_TO_UV_PARAMETER_INDICES, // indexed by
                        paramTransformType,
                        modelRef);
        header->AddComponent(*matrixDescr.get());
        }

    if (meshData.GetParamIndexCP () != NULL && numIndex > 0)
        {
        matrixDescr = MatrixHeaderUtils::CreateIntDataChain (templateEh,
                        meshData.GetParamIndexCP (),
                        numIndex,
                        1,  // numPerStruct
                        effectiveNumPerFace,  // structsPerRow
                        MESH_ELM_TAG_FACE_LOOP_TO_UV_PARAMETER_INDICES,    // tag
                        MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP,  // index type
                        MESH_ELM_INDEX_FAMILY_NONE, // indexed by
                        paramIndexType,
                        modelRef);
        header->AddComponent(*matrixDescr.get());
        }

    if (meshData.GetDoubleColorCP () != NULL &&
        meshData.GetColorCount () > 0)
        {
        matrixDescr = MatrixHeaderUtils::CreateDoubleDataChain (templateEh,
                        (double*)meshData.GetDoubleColorCP (),
                        meshData.GetColorCount (), // numStructs
                        3,  // numPerStruct
                        structsPerRow,
                        MESH_ELM_TAG_DOUBLE_COLOR,    // tag
                        MESH_ELM_INDEX_FAMILY_NONE,  // index type
                        MESH_ELM_TAG_FACE_LOOP_TO_DOUBLE_COLOR_INDICES, // indexed by
                        colorTransformType,
                        modelRef);
        header->AddComponent(*matrixDescr.get());
        }

    if (meshData.GetIntColorCP () != NULL
        && meshData.GetColorCount () > 0)
        {
        matrixDescr = MatrixHeaderUtils::CreateIntDataChain (templateEh,
                        (const int*)meshData.GetIntColorCP (),
                        meshData.GetColorCount (), // numStructs
                        1,  // numPerStruct
                        1,  // structsPerRow
                        MESH_ELM_TAG_INT_COLOR,    // tag
                        MESH_ELM_INDEX_FAMILY_NONE,  // index type
                        MESH_ELM_TAG_FACE_LOOP_TO_INT_COLOR_INDICES, // indexed by
                        colorTransformType,
                        modelRef);
        header->AddComponent(*matrixDescr.get());
        }

    if (meshData.GetColorTableCP () != NULL &&
        meshData.GetColorCount () > 0)
        {
        matrixDescr = MatrixHeaderUtils::CreateIntDataChain (templateEh,
                        (const int *)meshData.GetColorTableCP (),
                        meshData.GetColorCount (), // numStructs
                        1,  // numPerStruct
                        structsPerRow,
                        MESH_ELM_TAG_TABLE_COLOR,    // tag
                        MESH_ELM_INDEX_FAMILY_NONE,  // index type
                        MESH_ELM_TAG_FACE_LOOP_TO_TABLE_COLOR_INDICES, // indexed by
                        colorTransformType,
                        modelRef);
        header->AddComponent(*matrixDescr.get());
        }

    if (meshData.GetColorIndexCP () != NULL &&
        numIndex > 0)
        {
        matrixDescr = MatrixHeaderUtils::CreateIntDataChain (templateEh,
                        meshData.GetColorIndexCP (),
                        numIndex,
                        1,  // numPerStruct
                        effectiveNumPerFace,  // structsPerRow
                        MESH_ELM_TAG_FACE_LOOP_TO_DOUBLE_COLOR_INDICES,    // tag
                        MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP,  // index type
                        MESH_ELM_INDEX_FAMILY_NONE, // indexed by
                        colorIndexType,
                        modelRef);
        header->AddComponent(*matrixDescr.get());
        }

    eeh.SetElementDescr (header.get(), false);
    
    return eeh.GetDisplayHandler ()->ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MeshHeaderHandler::_GetMeshData (ElementHandleCR source, PolyfaceHeaderPtr& meshData)
    {
    return MeshHeaderHandler::PolyfaceFromElement (meshData, source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MeshHeaderHandler::_SetMeshData (EditElementHandleR eeh, PolyfaceQueryR meshData)
    {
    EditElementHandle  newEeh;

    // NOTE: In case eeh is component use ReplaceElementDescr, Create methods uses SetElementDescr...
    if (SUCCESS != MeshHeaderHandler::CreateMeshElement (newEeh, &eeh, meshData, eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ()))
        return ERROR;

    return (BentleyStatus) eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt mesh_indexPositionToFaceRange (PolyfaceHeaderP meshData, int& index0, int& index1, int directIndex)
    {
    int         arrayCount = (int) meshData->GetPointIndexCount ();
    Int32 const*      pIndexBuffer = meshData->GetPointIndexCP ();
    UInt32      numPerFace = meshData->GetNumPerFace ();
    StatusInt   status = ERROR;

    index0 = index1 = -1;

    if (directIndex < 0 || directIndex > arrayCount)
        return ERROR;

    if (numPerFace > 1)
        {
        int     faceIndex = directIndex / numPerFace;

        index0 = faceIndex * numPerFace;
        index1 = index0 + numPerFace - 1; // just before next face

        // advance over (misplaced) padding to first index
        while (index0 < index1 && pIndexBuffer[index0] == 0/*s_termPad*/)
            index0++;

        // backtrack over padding to last index
        while (index1 > index0 && pIndexBuffer[index1] == 0/*s_termPad*/)
            index1--;
        }
    else
        {
        // backtrack over terminator(s) so that directIndex is in actual face range
        while (directIndex > 0 && pIndexBuffer[directIndex] == 0/*s_termPad*/)
            directIndex--;

        index0 = index1 = directIndex;

        // backtrack over indices to first index
        while (index0 > 0 && pIndexBuffer[index0 - 1] != 0/*s_termPad*/)
            index0--;

        // advance over indices to last index
        while (index1 + 1 < arrayCount && pIndexBuffer[index1 + 1] != 0/*s_termPad*/)
            index1++;
        }

    if (index0 < index1)
        status = SUCCESS;
    else
        index1 = index0;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   mesh_getEdgeIndex (PolyfaceHeaderP meshData, int& edgeIndex, int vertexIndex0, int vertexIndex1)
    {
    if (vertexIndex0 < 0 || vertexIndex1 < 0 || vertexIndex0 == vertexIndex1)
        return ERROR;

    bool        eFwdFlag, eRevFlag;
    int         i0, i1, f0, f1, eFwd, eRev; // 0-based indices into polyface index array
    int         vIndex0, vIndex1;           // 1-based signed indices into polyface vertex array
    int         nIndex = (int) meshData->GetPointIndexCount ();
    Int32 const*      pIndexBuffer = meshData->GetPointIndexCP ();

    vertexIndex0++;
    vertexIndex1++;

    eFwd = eRev = -1;
    eFwdFlag = eRevFlag = false;

    // look for first 1-based entry pairs (v0,v1) and (v1,v0) in the index array
    for (i0 = 0; i0 < nIndex; i0++)
        {
        vIndex0 = *(pIndexBuffer + i0);

        // get containing face loop if entry matches first vertex index
        if (vIndex0 && (vertexIndex0 == vIndex0 || vertexIndex0 == -vIndex0) &&
            SUCCESS == mesh_indexPositionToFaceRange (meshData, f0, f1, i0))
            {
            // get next vertex index in face loop
            i1 = (i0 < f1) ? i0 + 1 : f0;
            vIndex1 = *(pIndexBuffer + i1);

            // does next vertex index match?
            if (vertexIndex1 == vIndex1 || vertexIndex1 == -vIndex1)
                {
                eFwd = i0;
                eFwdFlag = (vIndex0 > 0);
                break;
                }

            // check for reverse orientation, if haven't found one yet
            if (eRev < 0)
                {
                // get prev vertex index in face loop
                i1 = (i0 > f0) ? i0 - 1 : f1;
                vIndex1 = *(pIndexBuffer + i1);

                // does prev vertex index match?
                if (vertexIndex1 == vIndex1 || vertexIndex1 == -vIndex1)
                    {
                    eRev = i1;
                    eRevFlag = (vIndex1 > 0);
                    }
                }
            }
        }

    // did we find a forward edge (or failing that, a reversed edge)?
    if (eFwd >= 0 || eRev >= 0)
        {
        edgeIndex = (eFwd >= 0) ? eFwd : eRev;

        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MeshHeaderHandler::CreateMeshAssoc
(
ElementHandleCR eh,
AssocPoint&     assoc,
SnapPathCR      snapPath,
int             modifierMask,           /* => allowed association type mask */
bool            createFarPathElems,
DgnModelP    parentModel
)
    {
    DSegment3d    segment;
    GeomDetailCR  detail = snapPath.GetGeomDetail ();

    if (!detail.GetSegment (segment))
        return ERROR;

    ICurvePrimitiveInfoPtr const& curveInfo = detail.GetCurvePrimitive ()->GetCurvePrimitiveInfo ();

    if (!curveInfo.IsValid ())
        return ERROR;

    MeshSegmentInfo* meshSegment;

    if (NULL == (meshSegment = dynamic_cast <MeshSegmentInfo*> (curveInfo.get ())))
        return ERROR;

    int         closeVertexIndex = meshSegment->m_closeVertex;
    int         segmentVertexIndex = meshSegment->m_segmentVertex;
    double      edgeParam = detail.GetCloseParam ();

    if (edgeParam < 0.5 && bsiDPoint3d_pointEqual (&snapPath.GetSnapPoint (), &segment.point[0]))
        {
        if (0 == (MESH_VERTEX_ASSOC & ~modifierMask))
            return ERROR;

        AssociativePoint::InitMeshVertex (assoc, closeVertexIndex, 0);
        }
    else if (edgeParam > 0.5 && bsiDPoint3d_pointEqual (&snapPath.GetSnapPoint (), &segment.point[1]))
        {
        if (0 == (MESH_VERTEX_ASSOC & ~modifierMask))
            return ERROR;

        AssociativePoint::InitMeshVertex (assoc, closeVertexIndex, 0);
        }
    else
        {
        if (0 == (MESH_EDGE_ASSOC & ~modifierMask))
            return ERROR;

        IMeshQuery* meshQuery;

        if (NULL == (meshQuery = dynamic_cast <IMeshQuery*> (&eh.GetHandler ())))
            return ERROR;

        PolyfaceHeaderPtr meshData;

        if (SUCCESS != meshQuery->GetMeshData (eh, meshData))
            return ERROR;

        int     edgeIndex = 0;

        if (SUCCESS != mesh_getEdgeIndex (meshData.get (), edgeIndex, closeVertexIndex, segmentVertexIndex))
            return ERROR;

        double  dDivisor  = (double) snapPath.GetSnapDivisor ();
        double  snapParam = int (edgeParam * dDivisor + 0.5) / dDivisor;

        AssociativePoint::InitMeshEdge (assoc, edgeIndex, 0, snapParam);
        }

    if (SUCCESS != AssociativePoint::SetRoot (assoc, &snapPath, parentModel, createFarPathElems))
        return ERROR;

    return AssociativePoint::IsValid (assoc, snapPath.GetEffectiveRoot (), parentModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MeshHeaderHandler::EvaluateMeshAssoc (ElementHandleCR eh, AssocPoint& assocPoint, DPoint3dR outPoint)
    {
    PolyfaceHeaderPtr meshData;

    if (SUCCESS != MeshHeaderHandler::PolyfaceFromElement (meshData, eh))
        return ERROR;

    AssocGeom*  assocP = (AssocGeom *) &assocPoint;

    if (MESH_VERTEX_ASSOC == assocP->type)
        {
        outPoint = *(meshData->GetPointCP () + assocP->meshVertex.vertexIndex);

        return SUCCESS;
        }

    if (MESH_EDGE_ASSOC != assocP->type)
        return ERROR;

    int     faceIndex0, faceIndex1;

    if (SUCCESS != mesh_indexPositionToFaceRange (meshData.get (), faceIndex0, faceIndex1, assocP->meshEdge.edgeIndex))
        return ERROR;

    int     i0, i1;
    int     signedVertexIndex0, signedVertexIndex1;
    int     vertexIndex0, vertexIndex1;

    i0 = assocP->meshEdge.edgeIndex;
    i1 = (int) assocP->meshEdge.edgeIndex < faceIndex1 ? assocP->meshEdge.edgeIndex + 1 : faceIndex0;

    signedVertexIndex0 = *(meshData->GetPointIndexCP () + i0);
    signedVertexIndex1 = *(meshData->GetPointIndexCP () + i1);

    vertexIndex0 = abs (signedVertexIndex0) - 1;
    vertexIndex1 = abs (signedVertexIndex1) - 1;

    if (vertexIndex0 < 0 || vertexIndex1 < 0)
        return ERROR;

    DPoint3d    xyz0, xyz1;

    xyz0 = *(meshData->GetPointCP () + vertexIndex0);
    xyz1 = *(meshData->GetPointCP () + vertexIndex1);

    outPoint.interpolate (&xyz0, assocP->meshEdge.uParam, &xyz1);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Evan.Williams                  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MatrixDoubleDataHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_MATRIX_DOUBLE_DATA_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MatrixDoubleDataHandler::_OnTransform
(
EditElementHandleR elemHandle,
TransformInfoCR trans
)
    {
    MSElementDescrP edP = elemHandle.GetElementDescrP ();

    return MatrixHeaderUtils::TransformDoubleDataElementInPlace (&edP->ElementR(), trans.GetTransform ());
    }
