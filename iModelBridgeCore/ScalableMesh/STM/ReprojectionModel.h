//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ReprojectionModel.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HGF2DTransfoModel.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
class ReprojectionModel : public ImagePP::HGF2DTransfoModel
    {
    public:

        ReprojectionModel(GeoCoordinates::BaseGCSCR source, GeoCoordinates::BaseGCSCR destination);
        virtual ~ReprojectionModel();

        ReprojectionModel(const ReprojectionModel& obj);
        ReprojectionModel& operator=(const ReprojectionModel& obj) = delete;

        virtual HGF2DTransfoModel* Clone() const override;

        GeoCoordinates::BaseGCSCR GetSourceGCS() const;
        GeoCoordinates::BaseGCSCR                  GetDestinationGCS() const;

        virtual bool IsConvertDirectThreadSafe() const override { return false; }
        virtual bool IsConvertInverseThreadSafe() const override { return false; }

        // HGF2DTransfoModel interface

        // Conversion interface
        virtual StatusInt ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const override;
        virtual StatusInt ConvertDirect(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const override;
        virtual StatusInt ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const override;
        virtual StatusInt ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const override;

        virtual StatusInt ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const override;
        virtual StatusInt ConvertInverse(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const override;
        virtual StatusInt ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const override;
        virtual StatusInt ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const override;

        virtual bool IsStretchable(double pi_AngleTolerance = 0) const override { return false; }
        virtual void GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY, ImagePP::HGF2DDisplacement* po_pDisplacement) const override;

        virtual ImagePP::HFCPtr<ImagePP::HGF2DTransfoModel> ComposeInverseWithDirectOf(const ImagePP::HGF2DTransfoModel& pi_rModel) const override;

        virtual bool                CanBeRepresentedByAMatrix() const override { return false; }
        virtual ImagePP::HFCMatrix<3, 3>     GetMatrix() const override;

        virtual ImagePP::HFCPtr<ImagePP::HGF2DTransfoModel> CreateSimplifiedModel() const override;

        virtual bool          PreservesLinearity() const override { return false; }
        virtual bool          PreservesParallelism() const override { return false; }
        virtual bool          PreservesShape() const override { return false; }
        virtual bool          PreservesDirection() const override { return false; }

        // Domain management ... a cartographic reprojection model usually has a limited domain.
        virtual bool                                    HasDomain() const override { return true; }
        virtual ImagePP::HFCPtr<ImagePP::HGF2DShape>    GetDirectDomain() const override;
        virtual ImagePP::HFCPtr<ImagePP::HGF2DShape>    GetInverseDomain() const override;

        // Operations
        virtual void Reverse();

    protected:

        virtual void  Prepare() override {};
        virtual ImagePP::HFCPtr<ImagePP::HGF2DTransfoModel>   ComposeYourself(const ImagePP::HGF2DTransfoModel& pi_rModel) const;

    private:

        StatusInt Reproject(double& pio_XInOut, double& pio_YInOut, bool inverse) const;

        StatusInt  ComputeDomain() const;

        bool m_isReverse;

        // Primary attributes
        RefCountedCPtr<GeoCoordinates::BaseGCS> m_pSrcGCS;
        RefCountedCPtr<GeoCoordinates::BaseGCS>                  m_pDestGCS;
        double                                  m_srcUnitsFromMeters;

        // Domain related cached members
        mutable bool m_domainComputed;
        mutable ImagePP::HFCPtr<ImagePP::HGF2DShape>  m_domainDirect;
        mutable ImagePP::HFCPtr<ImagePP::HGF2DShape>  m_domainInverse;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
