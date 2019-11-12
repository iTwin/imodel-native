//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
inline HGF2DLiteExtent HGF2DBoundaryModel::GetOuterExtent() const
    {
    return m_outerExtent;
    }

inline HFCPtr<HGF2DTransfoModel> HGF2DBoundaryModel::GetOuterModel() const
    {
    return m_pOuterModel;
    }
END_IMAGEPP_NAMESPACE
