/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/AreaPattern.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#define  MAX_DWG_HATCH_LINE_DASHES      20

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
 @addtogroup AreaPattern
 @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DwgHatchDefLine
{
    double      m_angle;
    DPoint2d    m_through;
    DPoint2d    m_offset;
    short       m_nDashes;
    double      m_dashes[MAX_DWG_HATCH_LINE_DASHES];
};

//=======================================================================================
//! The PatternParams structure defines a hatch, cross hatch, or area pattern.
//! @ingroup AreaPattern
//=======================================================================================
struct PatternParams : RefCountedBase
{
protected:

    DPoint3d                    m_origin;               //!< Pattern origin (offset from to element's placement).
    RotMatrix                   m_rMatrix;              //!< Pattern coordinate system (relative to element's placement).
    double                      m_space1;               //!< Primary (row) spacing.
    double                      m_space2;               //!< Secondary (column) spacing.
    double                      m_angle1;               //!< Angle of first hatch or pattern.
    double                      m_angle2;               //!< Angle of second hatch.
    double                      m_scale;                //!< Pattern scale.
    bool                        m_invisibleBoundary;    //!< Whether pattern boundary should not display (ignored when also filled)...
    bool                        m_snappable;            //!< Whether pattern geometry can be snapped to.
    bool                        m_useColor;             //!< Whether to use pattern color instead of inheriting current color.
    bool                        m_useWeight;            //!< Whether to use pattern weight instead of inheriting current weight.
    ColorDef                    m_color;                //!< The pattern / hatch color.
    uint32_t                    m_weight;               //!< The pattern / hatch weight.
    DgnGeometryPartId           m_symbolId;             //!< The id of the GeometryPart to use for an area pattern.
    bvector<DwgHatchDefLine>    m_hatchLines;           //!< The DWG style hatch definition.

    PatternParams()
        {
        m_origin.Zero();
        m_rMatrix.InitIdentity();
        m_space1 = m_space2 = m_angle1 = m_angle2 = 0.0;
        m_scale = 1.0;
        m_useColor = m_useWeight = m_invisibleBoundary = m_snappable = false;
        m_weight = 0;
        }

public:
    //! Create an instance of a PatternParams.
    static PatternParamsPtr Create() {return new PatternParams();}

    //! Create an instance of a PatternParams from an existing PatternParams.
    static PatternParamsPtr Create(PatternParamsCR params)
        {
        PatternParamsP newParams = new PatternParams();

        newParams->m_origin             = params.m_origin;
        newParams->m_rMatrix            = params.m_rMatrix;
        newParams->m_space1             = params.m_space1;
        newParams->m_space2             = params.m_space2;
        newParams->m_angle1             = params.m_angle1;
        newParams->m_angle2             = params.m_angle2;
        newParams->m_scale              = params.m_scale;
        newParams->m_invisibleBoundary  = params.m_invisibleBoundary;
        newParams->m_snappable          = params.m_snappable;
        newParams->m_useColor           = params.m_useColor;
        newParams->m_useWeight          = params.m_useWeight;
        newParams->m_color              = params.m_color;
        newParams->m_weight             = params.m_weight;
        newParams->m_symbolId           = params.m_symbolId;
        newParams->m_hatchLines         = params.m_hatchLines;

        return newParams;
        }

    //! Compare two PatternParams.
    bool operator==(PatternParamsCR rhs) const
        {
        if (this == &rhs)
            return true;

        if (!rhs.m_origin.IsEqual(m_origin, 1.0e-10))
            return false;

        if (!rhs.m_rMatrix.IsEqual(m_rMatrix, 1.0e-10))
            return false;

        if (!DoubleOps::WithinTolerance(rhs.m_space1, m_space1, 1.0e-10))
            return false;

        if (!DoubleOps::WithinTolerance(rhs.m_space2, m_space2, 1.0e-10))
            return false;

        if (!DoubleOps::WithinTolerance(rhs.m_angle1, m_angle1, 1.0e-10))
            return false;

        if (!DoubleOps::WithinTolerance(rhs.m_angle2, m_angle2, 1.0e-10))
            return false;

        if (!DoubleOps::WithinTolerance(rhs.m_scale, m_scale, 1.0e-10))
            return false;

        if (rhs.m_invisibleBoundary != m_invisibleBoundary)
            return false;

        if (rhs.m_snappable != m_snappable)
            return false;

        if (rhs.m_useColor != m_useColor)
            return false;
        else if (m_useColor && rhs.m_color != m_color)
            return false;

        if (rhs.m_useWeight != m_useWeight)
            return false;
        else if (m_useWeight && rhs.m_weight != m_weight)
            return false;

        if (rhs.m_symbolId.GetValueUnchecked() != m_symbolId.GetValueUnchecked())
            return false;

        if (rhs.m_hatchLines.size() != m_hatchLines.size())
            return false;

        for (size_t iLine=0; iLine < m_hatchLines.size(); ++iLine)
            {
            DwgHatchDefLine rhsLine = rhs.m_hatchLines.at(iLine);
            DwgHatchDefLine line = m_hatchLines.at(iLine);

            if (rhsLine.m_nDashes != line.m_nDashes)
                return false;

            if (!DoubleOps::WithinTolerance(rhsLine.m_angle, line.m_angle, 1.0e-10))
                return false;

            if (!rhsLine.m_through.IsEqual(line.m_through, 1.0e-10))
                return false;

            if (!rhsLine.m_offset.IsEqual(line.m_offset, 1.0e-10))
                return false;

            for (int iDash=0; iDash < line.m_nDashes; ++iDash)
                {
                if (!DoubleOps::WithinTolerance(rhsLine.m_dashes[iDash], line.m_dashes[iDash], 1.0e-10))
                    return false;
                }
            }

        return true;
        }

