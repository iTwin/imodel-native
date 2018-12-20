/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsPointComponent.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#include    <DgnPlatform/LsLocal.h>

#define LCPOINT_ANYVERTEX   (LCPOINT_LINEORG | LCPOINT_LINEEND | LCPOINT_LINEVERT)

/*---------------------------------------------------------------------------------**//**
* Stroke a LsPointComponent by processing the underlying stroke component and calling the "_ProcessSymbol" method at
* every place along the input line where a symbol should appear. Afterwards, process the symbols that appear at vertices.
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsPointComponent::_DoStroke (LineStyleContextR context, DPoint3dCP inPoints, int nPoints, LineStyleSymbR modifiers) const
    {
    if (NULL == m_strokeComponent.get ())
        return  SUCCESS;

    if (HasStrokeSymbol() && (SUCCESS != m_strokeComponent->ProcessStroke (context, this, inPoints, nPoints, modifiers)))
        return  ERROR;

    DPoint3dCP  org   = inPoints;
    DPoint3dCP  end   = org + (nPoints-1);

    // process vertex symbols
    LsSymbolReferenceCP   sym;
    DPoint3d     dir, dir1, dir2;

    if (!modifiers.ContinuationXElems() && NULL != (sym = GetSymbolForVertexCP (LsSymbolReference::VERTEX_LineOrigin)))
        {
        dir.NormalizedDifference (org[1], *org);

        if (SUCCESS != sym->Output (context, &modifiers, org, &dir))
            return  ERROR;
        }

    if (NULL != (sym = GetSymbolForVertexCP (LsSymbolReference::VERTEX_LineEnd)))
        {
        dir.NormalizedDifference (*end, *(end-1));
        if (SUCCESS != sym->Output (context, &modifiers, end, &dir))
            return  ERROR;
        }

    // If treat as single segment is set we are on a curve or arc - in which case we can't support vertex symbols either
    if (modifiers.IsTreatAsSingleSegment())
        return  SUCCESS;

    if (NULL != (sym = GetSymbolForVertexCP (LsSymbolReference::VERTEX_Each)))
        {
        for (org++ ;org < end; org++)
            {
            dir1.NormalizedDifference (org[1], *org);
            dir2.NormalizedDifference (*org, *(org-1));

            dir.Interpolate (dir1, 0.5, dir2);
            dir.Normalize ();

            if (SUCCESS != sym->Output (context, &modifiers, org, &dir))
                return  ERROR;
            }
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsPointComponent::HasStrokeSymbol () const
    {
    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if (-1 != curr->m_strokeNo)
            return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceP LsPointComponent::GetSymbolReferenceP
(
T_SymbolsCollectionConstIter iter
) const
    {
    LsSymbolReferenceCP lsSym = &*iter;
    return const_cast <LsSymbolReferenceP> (lsSym);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceP LsPointComponent::GetSymbolP
(
size_t           index
) const
    {
    if (index >= m_symbols.size ())
        return NULL;
        
    T_SymbolsCollectionConstIter    entry = m_symbols.begin () + index;
    return GetSymbolReferenceP (entry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceCP LsPointComponent::GetSymbolCP
(
size_t           index
) const
    {
    return GetSymbolP (index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceP LsPointComponent::GetSymbolForStrokeP
(
int             strokeNo
) const
    {
    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if ((0 == (curr->m_mod1 & LCPOINT_ANYVERTEX)) && (curr->m_mod1 & LCPOINT_ONSTROKE) && (curr->m_strokeNo == strokeNo))
            return GetSymbolReferenceP (curr);
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceCP LsPointComponent::GetSymbolForStrokeCP
(
int             strokeNo
) const
    {
    return GetSymbolForStrokeP (strokeNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceP LsPointComponent::GetSymbolForVertexP (LsSymbolReference::VertexMask vertexMask) const
    {
    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if (0 != (curr->m_mod1 & vertexMask))
            return GetSymbolReferenceP (curr);
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceCP LsPointComponent::GetSymbolForVertexCP (LsSymbolReference::VertexMask vertexMask) const
    {
    return GetSymbolForVertexP (vertexMask);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool LsPointComponent::_ProcessSymbol(LineStyleContextR context, Centerline const* centerline, LineStyleSymbCP modifiers, LsStrokeCP pStroke, int strokeIndex, int endCondition) const
    {
    LsSymbolReferenceCP    symRef = GetSymbolForStrokeCP (strokeIndex);

    if (NULL == symRef)
        return  false;

    double  outStrokeLen = pStroke->GetLength();
    DPoint3dCP const firstPoint = centerline->GetPointAt(0);
    DPoint3dCP const lastPoint  = firstPoint + (centerline->GetCount() - 1);
    double   const* const firstLen   = centerline->GetLengthAt(0);
    double   const* const lastLen    = firstLen + (centerline->GetCount() - 1);

    /*-----------------------------------------------------------------------------------
    If this symbol reference doesn't allow partial strokes then short circuit early

    The check for IsRigid is a throwback.  It might have been a bug in earlier versions
    that you didn't have to set the partial items correctly on the point symbol, or it
    might be a necessary default for strokes that bypass corners.  Either way it is something
    we must continue to support.
    -----------------------------------------------------------------------------------*/
    if (symRef->GetNoPartial() && !pStroke->IsRigid () && ((*firstLen > 0) || (*lastLen < outStrokeLen)))
        return  false;

    DPoint3d    segOrg, segEnd, segDir, symOrg;
    bool        checkOrigin = !symRef->GetDgnDb();

    switch (symRef->GetJustification())
        {
        case LCPOINT_ORIGIN:
            if (checkOrigin && (*firstLen > 0))
                return  false;

            segOrg = *firstPoint;
            segEnd = centerline->GetCount() == 1 ? *(firstPoint) : *(firstPoint+1);
            centerline->GetDirectionVector (segDir, segOrg, segEnd);
            symOrg.SumOf (segOrg,segDir, - *firstLen);
            break;

        case LCPOINT_CENTER:
            {
            double  centerOffset = outStrokeLen / 2.0;

            if (checkOrigin && (centerOffset >= *lastLen || centerOffset <= *firstLen))
                return  false;

            DPoint3dCP pPoint = firstPoint;
            double const*   pLen   = firstLen;
            while (pPoint < lastPoint && centerOffset > *pLen)
                {
                pPoint++;
                pLen++;
                }

            if (pPoint == firstPoint)
                {   // Symbol is before the segment origin
                segOrg = *pPoint;
                segEnd = centerline->GetCount() == 1 ? *(pPoint) : *(pPoint + 1);
                centerline->GetDirectionVector (segDir, segOrg, segEnd);
                symOrg.SumOf (segOrg,segDir, centerOffset - *pLen);
                }
            else
                {   // Symbol is within the segment or past the end
                segOrg = centerline->GetCount() == 1 ? *(pPoint) : *(pPoint-1);
                segEnd = *pPoint;
                centerline->GetDirectionVector (segDir, segOrg, segEnd);
                symOrg.SumOf (segOrg,segDir, centerOffset - *(pLen-1));
                }
            }
            break;

        case LCPOINT_END:
            if (checkOrigin && outStrokeLen > *lastLen)
                return  false;

            segOrg = centerline->GetCount() == 1 ? *(lastPoint) : *(lastPoint-1);
            segEnd = *(lastPoint);
            centerline->GetDirectionVector (segDir, segOrg, segEnd);
            symOrg.SumOf (segEnd,segDir, outStrokeLen - *lastLen);
            break;

        default:
            return  false;
        }

    /*-----------------------------------------------------------------------------------
    Setup clipping points if clip is enabled. Note: segOrg and segEnd (calculated above)
    are not necessarily the endpoints of the stroke so they aren't used as the clip points.

    See comment about IsRigid above.
    -----------------------------------------------------------------------------------*/
    DPoint3d    *pOrgClip=NULL, *pEndClip=NULL;
    DPoint3d    strokeOrg, strokeEnd;

    if (symRef->GetClipPartial() || pStroke->IsRigid ())
        {
        if (*firstLen > 0)
            {
            strokeOrg = *firstPoint;
            pOrgClip = &strokeOrg;
            }

        if (*lastLen < outStrokeLen)
            {
            strokeEnd = *lastPoint;
            pEndClip = &strokeEnd;
            }
        }

    double xScale = 1.0;
    /*-----------------------------------------------------------------------------------
    If this symbol allows stretch and the underlying stroke has been stretched then
    the stretch has to be applied to the x bvector of the transform.
    -----------------------------------------------------------------------------------*/
    if (symRef->GetStretchable())
        {
        double baseLen =  m_strokeComponent->GetConstStrokePtr(strokeIndex)->GetLength() * modifiers->GetScale();

        if ((outStrokeLen != baseLen) && baseLen > 0)
            xScale = outStrokeLen / baseLen;
        }

    symRef->Output (context, modifiers, &symOrg, &segDir, &xScale, pOrgClip, pEndClip);
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsPointComponent::_PostProcessLoad ()
    {
    if (m_postProcessed)
        return;
        
    m_postProcessed = true;
    if (NULL != m_strokeComponent.get ())
        m_strokeComponent->_PostProcessLoad ();
        
    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if (NULL != curr->m_symbol.get ())
            curr->m_symbol->_PostProcessLoad ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsPointComponent::_ClearPostProcess ()
    {
    m_postProcessed = false;
    if (NULL != m_strokeComponent.get ())
        m_strokeComponent->_ClearPostProcess ();
        
    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if (NULL != curr->m_symbol.get ())
            curr->m_symbol->_ClearPostProcess ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
void LsPointComponent::_StartTextureGeneration() const
    {
    m_okayForTextureGeneration = Dgn::LsOkayForTextureGeneration::Unknown;
    if (m_strokeComponent.IsValid())
        m_strokeComponent->_StartTextureGeneration();

    for (LsSymbolReference const& symref : m_symbols)
        symref.m_symbol->_StartTextureGeneration();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
LsPointComponent::LsPointComponent(LsPointComponentCR source, bool removeSymbrefOnVertex) : LsComponent(source)
    {
    m_strokeComponent = source.m_strokeComponent;
    m_okayForTextureGeneration = source.m_okayForTextureGeneration;
    m_postProcessed = false;
    for (LsSymbolReference const& ref: source.m_symbols)
        {
        if (!removeSymbrefOnVertex || ref.GetVertexMask() == LsSymbolReference::VERTEX_None)
            m_symbols.push_back(ref);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsPointComponent::_GetForTextureGeneration() const
    {
    if (m_okayForTextureGeneration == Dgn::LsOkayForTextureGeneration::NoChangeRequired)
        return const_cast<LsPointComponentP>(this);

    BeAssert(m_okayForTextureGeneration != Dgn::LsOkayForTextureGeneration::NotAllowed);  //  the caller must check for this

    LsPointComponentP retval = new LsPointComponent(*this, true);
    retval->m_okayForTextureGeneration = LsOkayForTextureGeneration::NoChangeRequired;

    LsComponentPtr strokeComp = m_strokeComponent->_GetForTextureGeneration();
    retval->m_strokeComponent = dynamic_cast<LsStrokePatternComponentP>(strokeComp.get());

    for (LsSymbolReference& symbref : retval->m_symbols)
        {
        symbref.m_parent = const_cast<LsPointComponentP>(this);
        LsComponentPtr symbol = symbref.m_symbol->_GetForTextureGeneration();
        symbref.m_symbol = dynamic_cast<LsSymbolComponentP>(symbol.get());

        if (symbref.GetVertexMask() != LsSymbolReference::VERTEX_None)
            {
            BeAssert(symbref.GetVertexMask() == LsSymbolReference::VERTEX_None);    //  it should have been removed prior to this
            continue;
            }

        if (symbref.GetRotationMode() != LsSymbolReference::ROTATE_Relative)
            {
            symbref.SetRotationMode(LsSymbolReference::ROTATE_Relative);
            symbref.SetAngle(0.0);
            }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
        DRange3d  range;
        // NEEDSWORK -- linestyles -- don't rely on range from symbol when deciding if symbol will fit in stroke.
        symbref.m_symbol->_GetRange(range);
#endif
        }

    return retval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
LsOkayForTextureGeneration LsPointComponent::_IsOkayForTextureGeneration() const
    {
    if (!m_strokeComponent.IsValid())
        //  If there is not a stroke component then either part of the line style definition is missing
        //  or all symbols are associated with vertices instead of strokes. Either way, there is no
        //  way to generate a texture from this line style.
        return LsOkayForTextureGeneration::NotAllowed;

    if (m_okayForTextureGeneration != LsOkayForTextureGeneration::Unknown)
        return m_okayForTextureGeneration;

    m_okayForTextureGeneration = m_strokeComponent->_IsOkayForTextureGeneration();
    if (m_okayForTextureGeneration == LsOkayForTextureGeneration::NotAllowed)
        return m_okayForTextureGeneration;

    //  For each stroke that has a symbol we want to decide if drawing the symbol will go outside the stroke.  If so
    //  and any symbol goes outside of the full pattern we shift the symbols to try to make each symbol fall into 
    //  its stroke.
    //
    //  If any symbol has a vertex mask then it should be removed from the definition.  Some line styles are useless with
    //  these symbols removed; other you barely notice.  We may want to do line style stroking for styles that only have
    //  symbols at the beginning and end. 
    for (LsSymbolReference const& symref : m_symbols)
        {
        if ((symref.GetVertexMask() & LsSymbolReference::VERTEX_Each) != 0)
            return m_okayForTextureGeneration = LsOkayForTextureGeneration::NotAllowed;

        if (symref.GetVertexMask() != 0)
            return m_okayForTextureGeneration = LsOkayForTextureGeneration::ChangeRequired;
        }

    UpdateLsOkayForTextureGeneration(m_okayForTextureGeneration, VerifySymbols());
    return m_okayForTextureGeneration;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsPointComponent::LsPointComponent (LsLocation const *pLocation) : LsComponent (pLocation)
    {
    m_postProcessed   = false;
    m_strokeComponent = NULL;
    m_okayForTextureGeneration = LsOkayForTextureGeneration::Unknown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsPointComponent::~LsPointComponent()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsPointComponent::_ContainsComponent
(
LsComponentP    other
) const
    {
    if (LsComponent::_ContainsComponent(other))
        return  true;

    for (size_t i=0; i<GetNumberSymbols(); i++)
        {
        if (GetSymbolCP(i)->GetSymbolComponentCP()->_ContainsComponent(other))
            return true;
        }

    LsStrokePatternComponentCP stroke = GetStrokeComponentCP();
    return  stroke ? stroke->_ContainsComponent(other) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
double          LsPointComponent::_GetMaxWidth () const
    {
    double width = 0.0;

    for (size_t i=0; i<GetNumberSymbols(); i++)
        {
        double thisWidth = GetSymbolCP(i)->_GetMaxWidth();

        if (thisWidth > width)
            width = thisWidth;
        }

    return  width;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
                LsSymbolReference::LsSymbolReference ()
 : m_symbol (NULL), m_parent (NULL), m_mod1 (0), m_angle (0.0), m_strokeNo (0)
    {
    m_offset.x = 0;
    m_offset.y = 0;
    m_offset.z = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
                LsSymbolReference::LsSymbolReference 
(
LsSymbolComponentP symbol, 
LsPointComponentP parent, 
uint32_t        mod1, 
double          xOffset, 
double          yOffset, 
double          radians,
int             strokeNo
)  : m_symbol (symbol), m_parent (parent), m_mod1 (mod1), m_angle (radians), m_strokeNo (strokeNo)
    {
    m_offset.x = xOffset;
    m_offset.y = yOffset;
    m_offset.z = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsPointComponent* LsPointComponent::LoadLinePoint(LsComponentReader*reader)
    {
    Json::Value     jsonValue;
    reader->GetJsonValue(jsonValue);

    LsPointComponentP compPtr;
    LsPointComponent::CreateFromJson(&compPtr, jsonValue, reader->GetSource());

    return compPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
LsOkayForTextureGeneration LsPointComponent::VerifySymbol(double& adjustment, double startOfStroke, double patternLength, uint32_t strokeIndex) const
    {
    adjustment = 0;

    LsStrokeCP pStroke = m_strokeComponent->GetStrokeCP(strokeIndex);
    LsSymbolReferenceCP    symRef = GetSymbolForStrokeCP (strokeIndex);
    if (NULL == symRef)
        return  LsOkayForTextureGeneration::NoChangeRequired;

    LsOkayForTextureGeneration retval = LsOkayForTextureGeneration::NoChangeRequired;
    double xOrigin = 0;
    double outStrokeLen = pStroke->GetLength();

    switch (symRef->GetJustification())
        {
        case LCPOINT_ORIGIN:
            xOrigin = startOfStroke + symRef->m_offset.x;
            break;

        case LCPOINT_CENTER:
            xOrigin = startOfStroke + outStrokeLen/2 + symRef->m_offset.x;
            break;

        case LCPOINT_END:
            xOrigin = startOfStroke + outStrokeLen + symRef->m_offset.x;
            break;

        default:
            BeAssert(false && L"Unknown stroke justification");
            return  LsOkayForTextureGeneration::NotAllowed;
        }

    LsSymbolComponentCP symbolComponent = symRef->GetSymbolComponentCP();
    BeAssert(NULL != symbolComponent);
    if (NULL == symbolComponent)
        return LsOkayForTextureGeneration::NotAllowed;

    DRange3d  range3d;

    symbolComponent->GetRange(range3d);

    //  Now adjust the range.  This is simplified because we just drawing a line that has coordinates (0,0,0), (strokeLen, 0, 0).  We are assuming
    //  top view so have identity for the view transform.
    //
    //  For these tests we are just using a scale factor of 1.
    //
    //  Converting to texture always forces Relative rotation so it is okay to assume Relative here.  We should test the range of the rotated graphics but for now will just
    //  test the unrotated range of the unrotated graphics.  
    double xLow = range3d.low.x/symbolComponent->GetUnitScale() + xOrigin;
    double xHigh = range3d.high.x/symbolComponent->GetUnitScale() + xOrigin;

    if (xLow < 0)
        {
        adjustment = 0 - xLow;
        retval = LsOkayForTextureGeneration::ChangeRequired;
#if defined(NEEDSWORK_LINESTYLE)  //  How can we handle the symbol being larger than the stroke?  It is for Batten. It is easy to make this mistake and not notice it if there is a small overlap.
        if (xHigh + adjustment >= patternLength)
            retval = LsOkayForTextureGeneration::NotAllowed;
#endif
        }
    else if (xHigh >= patternLength)
        {
        adjustment = patternLength - xHigh;
        retval = LsOkayForTextureGeneration::ChangeRequired;
#if defined(NEEDSWORK_LINESTYLE)  //  How can we handle the symbol being larger than the stroke?  It is for Batten
        if (xLow + adjustment < 0)
            retval = LsOkayForTextureGeneration::NotAllowed;
#endif
        }

    return retval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
LsOkayForTextureGeneration LsPointComponent::VerifySymbols() const
    {
    LsOkayForTextureGeneration  result = LsOkayForTextureGeneration::NoChangeRequired;
    double currentOffset = 0;
    if (!m_strokeComponent.IsValid())
        return result;

    double patternLength = m_strokeComponent->GetLength(NULL);
    for (uint32_t i = 0; i < m_strokeComponent->GetNumberStrokes(); ++i)
        {
        double adjustment = 0.0;
        LsStrokeCP pStroke = m_strokeComponent->GetStrokeCP(i);
        LsOkayForTextureGeneration temp =  VerifySymbol(adjustment, currentOffset, patternLength, i);
        UpdateLsOkayForTextureGeneration(result, temp);
        currentOffset += pStroke->GetLength();
        }

    return result;
    }

size_t                       LsPointComponent::GetNumberSymbols()         const   {return m_symbols.size ();}
LsStrokePatternComponentP    LsPointComponent::GetStrokeComponentP ()     const   {return m_strokeComponent.get ();}
LsStrokePatternComponentCP    LsPointComponent::GetStrokeComponentCP ()   const   {return GetStrokeComponentP ();}
