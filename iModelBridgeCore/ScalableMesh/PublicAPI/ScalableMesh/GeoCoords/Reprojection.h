/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/GeoCoords/Reprojection.h $
|    $RCSfile: Reprojection.h,v $
|   $Revision: 1.18 $
|       $Date: 2011/12/01 18:51:36 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/GeoCoords/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE

struct GCS;
struct ReprojectionPolicy;
struct Reprojection;
struct TransfoModelBase;
struct TransfoModel;

/*---------------------------------------------------------------------------------**//**
* @description  Factory that creates a reprojection adapted for transforming 
*               coordinates from a source CS to a target CS. This class takes care
*               of every possible combination of source and target CS and returns
*               an error if a combination is found to be impossible or forbidden by
*               the specified policy.
*             
*               Error handling: User is given three choices for dealings with errors:
*                   - No throw interface: choosing the overloads that returns a status
*                     guarantee that no exception will ever be thrown and allow the
*                     user to react on error using traditional statuses. Null reprojection 
*                     will be returned on error.
*                   - Throwing interface: 
*                       - All create methods returning no statuses will throw on errors
*                         by default.
*                       - User may specify in the policy that exceptions normally thrown
*                         by this interface by redirected to the specified Log as an
*                         error. Null reprojection will be returned on error. // TDORAY: Still to come
*
*
* @see Reprojection
* @see ReprojectionPolicy
* @see Foundations::Exception
*
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionFactory : private Unassignable
    {
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        S_QTY,
        };
private:
    struct                                  Impl;
    SharedPtrTypeTrait<const Impl>::type    m_implP;
public:
    GEOCOORDS_DLLE explicit                 ReprojectionFactory                (Log&                            log = GetDefaultLog());
    GEOCOORDS_DLLE explicit                 ReprojectionFactory                (const ReprojectionPolicy&       policy,
                                                                                Log&                            log = GetDefaultLog());

    GEOCOORDS_DLLE                          ~ReprojectionFactory               ();

    GEOCOORDS_DLLE                          ReprojectionFactory                (const ReprojectionFactory&       rhs);


    GEOCOORDS_DLLE Reprojection             Create                             (const GCS&                      sourceGCS,
                                                                                const GCS&                      targetGCS,
                                                                                const DRange3d*                 sourceExtentP,
                                                                                Status&                         status) const;


    GEOCOORDS_DLLE Reprojection             Create                             (const GCS&                      sourceGCS,
                                                                                const GCS&                      targetGCS,
                                                                                const DRange3d*                 sourceExtentP) const;

    // TDORAY: Add an overload that takes a convex hull or something similar of the sources so that we won't enable
    // point clipping or return an error when we shouldn't

    };



/*---------------------------------------------------------------------------------**//**
* @description  Class for handling the reprojection/transformation step that transforms 
*               coordinates from a source CS to a target CS.
*    
*               Here is the path that is followed through reprojection/transformation
*               process:
*               sFCS -> sBCS -> tBCS -> tFCS 
*               or
*               sLocal -> sBase -> tBase -> tLocal
*
*               Where:
*                   -s: Source
*                   -t: Target
*                   -FCS: Fitted Coordinate System -> A coordinate system that is defined 
*                                                     inside another.
*                   -BCS: Base Coordinate System -> May be either LCS or GCS
*                   -LCS: Local Coordinate System (Not spatially referenced)
*                   -GCS: Geographic Coordinate System (Spatially referenced relatively to earth)
*                   -sFCS is optional and so can be equal to sBCS
*                   -tFCS is optional and so can be equal to tBCS
*
*
* NOTE:     - Not designed to be a base class. 
*           - Optimized with copy on write.
* @see GCS
* @see ReprojectionFactory
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Reprojection
    {
private:
    friend struct                           ReprojectionFactory;

    typedef TransfoModelBase                Impl;
    typedef SharedPtrTypeTrait<Impl>::type  ImplPtr;
    ImplPtr                                 m_implP;

    typedef const std::type_info*           ClassID;
    ClassID                                 m_classID;

    explicit                                Reprojection                       (Impl*                           implP);
    explicit                                Reprojection                       (const TransfoModel&             transfoModel);

public:
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        S_ERROR_DOES_NOT_FIT_MATHEMATICAL_DOMAIN,
        S_QTY,
        };

    GEOCOORDS_DLLE static const Reprojection&   
                                            GetNull                            ();

    GEOCOORDS_DLLE                          ~Reprojection                      ();

    GEOCOORDS_DLLE                          Reprojection                       (const Reprojection&             rhs);
    GEOCOORDS_DLLE Reprojection&            operator=                          (const Reprojection&             rhs);


    GEOCOORDS_DLLE bool                     IsNull                             () const;

    GEOCOORDS_DLLE Status                   Reproject                          (const DPoint3d&                 sourcePt,
                                                                                DPoint3d&                       targetPt) const;


    GEOCOORDS_DLLE Status                   Reproject                          (const DPoint3d*                 sourcePtP,
                                                                                size_t                          sourcePtQty,
                                                                                DPoint3d*                       targetPtP) const;


    GEOCOORDS_DLLE friend TransfoModel      AsTransfoModel                     (const Reprojection&             reprojection);
    GEOCOORDS_DLLE friend TransfoModel      AsTransfoModel                     (Reprojection&                   reprojection);

    // TDORAY: Add an overload that oversample source for lines? 
    //         Will also need a method that return the new size when oversampled if it is the case.
    };




/*---------------------------------------------------------------------------------**//**
* @description  Describes the rules applied throughout the reprojection process. 
*
*               Default values are:
*                   true for attributes prefixed by Enable
*                   false for attributes prefixed by Allow
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionPolicy
    {
private:
    enum 
        {
        FLAG_ALLOW_CONVERSION_BETWEEN_UNIT_BASES = 0x1,
        FLAG_ALLOW_GCS_TO_LCS = 0x2,
        FLAG_ALLOW_LCS_TO_GCS = 0x4,
        };

    uint32_t                                  m_flags;
    double                                  m_angularToLinearUnitRatio;
    const void*                             m_implP; // Reserved some space for further use.

public:
    GEOCOORDS_DLLE explicit                 ReprojectionPolicy                         ();

    GEOCOORDS_DLLE                          ~ReprojectionPolicy                        ();
    GEOCOORDS_DLLE                          ReprojectionPolicy                         (const ReprojectionPolicy&       rhs);
    GEOCOORDS_DLLE ReprojectionPolicy&      operator=                                  (const ReprojectionPolicy&       rhs);
    
    ReprojectionPolicy&                     AllowConversionBetweenUnitBases            (bool                            allowed);
    ReprojectionPolicy&                     AllowGCSToLCS                              (bool                            allowed);
    ReprojectionPolicy&                     AllowLCSToGCS                              (bool                            allowed);

    ReprojectionPolicy&                     SetAngularToLinearUnitRatio                (double                          ratio);

    bool                                    IsConversionBetweenUnitBasesAllowed        () const;
    bool                                    IsGCSToLCSAllowed                          () const;
    bool                                    IsLCSToGCSAllowed                          () const;


    double                                  GetAngularToLinearUnitRatio                () const;

    };





// TDORAY: Move to hpp
inline ReprojectionPolicy& ReprojectionPolicy::AllowConversionBetweenUnitBases (bool enabled) 
    { SetBitsTo(m_flags, FLAG_ALLOW_CONVERSION_BETWEEN_UNIT_BASES, enabled); return *this; }
inline ReprojectionPolicy& ReprojectionPolicy::AllowGCSToLCS (bool allowed) 
    { SetBitsTo(m_flags, FLAG_ALLOW_GCS_TO_LCS, allowed); return *this; }
inline ReprojectionPolicy& ReprojectionPolicy::AllowLCSToGCS (bool allowed) 
    { SetBitsTo(m_flags, FLAG_ALLOW_LCS_TO_GCS, allowed); return *this; }
inline ReprojectionPolicy& ReprojectionPolicy::SetAngularToLinearUnitRatio (double ratio)
    { m_angularToLinearUnitRatio = ratio; return *this; }


inline bool ReprojectionPolicy::IsConversionBetweenUnitBasesAllowed () const 
    { return HasBitsOn(m_flags, FLAG_ALLOW_CONVERSION_BETWEEN_UNIT_BASES); }
inline bool ReprojectionPolicy::IsGCSToLCSAllowed () const 
    { return HasBitsOn(m_flags, FLAG_ALLOW_GCS_TO_LCS); }
inline bool ReprojectionPolicy::IsLCSToGCSAllowed () const 
    { return HasBitsOn(m_flags, FLAG_ALLOW_LCS_TO_GCS); }
inline double ReprojectionPolicy::GetAngularToLinearUnitRatio () const 
    { return m_angularToLinearUnitRatio; }

END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
