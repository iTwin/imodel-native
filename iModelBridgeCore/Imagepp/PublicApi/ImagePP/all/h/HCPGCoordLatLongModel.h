/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HCPGCoordLatLongModel.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <ImagePP\all\h\HGFException.h>
#include <ImagePP\all\h\HGF2DLiteExtent.h>

#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <ImagePP/all/h/interface/IRasterGeoCoordinateServices.h>

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

        IMAGEPP_EXPORT              HCPGCoordLatLongModel(IRasterBaseGcsR pi_SourceGEOCS);


                                    HCPGCoordLatLongModel(const HCPGCoordLatLongModel& pi_rObj);
        IMAGEPP_EXPORT virtual                     ~HCPGCoordLatLongModel();
        HCPGCoordLatLongModel&      operator=(const HCPGCoordLatLongModel& pi_rObj);


        virtual bool IsConvertDirectThreadSafe() const override {return false;}
        virtual bool IsConvertInverseThreadSafe() const override {return false;}

        // HGF2DTransfoModel interface

        // Conversion interface
        virtual StatusInt           ConvertDirect(double*   pio_pXInOut,
                                                  double*   pio_pYInOut) const override;

        virtual StatusInt           ConvertDirect(double    pi_YIn,
                                                  double    pi_XInStart,
                                                  size_t     pi_NumLoc,
                                                  double    pi_XInStep,
                                                  double*   po_pXOut,
                                                  double*   po_pYOut) const override;

        virtual StatusInt           ConvertDirect(double    pi_XIn,
                                                  double    pi_YIn,
                                                  double*   po_pXOut,
                                                  double*   po_pYOut) const override;

        virtual StatusInt           ConvertDirect(size_t    pi_NumLoc,
                                                  double*   pio_aXInOut,
                                                  double*   pio_aYInOut) const override;

        virtual StatusInt           ConvertInverse(double*   pio_pXInOut,
                                                   double*   pio_pYInOut) const override;

        virtual StatusInt           ConvertInverse(double    pi_YIn,
                                                   double    pi_XInStart,
                                                   size_t     pi_NumLoc,
                                                   double    pi_XInStep,
                                                   double*   po_pXOut,
                                                   double*   po_pYOut) const override;

        virtual StatusInt           ConvertInverse(double    pi_XIn,
                                                   double    pi_YIn,
                                                   double*   po_pXOut,
                                                   double*   po_pYOut) const override;

        virtual StatusInt           ConvertInverse(size_t    pi_NumLoc,
                                                   double*   pio_aXInOut,
                                                   double*   pio_aYInOut) const override;

        // Miscalenious
        virtual bool                IsIdentity      () const;
        virtual bool                IsStretchable   (double pi_AngleTolerance = 0) const;
        virtual void                GetStretchParams(double*           po_pScaleFactorX,
                                                     double*           po_pScaleFactorY,
                                                     HGF2DDisplacement* po_pDisplacement) const;

        virtual HGF2DTransfoModel* Clone () const override;
        virtual HFCPtr<HGF2DTransfoModel>
                                    ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const;


        // Model definition
        virtual bool                CanBeRepresentedByAMatrix() const;
        virtual HFCMatrix<3, 3>     GetMatrix() const;

        virtual HFCPtr<HGF2DTransfoModel>
                                    CreateSimplifiedModel() const;

        // Geometric properties
        virtual bool                PreservesLinearity() const;
        virtual bool                PreservesParallelism() const;
        virtual bool                PreservesShape() const;
        virtual bool                PreservesDirection() const;

        // Domain management ... a cartographic reprojection model usually has a limited domain.
        virtual bool                HasDomain() const override;
        virtual HFCPtr<HGF2DShape>  GetDirectDomain() const override;
        virtual HFCPtr<HGF2DShape>  GetInverseDomain() const override;

        // Operations
        virtual void                Reverse ();

    protected:

        virtual void                Prepare ();
        virtual HFCPtr<HGF2DTransfoModel>
                                    ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;
    private:

#ifdef HVERIFYCONTRACT
        void                        ValidateInvariants() const
        {
            HASSERT(m_GEOCS.IsValid());
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
        IRasterBaseGcsR                         m_GEOCS;
        bool                                    m_reversed;


        // Domain related cached members
        mutable bool                            m_domainComputed;
        mutable HFCPtr<HGF2DShape>              m_domainDirect;
        mutable HFCPtr<HGF2DShape>              m_domainInverse;
};

END_IMAGEPP_NAMESPACE
