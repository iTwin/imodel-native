/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/StrokePattern.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnCore/LsLocal.h>

static const double   s_lsTolerance      = 1.0e-7;
static const double  TOLERANCE_MitreLimit = -0.86602540378443864676372317075294;            // Limit mitering to 150 degree angle

#define MAX_ITERATIONS 200000

/*=================================================================================**//**
* @bsiclass                                                     JimBartlett     03/99
+===============+===============+===============+===============+===============+======*/
struct          StrokeGenerator
{
private:
    /*---------------------------------------------------------------------------------------
    Stroke input data is set by initialize. pFirst and pLast are not changed during stroker
    operation. pNext is incremented by the stroke generation functions.
    ---------------------------------------------------------------------------------------*/
    int             m_nPoints;
    DPoint3dCP      m_pFirst;    // pointer to the first point in the line
    DPoint3dCP      m_pLast;     // pointer to the last point
    DPoint3dCP      m_next;      // pointer to the next point

    double          m_currentPhase;             // current phase within this pattern.
    double          m_inSegsRemaining;          // total length of input line segments (other than current) remaining to be processed.

    /*---------------------------------------------------------------------------------------
    strokeLength and strokeRemaining are set by setStroke immediately before calling a
    stroke generation function. strokeRemaining is decremented by the stroke generation
    function. If it is not zero when the generation function returns it means the stroker
    did not generate a complete stroke (because it hit the end of the input data)
    ---------------------------------------------------------------------------------------*/
    double          m_strokeLength;             // length of current stroke
    double          m_strokeRemaining;          // unprocessed length of current stroke

    /*---------------------------------------------------------------------------------------
    The segmentXXX fields are set initially by initialize() and subsequently by each call to incrementSegment().
    ---------------------------------------------------------------------------------------*/
    DPoint3d        m_segmentDirection;         // normalized segment direction
    double          m_segmentLength;            // total length of current segment
    double          m_segmentRemaining;         // unprocessed length of current segment

    bool            m_fullTaper;
    double          m_width;
    double          m_taper;

    DPoint3d        m_lastGeneratedPoint;       // previous dash/gap ending point
    Centerline*     m_centerLine;

    void        InsertOutputPoint (DPoint3dCP  pPoint);
    void        SetStroke  (double strokeLength, double phaseCutLength);
    bool        IncrementSegment ();

public:
    StrokeGenerator (DPoint3dCP, int nPts, Centerline*, double length, double width, double taper);

    bool        FinalSegment ()         const {return  m_next == m_pLast;}
    double      GetSegmentRemaining ()  const {return  m_segmentRemaining;}
    double      GetSourceRemaining ()   const {return  m_segmentRemaining + m_inSegsRemaining;}
    bool        AnyStrokeRemaining()    const {return  m_strokeRemaining > s_lsTolerance;}
    bool        AnySegmentRemaining()   const {return  m_segmentRemaining > s_lsTolerance;}
    bool        HasMoreData ()          const {return !FinalSegment() || AnySegmentRemaining();}
    void        SetWidthAndTaper (double width, double taper) {m_width = width; m_taper = taper;}
    void        GenerateNormalStroke (double strokeLength, double phaseCutLength);
    StatusInt   GenerateRigidStroke  (double strokeLength, double phaseCutLength);
    void        SetCurrentPhase (double phase) {m_currentPhase = phase;}
    double      GetCurrentPhase () const      {return m_currentPhase;}
    };

/*---------------------------------------------------------------------------------------
Stroking end conditions:
    If end conditions are present they are tested for only in the first and last segment
    of the input data. If the first repetition of the stroke pattern goes beyond the
    first segment of the input data, only the strokes that fall within the first segment
    are considered for end conditions. Similarly if the final segment of the input is
    shorter than the stroke pattern then only the strokes that fall in the final segment
    are considered for end conditions. If the final segment is longer than one stroke
    pattern then all strokes within one pattern length from the end are considered
    for end conditions. That means that if the end of the segment does not coincide
    with the end of a stroke pattern then the final strokes in the second-to-last
    repetition of the pattern will be considered for end conditions.

Single Segment Mode
    Does not affect the processing of end conditions. The end conditions are still
    considered only at the start and end of the entire point stream, not the  start
    and end of each generated intermediate point stream.

Scales
    Modifier scale applies to all values
    DashScale applies only to stretchable dash strokes
    GapScale applies only to stretchable gap strokes

Automatic phase
    Overrides dashScale, gapScale and phaseShift.
    Calculates new iteration value unless one is already specified

Width override from modifiers, stroke taper, element taper
    If startWidth and endWidth are equal then applyModifiers resets the width fields
    in each stroke during the preprocessing stage.

    If they are not equal (taper) it calculates a taper rate based on the length of the
    input element. The taper rate is passed to generateStrokes. generateStrokes calculates
    a start and end width for each stroke and sets the values in pStroke.

    Wide strokes are rendered by processStroke() based on the stroke centerline produced
    by the stroke generator and the start and end width values in the pStroke passed
    through from generateStrokes().

    Taper is applied after single segment mode (i.e. taper is restarted at each segment).

Cap Angles
    * Used only when the dashes specify a width
    * Calculated automatically if (and only if) none are specified in the pattern
    * If the pattern uses segment mode and does not specify cap angles then the segment
      generator will calculate (override) cap angles for internal vertices
    * If the start and end points are coincident the origin and end angles are set
      to merge the final segment with the first
    Note: Internal strokes that end (or start) on a vertex do not get cap angle adjustment.
    needs-work - maybe it should be further refined to set cap angles only where
                 wide dashes meet - i.e. not where a wide dash meets a gap or a
                 non-width dash.
---------------------------------------------------------------------------------------*/
enum StrokeFlags
    {
    FLAG_None               = 0x00,             // No segment flags
    FLAG_FirstSeg           = 1<<0,             // First point stream segment
    FLAG_LastSeg            = 1<<1,             // Final point stream segment
    FLAG_FirstStroke        = 1<<2,             // First generated stroke
    FLAG_FirstStrokeInSeg   = 1<<4,
    FLAG_NoEndPhase         = 1<<6,
    };

