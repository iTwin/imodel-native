/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <ImagePP/all/h/HGFException.h>
#include <ImagePP/all/h/HGF2DLiteExtent.h>
#include <ImagePP/all/h/HGF2DTransfoModel.h>

BEGIN_IMAGEPP_NAMESPACE

class HGF2DDisplacement;


/*=================================================================================**//**
* @bsiclass
*
* @documentation
*
* This helper class implements a transformation model enabling conversion from and to
* the latitude/longitude Geographic coordinate system based upon the provided GCS. It thus
* requires a single GCS to be instantiated.
*
+===============+===============+===============+===============+===============+======*/
class HCPGCoordLatLongModel : public HGF2DTransfoModel
{
    HDECLARE_CLASS_ID(HCPGCoordLatLongModelId_Base, HGF2DTransfoModel)
    
    public:

        // Primary methods
                                    HCPGCoordLatLongModel();

        IMAGEPP_EXPORT              HCPGCoordLatLongModel(GeoCoordinates::BaseGCSCR pi_SourceGEOCS);


                                    HCPGCoordLatLongModel(const HCPGCoordLatLongModel& pi_rObj);
        IMAGEPP_EXPORT virtual                     ~HCPGCoordLatLongModel();
        HCPGCoordLatLongModel&      operator=(const HCPGCoordLatLongModel& pi_rObj);

protected:
        virtual bool _IsConvertDirectThreadSafe() const override {return false;}
        virtual bool _IsConvertInverseThreadSafe() const override {return false;}

        // HGF2DTransfoModel interface

        // Conversion interface
        virtual StatusInt           _ConvertDirect(double*   pio_pXInOut,
                                                  double*   pio_pYInOut) const override;

        virtual StatusInt           _ConvertDirect(double    pi_YIn,
                                                  double    pi_XInStart,
                                                  size_t     pi_NumLoc,
                                                  double    pi_XInStep,
                                                  double*   po_pXOut,
                                                  double*   po_pYOut) const override;

        virtual StatusInt           _ConvertDirect(double    pi_XIn,
                                                  double    pi_YIn,
                                                  double*   po_pXOut,
                                                  double*   po_pYOut) const override;

        virtual StatusInt           _ConvertDirect(size_t    pi_NumLoc,
                                                  double*   pio_aXInOut,
                                                  double*   pio_aYInOut) const override;

        virtual StatusInt           _ConvertInverse(double*   pio_pXInOut,
                                                   double*   pio_pYInOut) const override;

        virtual StatusInt           _ConvertInverse(double    pi_YIn,
                                                   double    pi_XInStart,
                                                   size_t     pi_NumLoc,
                                                   double    pi_XInStep,
                                                   double*   po_pXOut,
                                                   double*   po_pYOut) const override;

        virtual StatusInt           _ConvertInverse(double    pi_XIn,
                                                   double    pi_YIn,
                                                   double*   po_pXOut,
                                                   double*   po_pYOut) const override;

        virtual StatusInt           _ConvertInverse(size_t    pi_NumLoc,
                                                   double*   pio_aXInOut,
                                                   double*   pio_aYInOut) const override;

        // Miscalenious
        virtual bool                _IsIdentity      () const override;
        virtual bool                _IsStretchable   (double pi_AngleTolerance) const override;
        virtual void                _GetStretchParams(double*           po_pScaleFactorX,
                                                     double*           po_pScaleFactorY,
                                                     HGF2DDisplacement* po_pDisplacement) const override;

        virtual HGF2DTransfoModel* _Clone () const override;
        virtual HFCPtr<HGF2DTransfoModel>
            _ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const override;


        // Model definition
        virtual bool                _CanBeRepresentedByAMatrix() const override;
        virtual HFCMatrix<3, 3>     _GetMatrix() const override;

        virtual HFCPtr<HGF2DTransfoModel> _CreateSimplifiedModel() const override;

        // Geometric properties
        virtual bool                _PreservesLinearity() const override;
        virtual bool                _PreservesParallelism() const override;
        virtual bool                _PreservesShape() const override;
        virtual bool                _PreservesDirection() const override;

        // Domain management ... a cartographic reprojection model usually has a limited domain.
        virtual bool                _HasDomain() const override;
        virtual HFCPtr<HGF2DShape>  _GetDirectDomain() const override;
        virtual HFCPtr<HGF2DShape>  _GetInverseDomain() const override;

        // Operations
        virtual void                _Reverse () override;



        virtual void                _Prepare () override;

        virtual HFCPtr<HGF2DTransfoModel> _ComposeYourself (const HGF2DTransfoModel& pi_rModel) const override;
    private:

#ifdef HVERIFYCONTRACT
        void                        ValidateInvariants() const
        {
            HASSERT(m_pBaseGCS.IsValid());
        }
#endif

        // Private methods
        StatusInt ConvertDirectReversible(double  pi_XIn,
                                          double  pi_YIn,
                                          double* po_pXOut,
                                          double* po_pYOut,
                                          bool reverse) const;


        void               Copy (const HCPGCoordLatLongModel& pi_rObj);
    StatusInt                   ComputeDomain () const;



        // Primary attributes
        RefCountedCPtr<GeoCoordinates::BaseGCS> m_pBaseGCS;
        bool                                    m_reversed;


        // Domain related cached members
        mutable bool                            m_domainComputed;
        mutable HFCPtr<HGF2DShape>              m_domainDirect;
        mutable HFCPtr<HGF2DShape>              m_domainInverse;
};

END_IMAGEPP_NAMESPACE
