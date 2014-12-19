//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DBoundaryModel.hpp $
//:>
//:>  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
inline HGF2DLiteExtent HGF2DBoundaryModel::GetOuterExtent() const
    {
    return m_outerExtent;
    }

inline HFCPtr<HGF2DTransfoModel> HGF2DBoundaryModel::GetOuterModel() const
    {
    return m_pOuterModel;
    }
