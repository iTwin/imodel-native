//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HGF2DTransfoModel.h>

#ifndef VANCOUVER_API
#define TRANSFOMODEL ImagePP::HGF2DTransfoModel
#define DISPLACEMENT ImagePP::HGF2DDisplacement
#else
#define TRANSFOMODEL HGF2DTransfoModel
#define DISPLACEMENT HGF2DDisplacement
#define RefCountedCPtr RefCountedPtr
#endif
USING_NAMESPACE_IMAGEPP
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

#if defined(VANCOUVER_API)
    #define CONVERT_RETURN_TYPE void
#else
    #define CONVERT_RETURN_TYPE StatusInt
#endif

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

        

        GeoCoordinates::BaseGCSCR GetSourceGCS() const;
        GeoCoordinates::BaseGCSCR                  GetDestinationGCS() const;

        
        // HGF2DTransfoModel interface

        // Conversion interface
#if defined(VANCOUVER_API) || defined(DGNDB06_API)
        virtual CONVERT_RETURN_TYPE ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const override;
        virtual CONVERT_RETURN_TYPE ConvertDirect(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const override;
        virtual CONVERT_RETURN_TYPE ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const override;
        virtual CONVERT_RETURN_TYPE ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const;
		
		virtual CONVERT_RETURN_TYPE ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const ;
        virtual CONVERT_RETURN_TYPE ConvertInverse(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const ;
        virtual CONVERT_RETURN_TYPE ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const ;
        virtual CONVERT_RETURN_TYPE ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const ;

#if defined(DGNDB06_API)
        virtual bool IsConvertDirectThreadSafe() const override { return false; }
        virtual bool IsConvertInverseThreadSafe() const override { return false; }
#endif

        virtual TRANSFOMODEL* Clone() const override;

        virtual HFCMatrix<3, 3>  GetMatrix() const override;

        virtual bool IsStretchable(double pi_AngleTolerance = 0) const override { return false; }
        virtual void GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY, DISPLACEMENT* po_pDisplacement) const override;

        virtual HFCPtr<TRANSFOMODEL> ComposeInverseWithDirectOf(const TRANSFOMODEL& pi_rModel) const override;

        virtual bool                CanBeRepresentedByAMatrix() const override { return false; }


        virtual HFCPtr<TRANSFOMODEL> CreateSimplifiedModel() const override;

        virtual bool          PreservesLinearity() const override { return false; }
        virtual bool          PreservesParallelism() const override { return false; }
        virtual bool          PreservesShape() const override { return false; }
        virtual bool          PreservesDirection() const override { return false; }

        // Domain management ... a cartographic reprojection model usually has a limited domain.
        virtual bool                                    HasDomain() const  { return true; }
        virtual HFCPtr<HGF2DShape>    GetDirectDomain() const;
        virtual HFCPtr<HGF2DShape>    GetInverseDomain() const;

        // Operations
        virtual void Reverse();

#else       
        virtual StatusInt _ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const override;
        virtual StatusInt _ConvertDirect(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const override;
        virtual StatusInt _ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const override;
        virtual StatusInt _ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const override;
		
		virtual StatusInt _ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const ;
        virtual StatusInt _ConvertInverse(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const ;
        virtual StatusInt _ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const ;
        virtual StatusInt _ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const ;

        virtual bool _IsConvertDirectThreadSafe() const override { return false; }
        virtual bool _IsConvertInverseThreadSafe() const override { return false; }

        virtual TRANSFOMODEL* _Clone() const override;

        virtual HFCMatrix<3, 3>  _GetMatrix() const override;

        virtual bool _IsStretchable(double pi_AngleTolerance = 0) const override { return false; }
        virtual void _GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY, DISPLACEMENT* po_pDisplacement) const override;

        virtual HFCPtr<TRANSFOMODEL> _ComposeInverseWithDirectOf(const TRANSFOMODEL& pi_rModel) const override;

        virtual bool                _CanBeRepresentedByAMatrix() const override { return false; }


        virtual HFCPtr<TRANSFOMODEL> _CreateSimplifiedModel() const override;

        virtual bool          _PreservesLinearity() const override { return false; }
        virtual bool          _PreservesParallelism() const override { return false; }
        virtual bool          _PreservesShape() const override { return false; }
        virtual bool          _PreservesDirection() const override { return false; }

        // Domain management ... a cartographic reprojection model usually has a limited domain.
        virtual bool                  _HasDomain() const override { return true; }
        virtual HFCPtr<HGF2DShape>    _GetDirectDomain() const override;
        virtual HFCPtr<HGF2DShape>    _GetInverseDomain() const override;

        // Operations
        virtual void _Reverse() override;
#endif

    protected:

#if defined(VANCOUVER_API) || defined(DGNDB06_API)
        virtual void  Prepare() override {};
        virtual HFCPtr<TRANSFOMODEL>   ComposeYourself(const TRANSFOMODEL& pi_rModel) const;
#else        
        virtual void  _Prepare() override {};
        virtual HFCPtr<TRANSFOMODEL>   _ComposeYourself(const TRANSFOMODEL& pi_rModel) const override;
#endif

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
