//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPGCoordModel.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <ImagePP/all/h/interface/IRasterGeoCoordinateServices.h>

BEGIN_IMAGEPP_NAMESPACE

class HGF2DDisplacement;

// ----------------------------------------------------------------------------
//  HCPGCoordModel
// ----------------------------------------------------------------------------
class HCPGCoordModel : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HCPGCoordId_Model, HGF2DTransfoModel)

public:
    // Primary methods
//    IMAGEPP_EXPORT                       HCPGCoordModel();

    IMAGEPP_EXPORT                       HCPGCoordModel(IRasterBaseGcsR pi_SourceGEOCS,
                                                       IRasterBaseGcsR pi_DestinationGEOCS);


    IMAGEPP_EXPORT                       HCPGCoordModel(const HCPGCoordModel& pi_rObj);
    IMAGEPP_EXPORT virtual               ~HCPGCoordModel();
    IMAGEPP_EXPORT HCPGCoordModel&       operator=(const HCPGCoordModel& pi_rObj);

    // GCoord model specific interface
    IMAGEPP_EXPORT IRasterBaseGcsCR      GetSourceGEOCS() const;
    IMAGEPP_EXPORT IRasterBaseGcsCR      GetDestinationGEOCS() const;


    virtual bool IsConvertDirectThreadSafe() const override {return false;}
    virtual bool IsConvertInverseThreadSafe() const override {return false;}

    // HGF2DTransfoModel interface

    // Conversion interface
    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect(double*   pio_pXInOut,
                                                      double*   pio_pYInOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect(double    pi_YIn,
                                                      double    pi_XInStart,
                                                      size_t     pi_NumLoc,
                                                      double    pi_XInStep,
                                                      double*   po_pXOut,
                                                      double*   po_pYOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect(double    pi_XIn,
                                                      double    pi_YIn,
                                                      double*   po_pXOut,
                                                      double*   po_pYOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertDirect(size_t    pi_NumLoc,
                                                       double*   pio_aXInOut,
                                                       double*   pio_aYInOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertInverse(double*   pio_pXInOut,
                                                        double*   pio_pYInOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertInverse(double    pi_YIn,
                                                        double    pi_XInStart,
                                                        size_t     pi_NumLoc,
                                                        double    pi_XInStep,
                                                        double*   po_pXOut,
                                                        double*   po_pYOut) const override;

    IMAGEPP_EXPORT virtual StatusInt     ConvertInverse(double    pi_XIn,
                                                        double    pi_YIn,
                                                        double*   po_pXOut,
                                                        double*   po_pYOut) const override;
											   
    IMAGEPP_EXPORT virtual StatusInt     ConvertInverse(size_t    pi_NumLoc,
                                                       double*   pio_aXInOut,
                                                       double*   pio_aYInOut) const override;

    // Miscellaneous
    IMAGEPP_EXPORT virtual bool          IsIdentity      () const;
    IMAGEPP_EXPORT virtual bool          IsStretchable   (double pi_AngleTolerance = 0) const;
    IMAGEPP_EXPORT virtual void          GetStretchParams(double*           po_pScaleFactorX,
                                                         double*           po_pScaleFactorY,
                                                         HGF2DDisplacement* po_pDisplacement) const;

    IMAGEPP_EXPORT virtual HGF2DTransfoModel* 
                                        Clone () const override;
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DTransfoModel>
                                        ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const;


    // Model definition
    IMAGEPP_EXPORT virtual bool          CanBeRepresentedByAMatrix() const;
    IMAGEPP_EXPORT virtual HFCMatrix<3, 3>
                                        GetMatrix() const;

    IMAGEPP_EXPORT virtual HFCPtr<HGF2DTransfoModel>
                                        CreateSimplifiedModel() const;

    // Geometric properties
    IMAGEPP_EXPORT virtual bool          PreservesLinearity() const;
    IMAGEPP_EXPORT virtual bool          PreservesParallelism() const;
    IMAGEPP_EXPORT virtual bool          PreservesShape() const;
    IMAGEPP_EXPORT virtual bool          PreservesDirection() const;


    // Domain management ... a cartographic reprojection model usually has a limited domain.
    IMAGEPP_EXPORT virtual bool          HasDomain() const override;
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DShape> 
                                        GetDirectDomain() const override;
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DShape> 
                                        GetInverseDomain() const override;

    // Operations
    IMAGEPP_EXPORT virtual void          Reverse ();

protected:

    virtual void                        Prepare ();
    virtual HFCPtr<HGF2DTransfoModel>   ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;
private:
    enum ErrorTolerance
        {
        TOLERANCE_NONE,
        TOLERANCE_WARNING,
        TOLERANCE_ERROR
        };

#ifdef HVERIFYCONTRACT
    void                        ValidateInvariants() const;
#endif

    // Private methods
    void                        Copy (const HCPGCoordModel& pi_rObj);


    StatusInt                   ComputeDomain () const;

    // Primary attributes
    IRasterBaseGcsPtr     m_SourceGEOCS;
    IRasterBaseGcsPtr     m_DestinationGEOCS;

    // Domain related cached members
    mutable bool                            m_domainComputed;
    mutable HFCPtr<HGF2DShape>              m_domainDirect;
    mutable HFCPtr<HGF2DShape>              m_domainInverse;

    };

END_IMAGEPP_NAMESPACE