enum CapOptions
    {
    CAP_Closed       = 0,
    CAP_Open         = 1,
    CAP_Extended     = 2,
    CAP_Arc          = 30,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
                LsStroke::LsStroke ()
    {                
    Init (0.0, 0, 0, LCWIDTH_None, LsCapMode::Closed); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
                LsStroke::LsStroke
(
double          length, 
double          startWidth, 
double          endWidth, 
WidthMode       widthMode, 
LsCapMode       capMode
)
    {
    Init (length, startWidth, endWidth, widthMode, capMode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
                LsStroke::LsStroke (LsStroke const &source)
    {
    Init (source.m_length, source.m_orgWidth, source.m_endWidth, (WidthMode)source.m_widthMode, (LsCapMode)source.m_capMode);
    m_strokeMode = source.m_strokeMode;  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     11/98
+---------------+---------------+---------------+---------------+---------------+------*/
LsStrokePatternComponent::LsStrokePatternComponent (LsLocation const *pLocation) : LsComponent (pLocation)
    {
    m_phaseShift    = 0.0;
    m_autoPhase     = 0.0;
    m_maxCompress   = 0.3;
    m_startTangent  = NULL;
    m_endTangent    = NULL;
    m_nIterate      = 1;
    m_patternLength = 0.0;

    memset (&m_options, 0, sizeof(m_options));
    m_nStrokes    = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     03/99
+---------------+---------------+---------------+---------------+---------------+------*/
LsStrokePatternComponent::LsStrokePatternComponent (LsStrokePatternComponent const* base) : LsComponent (base)
    {
    m_phaseShift    = base->m_phaseShift;
    m_autoPhase     = base->m_autoPhase;
    m_startTangent  = base->m_startTangent;
    m_endTangent    = base->m_endTangent;
    m_nIterate      = base->m_nIterate;
    m_options       = base->m_options;
    m_maxCompress   = base->m_maxCompress;
    m_nStrokes      = base->m_nStrokes;
    m_patternLength = base->m_patternLength;

    memcpy (m_strokes, base->m_strokes, m_nStrokes * sizeof(LsStroke));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     11/98
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsStrokePatternComponent::_DoStroke (ViewContextP context, DPoint3dCP inPoints, int nPoints, LineStyleSymbCP modifiers) const
    {
    // test to see whether a single repetition of the pattern will be discernible in this view.
    // If not, use a continuous line style so we'll get width.
    if (!IsSingleRepDiscernible (context, modifiers, *inPoints))
        {
        LsStrokePatternComponent solid (this);
        solid.SetContinuous();
        return solid.ProcessStroke (context, NULL, inPoints, nPoints, modifiers);
        }

    return ProcessStroke (context, NULL, inPoints, nPoints, modifiers);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     11/98
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsStrokePatternComponent::ProcessStroke (ViewContextP context, ISymbolProcess const* symbolProcessor, DPoint3dCP inPoints,
                                              int nPoints, LineStyleSymbCP modifiers) const
    {
    if (nPoints < 2)
        return  ERROR;

    DPoint3dCP  tan1 = NULL;
    DPoint3dCP  tan2 = NULL;
    DPoint3d    startTangent, endTangent;

    // NEEDSWORK_V10: Linestyle api shouldn't require a DgnModel...should just need DgnDb or maybe ViewController...

    bool        hasWidth = (0 != _GetMaxWidth(nullptr) || 0 != modifiers->GetMaxWidth());

    if (hasWidth)
        {
        tan1 = &startTangent;
        tan2 = &endTangent;

        if (modifiers->HasStartTangent())
            startTangent = *modifiers->GetStartTangent();
        else
            {
            if (0.0 == startTangent.NormalizedDifference (*inPoints, inPoints[1]))
                tan1 = NULL;
            }

        if (modifiers->HasEndTangent())
            endTangent = *modifiers->GetEndTangent();
        else
            {
            DPoint3dCP pLast = inPoints + (nPoints-1);
            if (0 == endTangent.NormalizedDifference (*pLast, *(pLast-1)))
                tan2 = NULL;
            }

        // for closed elements, attempt to set the start and end tangents so first/last points join
        if (modifiers->IsElementClosed() && tan1 && tan2 && (-TOLERANCE_MitreLimit > startTangent.DotProduct (endTangent)))
            {
            if (0.0 != endTangent.NormalizedDifference (startTangent, endTangent))
                startTangent.Scale (endTangent, -1.0);
            }
        }

    if (CheckSegmentMode (modifiers))
        {
        DPoint3dCP currPoint = inPoints;
        DPoint3dCP lastPoint = inPoints + (nPoints-1);

        DPoint3d    prevDir, thisDir, nextDir;

        thisDir.NormalizedDifference (currPoint[1], *currPoint);
        prevDir = thisDir;

        int segFlag = FLAG_FirstSeg | FLAG_NoEndPhase;
        for (;currPoint < lastPoint; currPoint++)
            {
            if (currPoint == lastPoint-1)
                {
                nextDir = thisDir;
                segFlag |= FLAG_LastSeg;
                }
            else
                {
                nextDir.NormalizedDifference (currPoint[2], currPoint[1]);
                }

            if (hasWidth)
                {
                tan1 = inPoints  ? &startTangent : &prevDir;
                tan2 = currPoint+1==lastPoint ? &endTangent : &thisDir;
                }

            StrokeLocal (context, symbolProcessor, currPoint, 2, currPoint->Distance (currPoint[1]), modifiers, tan1, tan2, segFlag);
            segFlag  = FLAG_NoEndPhase;

            prevDir = thisDir;
            thisDir = nextDir;
            }
        }
    else
        {
        int32_t segFlag = FLAG_FirstSeg | FLAG_LastSeg;
        if (IsSingleSegment())  // This should be the case where it is an arc (IsTreatAsSingleSegment), but the style itself says to be single segment
            segFlag |= FLAG_NoEndPhase;
        StrokeLocal (context, symbolProcessor, inPoints, nPoints, modifiers->GetTotalLength(), modifiers, tan1, tan2, segFlag);
        }

    return  context->CheckStop() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/99
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::CheckSegmentMode (LineStyleSymbCP pModifiers) const
    {
    // If segment mode is on for this stroke and the line we're working on didn't say to treat as one segment...
    return (IsSingleSegment () && !pModifiers->IsTreatAsSingleSegment());
    }

/*---------------------------------------------------------------------------------**//**
* Apply scale factor to all stroke pattern length values
* @bsimethod                                                    JimBartlett     03/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::ApplyScale (double scale)
    {
    LsStroke    *pStroke    = m_strokes;
    LsStroke    *pEnd       = pStroke + m_nStrokes;

    while (pStroke < pEnd)
        {
        pStroke->m_length      *= scale;
        pStroke->m_orgWidth    *= scale;
        pStroke->m_endWidth    *= scale;
        pStroke++;
        }

    CalcPatternLength();
    m_phaseShift    *= scale;
    }

/*---------------------------------------------------------------------------------**//**
* Apply dash and gap stretch factors to stretchable strokes
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::ApplyStretch (double dashScale, double gapScale)
    {
    LsStroke    *pStroke    = m_strokes;
    LsStroke    *pEnd       = pStroke + m_nStrokes;

    for (;pStroke<pEnd; pStroke++)
        {
        if (pStroke->IsStretchable())
            {
            if (pStroke->IsDash())
                pStroke->m_length *= dashScale;
            else
                pStroke->m_length *= gapScale;
            }
        }

    CalcPatternLength();
    }

/*---------------------------------------------------------------------------------**//**
* Set the start and end width of each width-enabled stroke
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::ApplyWidth (double width)
    {
    LsStroke    *pStroke    = m_strokes;
    LsStroke    *pEnd       = pStroke + m_nStrokes;

    for (;pStroke<pEnd; pStroke++)
        {
        if (pStroke->m_widthMode != 0)
            {
            pStroke->m_orgWidth = width;
            pStroke->m_endWidth = width;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Center phase causes the pattern to be repeated the maximum whole number of times, centered
* on the line. The first and last strokes are "stretched" to accomplish this. We implement
* this by saving another stroke that is the appropriate length of the first and last pattern
* in the N+1 entry of the stroke array. It is then used in place of the original first stroke
* at the start and end of the segment. Of course the original first stroke must be used for
* all other (internal) repetitions of the pattern.
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::ApplyCenterPhase (double elementLength, bool closedElement)
    {
    m_phaseShift = 0.0;

    double fullLength    = _GetLength ();

    if (0.0 == fullLength)
        return;

    // Acad seems to add a slop of 2e-10 to determine if the line needs another iteration of the pattern;
    // so if the line is short of a full iteration by that amount it will "round up" and create another
    // repetition of the pattern.
    static double CENTERED_TOLERANCE = 2e-10;

    double expandedElementLength = elementLength + CENTERED_TOLERANCE;

    // segment is shorter than one full pattern, just draw a solid line.
    if (expandedElementLength < fullLength)
        {
        SetPhaseMode(PHASEMODE_Fixed);
        m_strokes[0].SetLength (expandedElementLength);
        m_strokes[0].SetIsDash (true);
        return;
        }

    // subtract off all full pattern reptitions so strech is between 0 and fullLength
    double stretch = fmod (elementLength, fullLength);

    // If it's close to a whole number of patterns, then just do that number.
    // I'm not as confident in this tolerance value.  This value is just above what I need for a particular DWG file, 
    //   but Acad may be using something larger.
    const double PATTERN_LENGTH_TOLERANCE = 2e-8;
    if (fabs (stretch - fullLength) < PATTERN_LENGTH_TOLERANCE)
        stretch = 0.0;

    // If the element is closed and the stretch exceeds CLOSED_STRETCH_TOLERANCE
    // then we want to avoid putting the stretch slop at the start/end point.
    // and instead we stretch the pattern to fit.
    static double       CLOSED_STRETCH_TOLERANCE = .00001;

    if (closedElement && stretch > CLOSED_STRETCH_TOLERANCE)
        {
        double      scale = elementLength / (fullLength *  floor (.5 + (elementLength / fullLength)));
        LsStroke    *pStroke    = m_strokes, *pEnd       = pStroke + m_nStrokes;

        for (;pStroke<pEnd; pStroke++)
            pStroke->m_length *= scale;
        
        SetPhaseMode (PHASEMODE_Fixed);
        }
    else
        {
        // if there is only one stroke in the linestyle, then we can't stretch the first stroke, so just adjust the phase so that
        // the leftover part of the pattern is split equally between the first and last repetition. TR#239355
        if (1 == m_nStrokes)
            {
            m_phaseShift = (fullLength-stretch) / 2.0;
            SetPhaseMode (PHASEMODE_Fixed);
            }
        else
            {
            double firstLength = m_strokes[0].GetLength();
    
            // save this in the last position in the stroke array.
            m_strokes[m_nStrokes] = m_strokes[0];
            m_strokes[m_nStrokes].SetLength ((stretch + firstLength) / 2.0); // will be applied at beginning and end
            
            // If the first stroke is 0-length, then the leader should not be displayed.  TR #280582.  However, we 
            //   couldn't display dots, so we made our 0-length strokes 1 UOR such that they would display.
            if (m_strokes[0].GetLength() == 1.0 || m_strokes[0].GetLength() == 0.0)
                m_strokes[m_nStrokes].SetIsDash (false);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Modifies stretchable stroke lengths, m_phaseShift and m_nIterate
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::ApplyAutomaticPhase (double elementLength)
    {
    // Automatic phase requires stretchable strokes and is not supported with rigid strokes
    if (!IsStretchable() || IsRigid())
        return;

    LsStroke* pFirst  = m_strokes;

    // Find the fixed and variable length portions of the stroke pattern
    double  stretchableLength=0;
    double  patternLength = GetLength(&stretchableLength);

    // find the length of the first stroke that will be drawn and that which will be clipped
    double  strokeDrawn   = pFirst->m_length * m_autoPhase;
    double  strokeClipped = pFirst->m_length - strokeDrawn;

    // Find the number of repetitions. Adjust for stroke cut at the origin, extra stroke
    // at the end and maximum allowable compression of stretchable strokes. (stroke cut by
    // phase allows more rep space, stroke added at end subtracts from rep space).
    elementLength += strokeClipped;

    if (!HasIterationLimit())
        {
        m_nIterate = (int)((elementLength - strokeDrawn) / (patternLength - (stretchableLength * m_maxCompress)));
        }

    // Find the length that needs to be added to the stretchable portion
    double stretchDist = (elementLength - (m_nIterate * patternLength)) - strokeDrawn;

    // Find the total stretchable stroke length to be generated
    double fullVarLen = stretchableLength * m_nIterate;
    if (m_nIterate && pFirst->IsStretchable())
        fullVarLen += strokeDrawn - strokeClipped;

    // Calculate the scale factor to fit the stretchable portion to the line
    double  scale;
    if (fullVarLen > 0)
        scale = (stretchDist + fullVarLen) / fullVarLen;
    else
        scale = 1.0 - m_maxCompress;

    // Apply the scale to each stretchable stroke
    ApplyStretch (scale, scale);

    // Set the phase shift to position the first stroke
    m_phaseShift = strokeClipped;
    if (pFirst->IsStretchable())
        m_phaseShift *= scale;
    }

/*---------------------------------------------------------------------------------**//**
* Modifies stretchable strokes to produce restricted iteration
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::ApplyIteration (double elementLength)
    {
    if (m_nIterate <= 0 || IsRigid() || !IsStretchable())
        return;

    double stretchable=0;
    double patternLength    = GetLength(&stretchable);
    double scale=0;

    double totalStretchable = stretchable * m_nIterate;
    double totalFixed       = (patternLength - stretchable) * m_nIterate;

    if (totalFixed < elementLength)
        scale = (elementLength - totalFixed) / totalStretchable;

    if (scale < 1.0 - m_maxCompress)
        scale = 1.0 - m_maxCompress;

    ApplyStretch (scale, scale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::ApplyModifiers (double* pOrgWidth, double* pEndWidth, LineStyleSymbCP modifiers)
    {
    double      modScale   = 1.0;

    if (modifiers->HasDashScale() || modifiers->HasGapScale())
        {
        double dashScale=1.0, gapScale=1.0;

        if (modifiers->HasDashScale() && modifiers->GetDashScale() > 0)
            dashScale = modifiers->GetDashScale();

        if (modifiers->HasGapScale() && modifiers->GetGapScale() > 0)
            gapScale = modifiers->GetGapScale();

        ApplyStretch (dashScale, gapScale);
        }

    if (modifiers->HasPhaseShift())
        {
        m_phaseShift = modifiers->GetPhaseShift();
        SetPhaseMode (PHASEMODE_Fixed);
        }

    if (modifiers->IsCenterPhase())
        SetPhaseMode (PHASEMODE_Center);

    if (modifiers->IsAutoPhase() && modifiers->GetFractionalPhase() >= 0)
        {
        m_autoPhase = modifiers->GetFractionalPhase();
        SetPhaseMode (PHASEMODE_Fraction);
        }

    if (modifiers->HasMaxCompress())
        m_maxCompress = modifiers->GetMaxCompress();

    if (modifiers->HasIterationLimit() && modifiers->GetNumIterations() > 0)
        {
        m_nIterate = modifiers->GetNumIterations();
        SetIterationMode (true);
        }

    if (modifiers->IsCosmetic())
        SetCosmetic (true);

    // Scale needs to be done late because it also modifies phase, etc. set above.
    if (modifiers->IsScaled() && modifiers->GetScale() > 0)
        {
        modScale = modifiers->GetScale();
        ApplyScale (modScale);
        }

    // And, as it turns out, width needs to be done after scale since the scale is applied to the width
    //   as appropriate when the symbology is created.
    // If start and end width are both present, not equal and not zero, then taper is in effect and must be applied
    // externally - set return status to true. Otherwise widths can be applied directly to the stroke pattern
    if (modifiers->HasOrgWidth() || modifiers->HasEndWidth())
        {
        double orgWidth = modifiers->GetOriginWidth();
        double endWidth = modifiers->GetEndWidth();

        if (orgWidth == endWidth)
            {
            ApplyWidth (orgWidth);
            }
        else
            {
            *pOrgWidth = orgWidth;
            *pEndWidth = endWidth;
            return  true;
            }
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::FixDashWidths (double& orgWidth, double& endWidth, bool taper, ViewContextCP context, DPoint3dCP pt)
    {
    // Make sure the start/end width are not less than a pixel. Otherwise they "disappear".
    //  DirectX doesn't display geometry that doesn't cross through the center of a pixel.
    // It is possible for either the style itself or the element modifiers to have applied
    //   widths, and in that case they may drop below pixel size.

    if (NULL == context->GetViewport())
        return;

    double pixelWidth = 0.0;
    bool pixelWidthCalculated = false;

    // If it's a tapered line, fix up taper widths
    if (taper)
        {
        // Don't calculate this until we absolutely need it.
        pixelWidth = context->GetPixelSizeAtPoint (pt);
        pixelWidthCalculated = true;

        if (orgWidth < pixelWidth)
            orgWidth = pixelWidth;
        if (endWidth < pixelWidth)
            endWidth = pixelWidth;
        }

    // Also need to look at the dashes, if appropriate
    if (!_HasWidth())
        return;

    for (uint32_t iStroke = 0; iStroke<m_nStrokes; iStroke++)
        {
        LsStroke* pStroke = m_strokes+iStroke;
        // HasWidth will be false for gaps.  Only want to do this if there is some non-zero width.
        if (pStroke->HasWidth() && (pStroke->m_orgWidth > 0.0 || pStroke->m_endWidth > 0.0))
            {
            // Don't calculate this until we absolutely need it.
            if (!pixelWidthCalculated)
                {
                pixelWidth = context->GetPixelSizeAtPoint (pt);
                pixelWidthCalculated = true;
                }

            if (pStroke->m_orgWidth < pixelWidth)
                pStroke->m_orgWidth = pixelWidth;
            if (pStroke->m_endWidth < pixelWidth)
                pStroke->m_endWidth = pixelWidth;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Called to stroke a preprocessed copy of a stroke pattern. Called only from the
* public stroke method.
* @bsimethod                                                    JimBartlett     03/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::StrokeLocal
(
ViewContextP            context,
ISymbolProcess const*   symbolProcessor,
DPoint3dCP              inPoints,
int                     nPoints,
double                  length,
LineStyleSymbCP         modifiers,
DPoint3dCP              startTangent,
DPoint3dCP              endTangent,
int                     segFlag
) const
    {
    double patLength  = _GetLength () * modifiers->GetScale();

    // Make a copy of the stroke definitions so that the original are unchanged
    LsStrokePatternComponent   oClone (this);

    // Occasionally due to problems in files, the number of iterations is huge.
    // In that case, just draw it as continuous.  TR #138719
    if (patLength <= 0.0 || ((length / patLength) > MAX_ITERATIONS))
        {
        oClone.SetContinuous();
        }

    oClone.m_startTangent = startTangent;
    oClone.m_endTangent   = endTangent;

    // Apply modifiers to the stroke pattern and check for length and taper requirements
    bool        usesTaper = false;
    double      orgWidth=0, endWidth=0, taper=0;

    if (modifiers != NULL)
        usesTaper = oClone.ApplyModifiers (&orgWidth, &endWidth, modifiers);

    oClone.FixDashWidths (orgWidth, endWidth, usesTaper, context, inPoints);  // No dashes less than 1 pixel wide.

    if (usesTaper && length > 0)
        {
        taper = (orgWidth - endWidth) / length;
        oClone.m_startTangent = oClone.m_endTangent = NULL;
        }

    // Apply auto-phase and iteration control adjustments
    if (LsStrokePatternComponent::PHASEMODE_Center == oClone.GetPhaseMode())
        oClone.ApplyCenterPhase (length, modifiers->IsElementClosed() && modifiers->IsCurve());
    else if (LsStrokePatternComponent::PHASEMODE_Fraction == oClone.GetPhaseMode())
        oClone.ApplyAutomaticPhase(length);
    else if (m_options.iterationLimit)
        oClone.ApplyIteration(length);

    double endPhase = oClone.GenerateStrokes (context, symbolProcessor, modifiers, inPoints, nPoints, length, orgWidth, taper, segFlag);

    if (!(segFlag & FLAG_NoEndPhase) && !oClone._IsContinuous() && (LsStrokePatternComponent::PHASEMODE_Center != oClone.GetPhaseMode()))
        {
        endPhase /= modifiers->GetScale();    // Phase is assumed to be in line style units; it will be scaled the next time through.
        ((LineStyleSymbP) modifiers)->SetXElemPhase (endPhase);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     03/99
+---------------+---------------+---------------+---------------+---------------+------7*/
double          LsStrokePatternComponent::GenerateStrokes (ViewContextP context, ISymbolProcess const* symbolProcessor, LineStyleSymbCP modifiers,
                                                DPoint3dCP inPoints, int nPoints, double length, double width, double taper, int segFlag)
    {
    BeAssert (m_nStrokes != 0);

    // Setup phase shift
    double patternLength = _GetLength();

    if (patternLength <= 0.0)
        return 0.0;

    double phaseShift = m_phaseShift;

    if (phaseShift >= patternLength) // keep phase within one pattern iteration length
        phaseShift = fmod (m_phaseShift,  patternLength);

    LsStroke*   pFirstStroke = m_strokes;
    LsStroke*   pLastStroke  = pFirstStroke + (m_nStrokes-1);
    LsStroke*   pStroke      = pFirstStroke;

    bool   taperWholeLine = (taper != 0.0);

    // Iterate through the stroke list consuming the input data and generating strokes.
    // NOTE: pStroke may have been advanced by phase shift above
    ScopedArray<DPoint3d, 50> scopedClPoints(nPoints);
    DPoint3dP   clPts     = scopedClPoints.GetData();
    ScopedArray<double, 50> scopedClWidths(nPoints);
    double*     clWidths  = scopedClWidths.GetData();
    ScopedArray<double, 50> scopedClLengths(nPoints);
    double*     clLengths = scopedClLengths.GetData();

    Centerline  centerLine (clPts, clWidths, clLengths, taperWholeLine);

    // Create a stroke generator and initialize it with the input point array
    StrokeGenerator   stroker (inPoints, nPoints, &centerLine, length, width, taper);
    stroker.SetCurrentPhase (phaseShift);

    DVec3d    normal;
    RotMatrix matrix;
    modifiers->GetPlaneAsMatrixRows (matrix);
    matrix.GetRow (normal, 2);

    double  segmentRemaining = 0;
    double  cutLength = 0.0;

    if (phaseShift > 0)
        {
        // If there is phase shift, find the stroke where the phase cut will end and calculate
        // the phase cut length for the stroke.
        while (pStroke->m_length <= phaseShift)
            {
            phaseShift -= pStroke->m_length;
            pStroke++;
            }

        if (phaseShift > 0.0)
            cutLength = phaseShift;
        }

    /*-----------------------------------------------------------------------------------
    If segFlag is zero it means that there is no single segment mode so the flags
    for first and last segment are set in the loop below.
    -----------------------------------------------------------------------------------*/
    int endFlag = FLAG_FirstStrokeInSeg;
    if (segFlag == 0 || ((segFlag & FLAG_FirstSeg) != 0))
        endFlag = FLAG_FirstStroke | FLAG_FirstSeg | FLAG_FirstStrokeInSeg;

    bool        isCenterPhase = LsStrokePatternComponent::PHASEMODE_Center == GetPhaseMode();
    LsStroke*   outStroke;
    double      lastPatternLen = patternLength;

    bool lastStrokeCenterPhase = false;
    bool firstStrokeCenterPhaseGap = false;
    for (; ; )
        {
        segmentRemaining = stroker.GetSegmentRemaining();

        // if this is the first stroke of a "center phase" pattern, use the n+1 pattern for the first stroke.
        if (isCenterPhase && ((endFlag & FLAG_FirstStrokeInSeg) != 0))
            {
            outStroke = &m_strokes[m_nStrokes];

            // add the additional (stretched) size to the last pattern length
            lastPatternLen += (outStroke->m_length - m_strokes[0].m_length);
            
            // If the first stroke is a gap, then the centering leader precedes the first stroke. 
            //   If it's a line, then it is the first stroke.  Adesk rule.
            firstStrokeCenterPhaseGap = !m_strokes[0].IsDash();
            }
        else
            {
            outStroke = pStroke;
            }

        if (!taperWholeLine && 0 != outStroke->GetWidthMode() && (outStroke->GetLength() > 0))
            stroker.SetWidthAndTaper (outStroke->GetStartWidth(), (outStroke->GetStartWidth() - outStroke->GetEndWidth()) / outStroke->GetLength());

        if (outStroke->IsRigid())
            {
            if (SUCCESS != stroker.GenerateRigidStroke (outStroke->m_length, cutLength))
                return 0.0;
            }
        else
            {
            stroker.GenerateNormalStroke (outStroke->m_length, cutLength);
            }

        if (isCenterPhase && (stroker.GetSourceRemaining() < lastPatternLen))
            {
            pLastStroke  = pFirstStroke + m_nStrokes;       // on the last repetition of Center Phase, use the N+1 stroke too
            lastStrokeCenterPhase = true;                   // but on the "stretched" stroke, don't draw any symbols
            }

        // fmod has some slight rounding 
        const double       PATLEN_TOLERANCE = 2e-10;
        if (isCenterPhase && (stroker.GetSourceRemaining() < lastPatternLen-PATLEN_TOLERANCE))
            {
            pLastStroke  = pFirstStroke + m_nStrokes;       // on the last repetition of Center Phase, use the N+1 stroke too
            lastStrokeCenterPhase = true;                   // but on the "stretched" stroke, don't draw any symbols
            }

        /*---------------------------------------------------------------------------
        Check for the final iteration of the stroke pattern in the final segment of
        a point stream. If segFlag is set the stream was broken up earlier by
        single segment mode. Otherwise the whole stream is processed on this loop.
        ---------------------------------------------------------------------------*/
        if (stroker.FinalSegment() && (segmentRemaining <= lastPatternLen))
            {
            if (0 != (segFlag & FLAG_LastSeg))
                endFlag |= FLAG_LastSeg;
            }

        // If end conditions are enabled then a dash may turn to a gap and vice versa in the
        // first and last repetition of the stroke pattern.
        bool    isDash = outStroke->IsDash();
        if (AreEndConditionsEnabled())
            {
            if ((endFlag & FLAG_FirstSeg) != 0)
                {
                isDash = outStroke->IsDashFirst();
                }
            else if ((endFlag & FLAG_LastSeg) != 0)
                {
                isDash = outStroke->IsDashLast();
                }
            }

        int     strokeIndex = static_cast<int>(outStroke - pFirstStroke);
        bool    skipSymbol  = (lastStrokeCenterPhase && strokeIndex==m_nStrokes && NULL != symbolProcessor); // only skip symbols on last segment of centered phase
        if (!skipSymbol && firstStrokeCenterPhaseGap && NULL != symbolProcessor)
            skipSymbol = true; // Also skip symbols on the first symbol if it the first stroke is a gap.

        strokeIndex %= m_nStrokes;
        if (!skipSymbol && ((NULL == symbolProcessor) || (symbolProcessor->_ProcessSymbol (context, &centerLine, modifiers, outStroke, strokeIndex, endFlag))))
            {
            if (isDash)
                {
                DPoint3dCP startTangent = (0 == (endFlag & FLAG_FirstStroke))    ? NULL : m_startTangent;
                DPoint3dCP endTangent   = (stroker.GetSourceRemaining() != 0.0)  ? NULL : m_endTangent;

                centerLine.Output (context, outStroke, &normal, startTangent, endTangent);
                }
            }

        centerLine.Empty();
        endFlag = 0;
        cutLength = 0.0;

        if (!stroker.HasMoreData() || context->CheckStop())
            break;

        if (firstStrokeCenterPhaseGap || ++pStroke > pLastStroke)
            {
            pStroke = pFirstStroke;
            stroker.SetCurrentPhase (0);
            firstStrokeCenterPhaseGap = false;
            }

        } // stroke loop

    return  stroker.GetCurrentPhase();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::CalcPatternLength ()
    {
    LsStroke const*    pStroke  = m_strokes;
    LsStroke const*    pEnd     = pStroke + m_nStrokes;

    m_patternLength = 0.0;

    while (pStroke < pEnd)
        {
        m_patternLength += pStroke->m_length;
        pStroke++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
double  LsStrokePatternComponent::_GetMaxWidth (DgnModelP DgnModel) const
    {
    LsStroke const*    pStroke = m_strokes;
    LsStroke const*    pEnd    = pStroke + m_nStrokes;

    double      width=0.0;

    while (pStroke < pEnd)
        {
        if ((pStroke->m_widthMode & 0x3) != 0)
            {
            if (pStroke->m_orgWidth > width)
                width = pStroke->m_orgWidth;

            if (pStroke->m_endWidth > width)
                width = pStroke->m_endWidth;
            }

        pStroke++;
        }

    return  width;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
double  LsStrokePatternComponent::GetLength (double* pStretchable) const
    {
    LsStroke const*    pStroke  = m_strokes;
    LsStroke const*    pEnd     = pStroke + m_nStrokes;

    double  stretchable=0.0, fixed=0.0;

    while (pStroke < pEnd)
        {
        if (pStroke->IsStretchable())
            stretchable += pStroke->m_length;
        else
            fixed += pStroke->m_length;

        pStroke++;
        }

    if (pStretchable != NULL)
        *pStretchable   = stretchable;

    return  stretchable + fixed;
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if one or more of the strokes in the stroke pattern is stretchable
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
bool    LsStrokePatternComponent::IsStretchable () const
    {
    LsStroke const*    pStroke  = m_strokes;
    LsStroke const*    pEnd     = pStroke + m_nStrokes;

    while (pStroke < pEnd)
        {
        if (pStroke->IsStretchable())
            return  true;

        pStroke++;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if one or more of the strokes in the stroke pattern is rigid
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
bool    LsStrokePatternComponent::IsRigid () const
    {
    LsStroke const*    pStroke  = m_strokes;
    LsStroke const*    pEnd     = pStroke + m_nStrokes;

    while (pStroke < pEnd)
        {
        if (pStroke->IsRigid())
            return  true;

        pStroke++;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if this line style requires the length of the input element/points
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::RequiresLength () const
    {
    int  phaseMode = GetPhaseMode();

    return (phaseMode == PHASEMODE_Fraction || phaseMode == PHASEMODE_Center || (m_options.iterationLimit != 0));
    }

/*---------------------------------------------------------------------------------**//**
* Gets the current phase mode. 0=None, 1=Fixed, 2=Automatic
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
LsStrokePatternComponent::PhaseMode LsStrokePatternComponent::GetPhaseMode () const
    {
    return (LsStrokePatternComponent::PhaseMode)m_options.phaseMode;
    }

/*---------------------------------------------------------------------------------**//**
* Set the phase mode for this stroke pattern
* @param        mode     0=None, 1=Fixed, 2=Automatic
* @return       false if the mode is invalid
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::SetPhaseMode (LsStrokePatternComponent::PhaseMode mode)
    {
    m_options.phaseMode = mode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
double          LsStrokePatternComponent::GetDistancePhase () const
    {
    return  m_phaseShift;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::SetDistancePhase (double newPhase)
    {
    SetPhaseMode (PHASEMODE_Fixed);
    m_phaseShift = newPhase;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
double          LsStrokePatternComponent::GetFractionalPhase () const
    {
    return  m_autoPhase;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::SetFractionalPhase (double newPhase)
    {
    SetPhaseMode (PHASEMODE_Fraction);
    m_autoPhase = newPhase;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::SetCenterPhaseMode ()
    {
    SetPhaseMode (PHASEMODE_Center);
    }

/*---------------------------------------------------------------------------------**//**
* Set the current phase value
* @bsimethod                                                    RayBentley      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::SetMaxCompress (double maxCompress)
    {
    m_maxCompress = maxCompress;
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if iteration limit is on, false if it is off.
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::HasIterationLimit () const
    {
    return  (m_options.iterationLimit != 0);
    }

/*---------------------------------------------------------------------------------**//**
* Sets the current iteration mode on or off. When called to turn iteration mode on
* it also checks the current iteration count and sets it to 1 if the current value is
* invalid (zero).
*
* Returns false if the stroke pattern has no stretchable strokes and therefore cannot
* support iteration limits.
*
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::SetIterationMode (bool limited)
    {
    // If it's not stretchable then you can turn off the limit but not turn it on
    if (limited && !IsStretchable())
        return  false;

    m_options.iterationLimit = limited;

    if (limited && m_nIterate <= 0)
        m_nIterate = 1;
    else if (!limited)
        m_nIterate = 0;

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Returns the current iteration limit
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
int             LsStrokePatternComponent::GetIterationLimit () const
    {
    return  m_nIterate;
    }

/*---------------------------------------------------------------------------------**//**
* Set the iteration limit count. Calling this method also enables iteration mode.
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::SetIterationLimit (int count)
    {
    if (count > 0)
        {
        m_nIterate = count;
        m_options.iterationLimit = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if end conditions are anabled
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::AreEndConditionsEnabled () const
    {
    return  m_options.enableEndConditions;
    }

/*---------------------------------------------------------------------------------**//**
* Enable or disable support for end conditions
* @bsimethod                                                    JimBartlett     01/99
+---------------+---------------+---------------+---------------+---------------+------*/
void    LsStrokePatternComponent::SetEndConditions (bool enabled)
    {
    m_options.enableEndConditions = enabled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/98
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::IsSingleSegment () const
    {
    return  (m_options.segMode != 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/98
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::SetSegmentMode (bool isSingle)
    {
    m_options.segMode = isSingle;
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if the stroke pattern is defined with cosmetic units
* @bsimethod                                                    JimBartlett     08/99
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::IsCosmetic () const
    {
    return  (m_options.cosmetic != 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::SetContinuous ()
    {
    m_nStrokes = 1;
    m_strokes[0].m_strokeMode = LsStroke::STROKE_Dash;
    m_strokes[0].m_length = fc_hugeVal;
    }

#define RMAXI4                  2147483647.0
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/99
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::_IsContinuous () const
    {
    return  (m_nStrokes == 0 ||
            (m_nStrokes == 1 && m_strokes[0].IsDash() && (RMAXI4 <= m_strokes[0].GetLength())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 09/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::_IsContinuousOrSingleDash () const
    {
    // Same as IsContinuous, but returns true for any single-dash style, not just those that are specifically DWG Continuous length
    return  (m_nStrokes == 0 ||
            (m_nStrokes == 1 && m_strokes[0].IsDash() ));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
double          LsStrokePatternComponent::_CalcRepetitions (LineStyleSymbCP lsSymb) const
    {
    if (IsCosmetic() || _IsContinuous())
        return  1;

    double reps  = LsComponent::_CalcRepetitions (lsSymb);
    double limit = GetIterationLimit();

    if (0 != limit && (reps > limit))
        reps = limit;

    return  reps;
    }

/*---------------------------------------------------------------------------------**//**
* Set the stroke pattern to use cosmetic or geometric units
* @param        cosmetic true for cosmetic units, false for geometric units
* @bsimethod                                                    JimBartlett     08/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::SetCosmetic (bool cosmetic)
    {
    m_options.cosmetic = cosmetic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/98
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::AppendStroke (double length, bool isDash)
    {
    BeAssert (length >= 0.0);

    LsStroke  stroke (length, 0.0, 0.0, LsStroke::LCWIDTH_None, LsCapMode::Closed);

    if (isDash)
        {
        stroke.SetIsDash (true);
        stroke.m_widthMode  = LsStroke::LCWIDTH_Full;
        }

    AppendStroke (stroke);
    }

/*---------------------------------------------------------------------------------**//**
* Insert a stroke into the stroke pattern component
* @param        pStroke Stroke to insert
* @param        index   Stroke index or -1 to append
* @bsimethod                                                    JimBartlett     12/99
+---------------+---------------+---------------+---------------+---------------+------*/
LsStrokeP       LsStrokePatternComponent::AppendStroke (LsStrokeCR stroke)
    {
    if (m_nStrokes >= 32)
        return NULL;

    LsStrokeP   retval = &m_strokes[m_nStrokes++];
    *retval = stroke;
    
    return retval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/99
+---------------+---------------+---------------+---------------+---------------+------*/
LsStrokeP       LsStrokePatternComponent::AppendStroke (double length, double startWidth, double endWidth, LsStroke::WidthMode widthMode, LsCapMode capMode)
    {
    LsStroke    stroke (length, startWidth, endWidth, widthMode, capMode);
    return AppendStroke (stroke);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/98
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::DeleteStroke (size_t index)
    {
    if (m_nStrokes > index)
        {
        m_nStrokes--;
        memcpy (m_strokes+index, m_strokes+index+1, m_nStrokes-index);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     12/98
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::GetStroke (LsStroke*  pStroke, size_t index)
    {
    *pStroke = m_strokes[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     03/99
+---------------+---------------+---------------+---------------+---------------+------*/
StrokeGenerator::StrokeGenerator
(
DPoint3dCP      inPoints,
int             nPoints,
Centerline*     centerline,
double          length,
double          width,
double          taper
)
    {
    m_nPoints = nPoints;
    m_pFirst  = inPoints;
    m_pLast   = m_pFirst + (nPoints-1);
    m_next    = m_pFirst;

    m_lastGeneratedPoint = *m_pFirst;

    m_inSegsRemaining   = length;
    m_strokeLength      = 0;
    m_strokeRemaining   = 0;
    m_segmentLength     = 0;
    m_segmentRemaining  = 0;

    m_centerLine = centerline;
    m_fullTaper  = (0.0 != taper);
    m_taper      = taper;
    m_width      = width;

    memset (&m_segmentDirection, 0, sizeof(m_segmentDirection));
    IncrementSegment();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     03/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            StrokeGenerator::SetStroke (double strokeLength, double phaseCutLength)
    {
    m_strokeRemaining   = strokeLength - phaseCutLength;
    m_strokeLength      = strokeLength;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     03/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            StrokeGenerator::GenerateNormalStroke (double strokeLength, double phaseCutLength)
    {
    SetStroke (strokeLength, phaseCutLength);

    InsertOutputPoint (&m_lastGeneratedPoint);
    m_centerLine->SetDirection (&m_segmentDirection);  // Needed if there's a 0-length segment with a point symbol

    /*-----------------------------------------------------------------------------------
    Loop until a complete stroke is generated or until the input data is exhausted.
    Note: Due to additive errors, we purposely leave the "stroke remaining" and "segment
          remaining" tests sloppy. A better way to do it would be to recalculate the distance
          from the end every time, but that would involve a square root, and this is executed a lot.
    -----------------------------------------------------------------------------------*/
    while (AnyStrokeRemaining())
        {
        if (m_strokeRemaining > m_segmentRemaining)
            {
            /*---------------------------------------------------------------------------
            This stroke goes beyond the current segment.
            ---------------------------------------------------------------------------*/
            m_strokeRemaining -= m_segmentRemaining;
            m_currentPhase    += m_segmentRemaining;

            m_segmentRemaining = 0;
            InsertOutputPoint (m_next);

            if (!IncrementSegment())
                return;
            }
        else
            {
            /*---------------------------------------------------------------------------
            This stroke fits within the current segment
            ---------------------------------------------------------------------------*/
            DPoint3d    tmpPoint;
            tmpPoint.SumOf (m_lastGeneratedPoint,m_segmentDirection, m_strokeRemaining);

            m_currentPhase      += m_strokeRemaining;
            m_segmentRemaining  -= m_strokeRemaining;
            m_strokeRemaining    = 0;

            InsertOutputPoint(&tmpPoint);

            if (!AnySegmentRemaining())
                {
                if (!IncrementSegment())
                    return;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     03/99
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       StrokeGenerator::GenerateRigidStroke (double strokeLength, double phaseCutLength)
    {
    SetStroke (strokeLength, phaseCutLength);

    /*-----------------------------------------------------------------------------------
    If the stroke will fit on the current segment then it is generated as a normal stroke
    -----------------------------------------------------------------------------------*/
    if (m_strokeLength <= m_segmentRemaining)
        {
        GenerateNormalStroke (strokeLength, phaseCutLength);

        return SUCCESS;
        }

    InsertOutputPoint (&m_lastGeneratedPoint);

    double      distance;
    do
        {
        distance = m_lastGeneratedPoint.Distance (*m_next);
        } while (distance < m_strokeLength && IncrementSegment());

    if (distance >= m_strokeLength)
        {
        /*-------------------------------------------------------------------------------
        The stroke went off the current segment, the segment has been incremented so that
        m_next should be the point past the stroke, and the stroke did not hit the end
        of the input data. To find the intersection point consider a sphere with it's
        center at lastGeneratedPoint, and a radius of strokeLength. Find the intersection
        with the ray that starts at m_next point and shoots back toward the sphere
        (direction is opposite of segmentDirection). In this case the intersection
        always exists and will hit the right one first. See Graphics Gems Pg 388.
        -------------------------------------------------------------------------------*/
        DPoint3d    rayDir, intPoint, strokeVec;
        double      dot, mag, radSq;

        strokeVec.DifferenceOf (m_lastGeneratedPoint, *m_next);
        rayDir.Scale (m_segmentDirection, -1.0);
        dot     = rayDir.DotProduct (strokeVec);

        mag     = strokeVec.DotProduct (strokeVec);
        radSq   = m_strokeLength * m_strokeLength;
        mag     = dot - sqrt(radSq - (mag - dot*dot));;
        intPoint.SumOf (*m_next,rayDir, mag);

        m_segmentRemaining  = intPoint.Distance (*m_next);
        m_strokeRemaining   = 0.0;

        InsertOutputPoint(&intPoint);
        }
    else
        {
        // If single repetition spans entire element no direction is defined, don't draw anything...
        double      dist = m_lastGeneratedPoint.Distance (*m_next);

        if (0.0 == dist)
            return ERROR;

        /*-------------------------------------------------------------------------------
        The input data is exhausted so the stroke is generated to the final input point
        -------------------------------------------------------------------------------*/
        m_segmentRemaining  = 0.0;
        m_strokeRemaining   = m_strokeLength - dist;
        InsertOutputPoint(m_next);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     03/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            StrokeGenerator::InsertOutputPoint (DPoint3dCP point)
    {
    BeAssert (m_centerLine->GetCount() < m_nPoints);

    double length = m_strokeLength - m_strokeRemaining;

    // reduce width by taper * lengthOfThisSegment
    double width;
    if (m_fullTaper)
        width  = m_width - (m_taper * (m_segmentLength - m_segmentRemaining));
    else
        width  = m_width - (m_taper * length);

    m_lastGeneratedPoint  = *point;
    m_centerLine->AddPoint (point, width, length);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            StrokeGenerator::IncrementSegment ()
    {
    // if we're doing a full taper, subtract the taper of previous segment (m_width is relative to current segment).
    if (m_fullTaper)
        m_width -= (m_segmentLength * m_taper);

    while (m_next < m_pLast)
        {
        m_next++;

        if (0.0 == (m_segmentLength = m_segmentDirection.NormalizedDifference (*m_next, *(m_next-1))))
            continue;

        m_segmentRemaining  = m_segmentLength;
        m_inSegsRemaining  -= m_segmentLength;

        return  true;
        }

    m_inSegsRemaining  = 0.0;
    m_segmentRemaining = 0.0;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* extend first and last point by half of the widths
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extendEndPoints (DPoint3dP pt, double const* width, int nPts)
    {
    DPoint3d    segDir;
    segDir.NormalizedDifference (*pt, pt[1]);
    pt->SumOf (*pt,segDir, *width / 2.0);

    pt     += nPts-1;
    width  += nPts-1;

    segDir.NormalizedDifference (*pt, *(pt-1));
    pt->SumOf (*pt,segDir, *width / 2.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    createPolygon
(
DPoint3dP        polygon,
int              nPts,
DPoint3dCP       pts,
double const*    widths,
LineJoint const* clJoints,
int              widthMode
)
    {
    DPoint3dCP  src   = pts;
    DPoint3dCP  end   = src + nPts;
    LineJoint const* joint = clJoints;
    double const*    width = widths;
    DPoint3d*        dst   = polygon;

    // Generate points to the left of center
    if ((widthMode & 0x1) != 0) // upper half
        {
        for (; src < end; src++, dst++, joint++, width++)
            dst->SumOf (*src,joint->m_dir, ((*width)/2.0) * joint->m_scale);
        }
    else
        {
        for (; src < end; src++, dst++)
            *dst = *src;
        }

    src    = pts + (nPts-1);
    joint  = clJoints + (nPts-1);
    width  = widths + (nPts-1);
    end    = pts;

    // Generate points to the right of center
    if ((widthMode & 0x2) != 0) // lower half
        {
        for (; src >= end; src--, dst++, joint--, width--)
            dst->SumOf (*src,joint->m_dir, -(((*width)/2.0) * joint->m_scale));
        }
    else
        {
        for (; src >= end; src--, dst++)
            *dst = *src;
        }

    // set last point equal to first to close polygon
    *dst = *polygon;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    createTristrip (DPoint3dP tristrip, int nPts, DPoint3dCP centerLinePts, double const* widths, LineJoint const* clJoints, int widthMode)
    {
    DPoint3dCP  src = centerLinePts;
    DPoint3dCP  end   = src + nPts;
    LineJoint const* joint = clJoints;
    double const*    width = widths;

    bool doUpper = (widthMode & 0x1) != 0;
    bool doLower = (widthMode & 0x2) != 0;

    for (DPoint3dP dst = tristrip; src < end; src++, dst++, joint++, width++)
        {
        if (doUpper)
            dst->SumOf (*src,joint->m_dir, ((*width)/2.0) * joint->m_scale);
        else
            *dst = *src;

        dst++;

        if (doLower)
            dst->SumOf (*src,joint->m_dir, -(((*width)/2.0) * joint->m_scale));
        else
            *dst = *src;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     outputCapArc (IDrawGeom* output, DPoint3dCP pt1, DPoint3dCP pt2, DPoint3dCP pt3, bool doFill)
    {
    DVec3d      xDir, yDir, zDir;

    if (0.0 == yDir.NormalizedDifference (*pt1, *pt2))
        return;

    if (0.0 == xDir.NormalizedDifference (*pt3, *pt1))
        return;

    zDir.CrossProduct (xDir, yDir);
    yDir.NormalizedCrossProduct (zDir, xDir);

    DPoint3d    origin;

    origin.Interpolate (*pt1, .5, *pt3);

    double      radius = origin.Distance (*pt1);
    DEllipse3d  ellipse;

    ellipse.InitFromDGNFields3d (origin, xDir, yDir, radius, radius, 0.0, msGeomConst_pi);
    output->DrawArc3d (ellipse, false, doFill, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void    outputPolygon
(
IDrawGeom*     output,
int            nPts,
DPoint3dCP     pts,
double const*  widths,
int            widthMode,
int            capMode,
DPoint3dCP     normal,
DPoint3dCP     startTangent,
DPoint3dCP     endTangent,
bool           polyLengthNotDiscernible
)
    {
    ScopedArray<LineJoint, 50>  scopedJoints(nPts);
    LineJoint*  joints = scopedJoints.GetData();
    LineJoint::FromVertices (joints, pts, nPts, normal, startTangent, endTangent);
    size_t numOutsidePts = (nPts * 2) + 11;
    ScopedArray<DPoint3d, 50>  scopedOutsidePts(numOutsidePts);
    DPoint3d*   outsidePts = scopedOutsidePts.GetData();

    if (capMode == CAP_Open)
        {
        createPolygon (outsidePts, nPts, pts, widths, joints, widthMode);
        output->DrawLineString3d (nPts, outsidePts, NULL);
        output->DrawLineString3d (nPts, outsidePts+nPts, NULL);
        }
    else
        {
        bool  doFill = true; // NOTE: Always fill...don't check output->GetDrawViewFlags()->fill

        if (polyLengthNotDiscernible)
            {
            createPolygon (outsidePts, nPts, pts, widths, joints, widthMode);
            output->DrawLineString3d (2, outsidePts+1, NULL);

            if (capMode > 2)
                {
                outputCapArc (output, outsidePts, outsidePts+1, outsidePts+2*nPts-1, doFill);
                DPoint3dCP base = outsidePts+nPts;
                outputCapArc (output, base, base+1, base-1, doFill);
                }
            }
        else
            {
            createTristrip (outsidePts, nPts, pts, widths, joints, widthMode);
            output->DrawTriStrip3d (nPts*2, outsidePts, 1,NULL);

            if (capMode > 2)
                {
                outputCapArc (output, outsidePts, outsidePts+2, outsidePts+1, doFill);
                DPoint3dCP base = outsidePts+((nPts-1)*2);
                outputCapArc (output, base, base-2, base+1, doFill);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/04
+---------------+---------------+---------------+---------------+---------------+------*/
static inline double  xySize (DPoint3dCP p1, DPoint3dCP p2)
    {
    return fabs (p1->x - p2->x) + fabs (p1->y - p2->y);
    }

static const double  MIN_LOD = 2.0;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            segmentNotDiscernible (ViewContextP context, int numPoints, DPoint3dCP points)
    {
    // Quick check on number of points...
    if (1 == numPoints)
        return true;

    if (numPoints > 2)
        return false;

    // Not attached...
    if (NULL == context->GetViewport ())
        return false;

    DPoint3d    viewPts[2];
    context->LocalToView (viewPts, points, 2);

    double      size = xySize (viewPts, viewPts+1);
    return (size < MIN_LOD);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 03/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            Centerline::GetDirectionVector (DPoint3dR segDir, DPoint3dCR org, DPoint3dCR end) const
    {
    // Convenience method. Centerlines with only 1 point are 0-length dashes, which draw as points.  If they
    //   have symbols attached then they must have a direction.  Can't always use the direction vector
    //   stored on the centerline because dashes that bend around corners will have multiple "directions."
    if (1 == GetCount())
        segDir = *GetDirection ();
    else
        segDir.NormalizedDifference (end, org);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            Centerline::AddPoint (DPoint3dCP pt, double width, double length)
    {
    m_pts[m_count]      = *pt;
    m_lengths[m_count]  = length;
    m_widths[m_count++] = width;
    if (0.0 != width)
        m_hasWidth = true;
    }

/*---------------------------------------------------------------------------------**//**
* Expand a single line into one or more polygons with a specified width.
*   pPoints is doubled +1 (last point duplicates first)
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            Centerline::Output (ViewContextP context, LsStrokeP pStroke, DPoint3dCP normal, DPoint3dCP startTangent, DPoint3dCP endTangent)
    {
    int nPts = GetCount();
    if (nPts <= 0)
        return;

    IDrawGeomR    output = context->GetIDrawGeom();

    int     widthMode = pStroke->GetWidthMode() & 0x3;

    // if there's no width, just output the centerline
    if (nPts == 1 || !m_hasWidth || (widthMode == 0))
        {
        DPoint3dCP points = GetPointAt (0);

        // NEEDSWORK: According to Karin, it would be much more efficient to pass all the points in the centerline as a single array.  
        //    Need to restructure Centerline to do store them separately.
        if (1 == nPts || (2 == nPts && points->IsEqual (points[1])))
            output.DrawPointString3d (1, points, NULL);
        else
            output.DrawLineString3d (nPts, points, NULL);
        return;
        }

    int     capMode   = (int)pStroke->GetCapMode();

    if (capMode > CAP_Extended)
        capMode = CAP_Arc;

    if (CAP_Extended == capMode)
        extendEndPoints (m_pts, m_widths, nPts);

    if (m_taper)
        startTangent = endTangent = NULL;

    // see if any of the joints are too sharp and need to be mitred.
    int     start = 0;
    if (nPts> 2)
        {
        DPoint3d normal1, normal2;

        normal1.NormalizedDifference (*m_pts, m_pts[1]);
        for (int i=1; i<nPts-1; i++)
            {
            normal2.NormalizedDifference (m_pts[i], m_pts[i+1]);
            if (TOLERANCE_MitreLimit > normal1.DotProduct (normal2))
                {
                outputPolygon (&output, i-start+1, m_pts+start, m_widths+start, widthMode, capMode, normal, startTangent, NULL, false);
                startTangent = NULL;
                start = i;
                }

            normal1 = normal2;
            }
        }

    bool    polyLengthNotDiscernible = (nPts > 2 ? false : segmentNotDiscernible (context, nPts, m_pts+start));
    outputPolygon (&output, nPts-start, m_pts+start, m_widths+start, widthMode, capMode, normal, startTangent, endTangent, polyLengthNotDiscernible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    getParallelandPerpVecs (DVec3dR parvec, DPoint3dR perpvec, DPoint3dCR normal, DPoint3dCP lineseg)
    {
    parvec.DifferenceOf (*lineseg, lineseg[1]);
    if (0.0 == perpvec.NormalizedCrossProduct (parvec, *((DVec3d*) &normal)))
        perpvec.x = 1.0;   // zero length bvector
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
static inline double    getPerpScale (DPoint3dCP perp1, DPoint3dCP perp2)
    {
    static const double MAX_PERP_SCALE = 50.0;

    double dot = perp1->DotProduct (*perp2);
    return  fabs (dot) < 1.0/MAX_PERP_SCALE ? MAX_PERP_SCALE : 1.0 / dot;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineJoint::FromVertices
(
LineJoint* joints,          // <= LineJoints
DPoint3dCP points,         // => Line vertices
int        nPoints,        // => Number of vertices
DPoint3dCP normal,         // => line face surface normal
DPoint3dCP pStartTangent,
DPoint3dCP pEndTangent
)
    {
    LineJoint* joint = joints;

    DVec3d parVec;
    getParallelandPerpVecs (parVec, joint->m_dir, *normal, points);
    joint->m_scale = 1.0;

    if (NULL != pStartTangent)
        {
        DVec3d cross;
        cross.CrossProduct (*pStartTangent, parVec);

        // if the start tangent is not parallel to the normal bvector, we're "twisting" from element-to-element. Skip it.
        if (cross.IsParallelTo (*((DVec3d*)normal)))
            {
            DPoint3d    perpVec = joint->m_dir;
            joint->m_dir.NormalizedCrossProduct (*normal, *pStartTangent);
            joint->m_scale = getPerpScale (&perpVec, &joint->m_dir);
            }
        }

    for (int i=1; i<nPoints-1; i++)
        {
        DPoint3d    perpVec1, perpVec2;
        getParallelandPerpVecs (parVec, perpVec1, *normal, points);
        points++;
        getParallelandPerpVecs (parVec, perpVec2, *normal, points);

        joint++;

        joint->m_dir.Interpolate (perpVec1, 0.5, perpVec2);
        joint->m_dir.Normalize ();
        joint->m_scale = getPerpScale (&perpVec2, &joint->m_dir);
        }

    joint++;
    getParallelandPerpVecs (parVec, joint->m_dir, *normal, points);
    joint->m_scale = 1.0;

    if (NULL != pEndTangent)
        {
        DVec3d cross;
        cross.CrossProduct (*pEndTangent, parVec);

        if (cross.IsParallelTo (*((DVec3d*)normal)))
            {
            DPoint3d    perpVec = joint->m_dir;
            joint->m_dir.NormalizedCrossProduct (*normal, *pEndTangent);
            joint->m_scale = getPerpScale (&perpVec, &joint->m_dir);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsStrokePatternComponent::_GetForTextureGeneration() const
    {
    LsOkayForTextureGeneration isOkay = _IsOkayForTextureGeneration();
    BeAssert(LsOkayForTextureGeneration::NotAllowed != isOkay);   //  The caller should have tested for this.

    if (isOkay == LsOkayForTextureGeneration::NoChangeRequired)
        return const_cast<LsStrokePatternComponentP>(this);

    LsStrokePatternComponentP retval = new LsStrokePatternComponent(this);

    if (GetPhaseMode() != LsStrokePatternComponent::PHASEMODE_Fixed)
        retval->SetDistancePhase(0.0);


    for (size_t i = 0; i < retval->m_nStrokes; ++i)
        {
        LsStroke& stroke(*(retval->m_strokes + i));
        if (stroke.GetCapMode() != LsCapMode::Open)
            stroke.SetCapMode(LsCapMode::Closed);

        stroke.SetIsStretchable(false);
        //  end conditions are not enabled so it should not be necessary to mess with dash-first, etc.
        //  Since we draw exactly one iteration for generating the texture, corner mode (IsRigid) should not be important.
        }

    return retval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
LsOkayForTextureGeneration LsStrokePatternComponent::_IsOkayForTextureGeneration() const 
    {
    if (m_okayForTextureGeneration != Dgn::LsOkayForTextureGeneration::Unknown)
        return m_okayForTextureGeneration;

    m_okayForTextureGeneration = LsOkayForTextureGeneration::NoChangeRequired; 

    if (HasIterationLimit())
        return m_okayForTextureGeneration = LsOkayForTextureGeneration::NotAllowed;

    //  Need to verify that fixed with a distance != 0 is okay.
    if (GetPhaseMode() != LsStrokePatternComponent::PHASEMODE_Fixed)
        m_okayForTextureGeneration = LsOkayForTextureGeneration::ChangeRequired;

    for (uint32_t i = 0; i < m_nStrokes; ++i)
        {
        LsStroke const& stroke(*(m_strokes+i));
        
        if (stroke.IsStretchable() || (stroke.GetCapMode() != LsCapMode::Closed && stroke.GetCapMode() != LsCapMode::Open))
            UpdateLsOkayForTextureGeneration(m_okayForTextureGeneration, LsOkayForTextureGeneration::ChangeRequired);
        }

    return m_okayForTextureGeneration;
    }

