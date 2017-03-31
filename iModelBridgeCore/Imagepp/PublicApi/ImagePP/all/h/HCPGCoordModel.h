//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPGCoordModel.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <ImagePP/all/h/HGF2DTransfoModel.h>

BEGIN_IMAGEPP_NAMESPACE

class HGF2DDisplacement;
class HGF2DStretch;

// ----------------------------------------------------------------------------
//  HCPGCoordModel
// ----------------------------------------------------------------------------
class HCPGCoordModel : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HCPGCoordId_Model, HGF2DTransfoModel)

public:
    // Primary methods
//    IMAGEPP_EXPORT                       HCPGCoordModel();

    IMAGEPP_EXPORT                       HCPGCoordModel(GeoCoordinates::BaseGCSCR pi_SourceGEOCS, GeoCoordinates::BaseGCSCR pi_DestinationGEOCS);


    IMAGEPP_EXPORT                       HCPGCoordModel(const HCPGCoordModel& pi_rObj);
    IMAGEPP_EXPORT virtual               ~HCPGCoordModel();
    IMAGEPP_EXPORT HCPGCoordModel&       operator=(const HCPGCoordModel& pi_rObj);

    // GCoord model specific interface
    IMAGEPP_EXPORT GeoCoordinates::BaseGCSCR GetSourceGEOCS() const;
    IMAGEPP_EXPORT GeoCoordinates::BaseGCSCR GetDestinationGEOCS() const; 

protected:

    virtual bool _IsConvertDirectThreadSafe() const override { return false; }
    virtual bool _IsConvertInverseThreadSafe() const override { return false; }

    // HGF2DTransfoModel interface

    // Conversion interface
    virtual StatusInt     _ConvertDirect(double*   pio_pXInOut,
                                                       double*   pio_pYInOut) const override;

    virtual StatusInt     _ConvertDirect(double    pi_YIn,
                                                       double    pi_XInStart,
                                                       size_t     pi_NumLoc,
                                                       double    pi_XInStep,
                                                       double*   po_pXOut,
                                                       double*   po_pYOut) const override;

    virtual StatusInt     _ConvertDirect(double    pi_XIn,
                                                       double    pi_YIn,
                                                       double*   po_pXOut,
                                                       double*   po_pYOut) const override;

    virtual StatusInt     _ConvertDirect(size_t    pi_NumLoc,
                                                       double*   pio_aXInOut,
                                                       double*   pio_aYInOut) const override;

    virtual StatusInt     _ConvertInverse(double*   pio_pXInOut,
                                                        double*   pio_pYInOut) const override;

    virtual StatusInt     _ConvertInverse(double    pi_YIn,
                                                        double    pi_XInStart,
                                                        size_t     pi_NumLoc,
                                                        double    pi_XInStep,
                                                        double*   po_pXOut,
                                                        double*   po_pYOut) const override;

    virtual StatusInt     _ConvertInverse(double    pi_XIn,
                                                        double    pi_YIn,
                                                        double*   po_pXOut,
                                                        double*   po_pYOut) const override;

    virtual StatusInt     _ConvertInverse(size_t    pi_NumLoc,
                                                        double*   pio_aXInOut,
                                                        double*   pio_aYInOut) const override;

    // Miscellaneous
    virtual bool          _IsIdentity() const override;
    virtual bool          _IsStretchable(double pi_AngleTolerance) const override;
    virtual void          _GetStretchParams(double*           po_pScaleFactorX,
                                                          double*           po_pScaleFactorY,
                                                          HGF2DDisplacement* po_pDisplacement) const override;

    virtual HGF2DTransfoModel* _Clone() const override;
    virtual HFCPtr<HGF2DTransfoModel> _ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const override;


    // Model definition
    virtual bool          _CanBeRepresentedByAMatrix() const override;
    virtual HFCMatrix<3, 3> _GetMatrix() const override;

    virtual HFCPtr<HGF2DTransfoModel> _CreateSimplifiedModel() const override;

    // Geometric properties
    virtual bool          _PreservesLinearity() const override;
    virtual bool          _PreservesParallelism() const override;
    virtual bool          _PreservesShape() const override;
    virtual bool          _PreservesDirection() const override;


    // Domain management ... a cartographic reprojection model usually has a limited domain.
    virtual bool                _HasDomain() const override;
    virtual HFCPtr<HGF2DShape>  _GetDirectDomain() const override;
    virtual HFCPtr<HGF2DShape>  _GetInverseDomain() const override;

    // Operations
    virtual void          _Reverse() override;

    virtual void                        _Prepare () override;
    virtual HFCPtr<HGF2DTransfoModel>   _ComposeYourself (const HGF2DTransfoModel& pi_rModel) const override;
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
    RefCountedCPtr<GeoCoordinates::BaseGCS> m_pSrcGCS;
    RefCountedCPtr<GeoCoordinates::BaseGCS> m_pDestGCS;

    // Domain related cached members
    mutable bool                            m_domainComputed;
    mutable HFCPtr<HGF2DShape>              m_domainDirect;
    mutable HFCPtr<HGF2DShape>              m_domainInverse;

    //Pre and post Stretches
    HFCPtr<HGF2DTransfoModel> m_PreStretch;
    HFCPtr<HGF2DTransfoModel> m_PostStretch;


    };

END_IMAGEPP_NAMESPACE
