//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPGCoordProjection.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>

class HCPGeoTiffKeys;

// ----------------------------------------------------------------------------
//  HCPGCoordProjection
// ----------------------------------------------------------------------------
class HCPGCoordProjection : public HFCShareableObject<HCPGCoordProjection>
    {
public:
    // Constructors
    _HDLLg explicit        HCPGCoordProjection (HFCPtr<HCPGeoTiffKeys> const& pi_pGeoTiffKeys);
    _HDLLg explicit        HCPGCoordProjection (IRasterBaseGcsPtr const& pi_pBaseGeoCoord);


    //Copy constructor
    _HDLLg                 HCPGCoordProjection(const HCPGCoordProjection& pi_rObj);
    _HDLLg HCPGCoordProjection&
    operator=(const HCPGCoordProjection& pi_rObj);

    //Destructor
    _HDLLg virtual         ~HCPGCoordProjection();

    // Information extraction
    _HDLLg double GetUnit () const;

    // Validity
    _HDLLg bool           IsValid() const;

    _HDLLg IRasterBaseGcsPtr GetBaseGCS() const;

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