    //! Get pattern orientation relative to element's origin.
    RotMatrixCR GetOrientation() const {return m_rMatrix;}

    //! Get pattern offset from element's origin.
    DPoint3dCR GetOrigin() const {return m_origin;}

    //! Get pattern hatch spacing or area pattern row spacing.
    double GetPrimarySpacing() const {return m_space1;}

    //! Get pattern cross hatch spacing or area pattern column spacing.
    double GetSecondarySpacing() const {return m_space2;}

    //! Get pattern hatch angle or area pattern angle.
    double GetPrimaryAngle() const {return m_angle1;}

    //! Get pattern cross hatch angle.
    double GetSecondaryAngle() const {return m_angle2;}

    //! Get pattern scale.
    double GetScale() const {return m_scale;}

    //! Get pattern symbol id.
    DgnGeometryPartId GetSymbolId() const {return m_symbolId;}

    //! Get DWG hatch definition.
    bvector<DwgHatchDefLine> const& GetDwgHatchDef() const {return m_hatchLines;}
    bvector<DwgHatchDefLine>& GetDwgHatchDefR() {return m_hatchLines;}

    //! Get whether pattern boundary should not display (ignored when also filled)...
    bool GetInvisibleBoundary() const {return m_invisibleBoundary;}

    //! Get whether pattern boundary can be snapped to.
    bool GetSnappable() const {return m_snappable;}

    //! Get whether pattern uses a fixed color.
    bool GetUseColor() const {return m_useColor;}

    //! Get whether pattern uses a fixed weight.
    bool GetUseWeight() const {return m_useWeight;}

    //! Get pattern color. Used if m_useColor is set.
    //! @note Uses element color if not set.
    ColorDef GetColor() const {return m_color;}

    //! Get pattern weight. Used if m_useWeight is set.
    //! @note Uses element weight if not set.
    uint32_t GetWeight() const {return m_weight;}

    //! Set pattern orientation.
    void SetOrientation(RotMatrixCR value) {m_rMatrix = value;}

    //! Set pattern offset from element origin.
    void SetOrigin(DPoint3dCR value) {m_origin = value;}

    //! Set pattern hatch spacing or area pattern row spacing.
    void SetPrimarySpacing(double value) {m_space1 = value;}

    //! Set pattern cross hatch spacing or area pattern column spacing.
    void SetSecondarySpacing(double value) {m_space2 = value;}

    //! Set pattern hatch angle or area pattern angle.
    void SetPrimaryAngle(double value) {m_angle1 = value;}

    //! Set pattern cross hatch angle.
    void SetSecondaryAngle(double value) {m_angle2 = value;}

    //! Set pattern pattern scale.
    void SetScale(double value) {m_scale = value;}

    //! Set pattern symbol id.
    void SetSymbolId(DgnGeometryPartId value) {m_symbolId = value;}

    //! Set DWG hatch definition.
    void SetDwgHatchDef(bvector<DwgHatchDefLine> const& value) {m_hatchLines = value;}

    //! Set whether pattern boundary should not display (ignored when also filled)...
    void SetInvisibleBoundary(bool value) {m_invisibleBoundary = value;}

    //! Set whether pattern boundary can be snapped to.
    void SetSnappable(bool value) {m_snappable = value;}

    //! Set pattern color.
    //! @note Use to set a pattern color that is different than element color.
    void SetColor(ColorDef value) {m_color = value; m_useColor = true;}

    //! Set pattern weight.
    //! @note Use to set a pattern weight that is different than element weight.
    void SetWeight(uint32_t value) {m_weight = value; m_useWeight = true;}

    //! Modify this PatternParams by the supplied transform.
    DGNPLATFORM_EXPORT void ApplyTransform(TransformCR transform);

}; // PatternParams

END_BENTLEY_DGN_NAMESPACE

