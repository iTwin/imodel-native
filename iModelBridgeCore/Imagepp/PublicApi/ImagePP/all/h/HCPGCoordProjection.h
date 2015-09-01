//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPGCoordProjection.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>

BEGIN_IMAGEPP_NAMESPACE

class HCPGeoTiffKeys;

// ----------------------------------------------------------------------------
//  HCPGCoordProjection
// ----------------------------------------------------------------------------
class HCPGCoordProjection : public HFCShareableObject<HCPGCoordProjection>
    {
public:
    // Constructors
    IMAGEPP_EXPORT explicit        HCPGCoordProjection (HFCPtr<HCPGeoTiffKeys> const& pi_pGeoTiffKeys);
    IMAGEPP_EXPORT explicit        HCPGCoordProjection(IRasterBaseGcsCP pi_pBaseGeoCoord);


    //Copy constructor
    IMAGEPP_EXPORT                 HCPGCoordProjection(const HCPGCoordProjection& pi_rObj);
    IMAGEPP_EXPORT HCPGCoordProjection& operator=(const HCPGCoordProjection& pi_rObj);

    //Destructor
    IMAGEPP_EXPORT virtual         ~HCPGCoordProjection();

    // Information extraction
    IMAGEPP_EXPORT double GetUnit () const;

    // Validity
    IMAGEPP_EXPORT bool           IsValid() const;

    IMAGEPP_EXPORT IRasterBaseGcsCP GetBaseGCS() const;

protected:

private:
    // Disbaled method
    HCPGCoordProjection();
    bool            operator== (HCPGCoordProjection const& pi_rObj) const;
    bool            operator!= (HCPGCoordProjection const& pi_rObj) const;


    void InitFromGeoTiffKeys(HFCPtr<HCPGeoTiffKeys> const& pi_pGeoTiffKeys);
    void InitFromGeoWellKnownText(unsigned short pi_WktFlavor, WCharCP pi_pWellKnownText);

#ifdef HVERIFYCONTRACT
    void            ValidateInvariants() const
        {
        }
#endif

    // Private member
    IRasterBaseGcsPtr m_pBaseGeoCoord;
    };

END_IMAGEPP_NAMESPACE