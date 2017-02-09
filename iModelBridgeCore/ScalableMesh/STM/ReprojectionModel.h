//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ReprojectionModel.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HGF2DTransfoModel.h>

#ifndef VANCOUVER_API
#define TRANSFOMODEL ImagePP::HGF2DTransfoModel
#define DISPLACEMENT ImagePP::HGF2DDisplacement
#else
#define TRANSFOMODEL HGF2DTransfoModel
#define DISPLACEMENT HGF2DDisplacement
#define RefCountedCPtr RefCountedPtr
#endif

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
class ReprojectionModel : public TRANSFOMODEL
    {
    public:

        ReprojectionModel(GeoCoordinates::BaseGCSCR source, GeoCoordinates::BaseGCSCR destination);
        virtual ~ReprojectionModel();

        ReprojectionModel(const ReprojectionModel& obj);
        ReprojectionModel& operator=(const ReprojectionModel& obj) = delete;

        virtual TRANSFOMODEL* Clone() const override;

        GeoCoordinates::BaseGCSCR GetSourceGCS() const;
        GeoCoordinates::BaseGCSCR                  GetDestinationGCS() const;

        virtual bool IsConvertDirectThreadSafe() const  { return false; }
        virtual bool IsConvertInverseThreadSafe() const  { return false; }

        // HGF2DTransfoModel interface

        // Conversion interface
#ifndef VANCOUVER_API
        virtual StatusInt ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const override;
        virtual StatusInt ConvertDirect(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const override;
        virtual StatusInt ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const override;
        virtual StatusInt ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const override;
		
		virtual StatusInt ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const ;
        virtual StatusInt ConvertInverse(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const ;
        virtual StatusInt ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const ;
        virtual StatusInt ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const ;
#else
        virtual void ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const override;
        virtual void ConvertDirect(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const override;
        virtual void ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const override;
        virtual void ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const;
		
		virtual void ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const ;
        virtual void ConvertInverse(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const ;
        virtual void ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const ;
        virtual void ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const ;
#endif



        virtual bool IsStretchable(double pi_AngleTolerance = 0) const  { return false; }
        virtual void GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY, DISPLACEMENT* po_pDisplacement) const ;

        virtual HFCPtr<TRANSFOMODEL> ComposeInverseWithDirectOf(const TRANSFOMODEL& pi_rModel) const ;

        virtual bool                CanBeRepresentedByAMatrix() const  { return false; }
        virtual HFCMatrix<3, 3>     GetMatrix() const override;

        virtual HFCPtr<TRANSFOMODEL> CreateSimplifiedModel() const ;

        virtual bool          PreservesLinearity() const  { return false; }
        virtual bool          PreservesParallelism() const  { return false; }
        virtual bool          PreservesShape() const  { return false; }
        virtual bool          PreservesDirection() const  { return false; }

        // Domain management ... a cartographic reprojection model usually has a limited domain.
        virtual bool                                    HasDomain() const  { return true; }
        virtual HFCPtr<HGF2DShape>    GetDirectDomain() const ;
        virtual HFCPtr<HGF2DShape>    GetInverseDomain() const ;

        // Operations
        virtual void Reverse();

    protected:

        virtual void  Prepare()  {};
        virtual HFCPtr<TRANSFOMODEL>   ComposeYourself(const TRANSFOMODEL& pi_rModel) const;

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
        mutable HFCPtr<HGF2DShape>  m_domainDirect;
        mutable HFCPtr<HGF2DShape>  m_domainInverse;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
