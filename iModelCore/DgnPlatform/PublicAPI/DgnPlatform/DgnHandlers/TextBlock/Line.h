/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/Line.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include "TextAPICommon.h"
#include "TextBlockNode.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    PaulChater      01/03
//=======================================================================================
struct Line : public TextBlockNode
    {
    DEFINE_T_SUPER(TextBlockNode)
    friend struct  Paragraph;
    friend struct  TextBlock;

    private:    RunArray    m_runArray;
    private:    double      m_maxUnitHeight;
    private:    double      m_maxUnitWidth;
    private:    double      m_maxAscender;
    private:    double      m_maxDescender;
    private:    double      m_lowestUnitY;
    private:    double      m_maxDistanceAboveBaseline;
    private:    double      m_maxExactDistanceAboveOrigin;
    private:    double      m_maxUnitOffset;
    private:    double      m_maxExactDepthBelowOrigin;
    private:    DPoint2d    m_baselineDisplacement;
    private:    bool        m_isFullLine;
    private:    double      m_maxHorizontalCellIncrement;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:                                                     Line                            ();
    public:                                                     Line                            (LineCR);
    public:                         virtual                     ~Line                           ();

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    private:                                void                ComputeLineInformation          (TextBlockCR);
    private:                                void                GetFractionAlignment            (size_t startIndex, StackedFractionAlignment&, bool& fractionsPresent) const;
    private:                                bool                AlignUnits                      (double nodeNumberHeight, DgnLineSpacingType);
    private:                                void                SetFullLine                     (bool);
    private:                                AppendStatus        AppendToLine                    (RunP currentRun, RunP nextRun, TextBlockNodeArrayR, ProcessContextCR);
    private:                                void                UpdateRunsForEndOfLineFlag      ();

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    protected:                      virtual TextBlockNodeLevel  _GetUnitLevel                   () const override;
    protected:                      virtual double              _GetExactWidth                  () const override;
    protected:                      virtual double              _GetExactHeight                 () const override;
    protected:                      virtual void                _Drop                           (TextBlockNodeArrayR unitArray) override;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:                                 double              GetMaxUnitWidth                 () const;
    public:     DGNPLATFORM_EXPORT          double              GetMaxUnitHeight                () const;
    public:                                 double              GetMaxAscender                  () const;
    public:     DGNPLATFORM_EXPORT          double              GetMaxDescender                 () const;
    public:                                 double              GetLowestUnitY                  () const;
    public:     DGNPLATFORM_EXPORT          double              GetMaxDistanceAboveBaseline     () const;
    public:                                 double              GetMaxExactHeightAboveOrigin    () const;
    public:                                 double              GetMaxExactDepthBelowOrigin     () const;
    public:                                 double              GetMaxUnitOffset                () const;
    public:                                 double              GetMaxHorizontalCellIncrement   () const;
    public:                                 size_t              GetNumberOfChars                () const;
    public:                                 bool                IsWrappedLine                   () const;
    public:     DGNPLATFORM_EXPORT          bool                EndsInParagraphBreak            () const;
    public:     DGNPLATFORM_EXPORT          bool                EndsInLineBreak                 () const;
    public:                                 bool                IsComplete                      (RunR, ProcessContextCR);
    public:     DGNPLATFORM_EXPORT          bool                IsEmpty                         () const;
    public:     DGNPLATFORM_EXPORT          bool                IsBlankLine                     () const;
    public:                                 bool                IsSpaceBeforeParagraphBreak     () const;
    public:                                 bool                AllowTrailingWordBreak          () const;
    public:                                 void                AddRun                          (RunP);
    public:                                 void                ExtractNodes                    (CaretCR, TextBlockNodeArrayR);
    public:                                 AppendStatus        AppendNodes                     (TextBlockNodeArrayR, ProcessContextR);
    public:                                 void                Draw                            (ViewContextR, bool isViewIndependent, TextBlockDrawOptionsCR) const;
    public:     DGNPLATFORM_EXPORT          size_t              GetRunCount                     () const;
    public:     DGNPLATFORM_EXPORT          RunP                GetRun                          (size_t) const;
    public:                                 BentleyStatus       ComputeCaretAtLocation          (CaretR, DPoint3dCR, bool isVertical, bool isStrict) const;
    public:                                 void                ComputeCaretParameters          (DPoint3dR location, DVec3dR direction, CaretCR) const;
    public:                                 double              ComputeLeftEdgeAlignDistance    () const;
    public:                                 void                ComputeBaselineDisplacement     (DVec3dR baselineDisplacement, DgnLineSpacingType, TextBlockCR) const;
    public:                                 void                SetBaselineAdjustedOrigin       (DPoint3dCR origin, TextBlockCR, DgnLineSpacingType);
    public:                                 void                GetBaselineAdjustedOrigin       (DPoint3dR origin, TextBlockCR, DgnLineSpacingType) const;
    public:                                 void                ComputeRange                    (bool recomputeComponentRanges, DgnLineSpacingType, double nodeNumberHeight);
    public:                                 void                FullJustify                     (double unusedSpace, bool vertical);
    public:                                 bool                ContainsOnlyWhitespace          () const;
    public:                                 bool                Equals                          (LineCR, TextBlockCompareOptionsCR) const;
    public:                                 void                SplitRunInPlace                 (CaretCR);
    public:                                 void                ComputeTransformedHitTestRange  (DRange3dR) const;
    public:                                 void                ComputeElementRange             (DRange3dR) const;

    }; // Line

END_BENTLEY_DGNPLATFORM_NAMESPACE
