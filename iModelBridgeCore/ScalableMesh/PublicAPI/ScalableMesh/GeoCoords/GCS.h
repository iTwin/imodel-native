/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/GeoCoords/GCS.h $
|    $RCSfile: GCS.h,v $
|   $Revision: 1.40 $
|       $Date: 2011/12/01 18:51:33 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/GeoCoords/Definitions.h>


// TDORAY: Temporarily included. Remove
#include <ScalableMesh/GeoCoords/LocalTransform.h>



/** 
 * External Forward declarations section
 */
BEGIN_BENTLEY_NAMESPACE
namespace GeoCoordinates {
class BaseGCS;

typedef RefCountedPtr<BaseGCS>          BaseGCSPtr;
typedef BENTLEY_NAMESPACE_NAME::RefCountedCPtr<BaseGCS> BaseGCSCPtr; 

} // namespace GeoCoordinates
END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE

struct GCS;
struct WKT;
struct Unit;

struct LocalTransform;
struct TransfoModel;
struct TransfoMatrix;

struct GeospatialReference;

/*
 * More user friendly synonyms
 */
typedef GeospatialReference                 GeoRef;

// Make BaseGCS and its pointers part of this namespace
using BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS;
using BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr;
using BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr;

/*---------------------------------------------------------------------------------**//**
* @description  Factory for creating GCS. User may specify custom log mechanism that will
*               be used through creation step and when required, by GCS instances. 
*
*               Error handling: User is given three choices for dealings with errors:
*                   - No throw interface: choosing the overloads that returns a status
*                     guarantee that no exception will ever be thrown and allow the
*                     user to react on error using traditional statuses. Null gcs 
*                     will be returned on error.
*                   - Throwing interface: 
*                       - All create methods returning no statuses will throw on errors
*                         by default.
*                       - User may specify in the policy that exceptions normally thrown
*                         by this interface by redirected to the specified Log as an
*                         error. Null gcs will be returned on error. // TDORAY: Still to come
*                     
*
*               User will also be able to specify creation policy //TDORAY.
* @see GCS
* @see Foundations::Exception
*
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct GCSFactory : private Unassignable
    {


private:
    struct                                  Impl;
    SharedPtrTypeTrait<const Impl>::type    m_implP;

public:
    GEOCOORDS_DLLE explicit                 GCSFactory                         (Log&                        log = GetDefaultLog());
    GEOCOORDS_DLLE                          ~GCSFactory                        ();

    GEOCOORDS_DLLE                          GCSFactory                         (const GCSFactory&           rhs);

    /*
     * No throw create methods
     */
    GEOCOORDS_DLLE GCS                      Create                             (const WChar*              wkt,
                                                                                SMStatus&                     status) const;

    GEOCOORDS_DLLE GCS                      Create                             (const WKT&                  wkt,
                                                                                SMStatus&                     status) const;

    GEOCOORDS_DLLE GCS                      Create                             (const wchar_t*              wkt,
                                                                                BaseGCS::WktFlavor          wktFlavor,
                                                                                SMStatus&                     status) const;

    GEOCOORDS_DLLE GCS                      Create                             (const BaseGCSCPtr&          baseGCSPtr,
                                                                                SMStatus&                     status) const;

    GEOCOORDS_DLLE GCS                      Create                             (const GeospatialReference&  geoRef,
                                                                                SMStatus&                     status) const;

    GEOCOORDS_DLLE GCS                      Create                             (const Unit&                 unit,
                                                                                SMStatus&                     status) const;

    GEOCOORDS_DLLE GCS                      Create                             (const BaseGCSCPtr&          baseGCSPtr,
                                                                                const LocalTransform&       localTransform,
                                                                                SMStatus&                     status) const;

    GEOCOORDS_DLLE GCS                      Create                             (const GeospatialReference&  geoRef,
                                                                                const LocalTransform&       localTransform,
                                                                                SMStatus&                     status) const;

    GEOCOORDS_DLLE GCS                      Create                             (const Unit&                 unit,
                                                                                const LocalTransform&       localTransform,
                                                                                SMStatus&                     status) const;

    GEOCOORDS_DLLE GCS                      Create                             (const GCS&                  gcs,
                                                                                const LocalTransform&       appendedLocalTransform,
                                                                                SMStatus&                     status) const;

    /*
     * Throwing/logging create methods
     * TDORAY: Permit user to specify a policy flag that would specifies the error is to be  redirecting this error to the specified log instead
     *         of throwing.
     */
    GEOCOORDS_DLLE GCS                      Create                             (const WChar*              wkt) const;

    GEOCOORDS_DLLE GCS                      Create                             (const WKT&                  wkt) const;

    GEOCOORDS_DLLE GCS                      Create                             (const BaseGCSCPtr&          baseGCSPtr) const;

    GEOCOORDS_DLLE GCS                      Create                             (const GeospatialReference&  geoRef) const;

    GEOCOORDS_DLLE GCS                      Create                             (const Unit&                 unit) const;

    GEOCOORDS_DLLE GCS                      Create                             (const BaseGCSCPtr&          baseGCSPtr,
                                                                                const LocalTransform&       localTransform) const;

    GEOCOORDS_DLLE GCS                      Create                             (const GeospatialReference&  geoRef,
                                                                                const LocalTransform&       localTransform) const;

    GEOCOORDS_DLLE GCS                      Create                             (const Unit&                 unit,
                                                                                const LocalTransform&       localTransform) const;

    GEOCOORDS_DLLE GCS                      Create                             (const GCS&                  gcs,
                                                                                const LocalTransform&       appendedLocalTransform) const;
    };



/*---------------------------------------------------------------------------------**//**
* @description  Class for describing a geographic coordinate system. This class was
*               designed as a wrapper that provides missing basegcs functionalities such
*               as describing local CS and fitted CS (GCS + local
*               transform). Another goal was to offer a simpler interface than base gcs
*               that nicely integrates with reprojection steps.
*
*               A GCS can be in one of these forms:
*                   - Null -> No geo spatial reference and no unit
*                   - Local CS -> No geo spatial reference and with units (spatial 
*                     reference is contextual)
*                   - GCS -> Full fledged geo CS with geo spacial reference and units
*                   - Fitted local CS -> Local CS with a supplementary local transform
*                     describing how to reach initial position from an unknown/contextual
*                     spatial reference.
*                   - Fitted GCS -> GCS with a supplementary local transform describing
*                     how to reach initial position from the defined spatial reference.
*               
*               GCS instance can be created by providing the following information:
*                   - a WKT
*                   - a BaseGCS instance
*                   - a Unit
*                   - a Horizontal Unit and a Vertical Unit //TDORAY
*                   - a BaseGCS with a local transform
*                   - a Unit with a local transform
*    
*               When working with local transforms, it is important to understand 
*               that units returned by this interface are units of the base world, 
*               not of the local world. It was chosen that units of the local 
*               world remain unspecified.
*
*               Once a GCS instance is created, user need not worry about its form for
*               reprojection when used with Reprojection class. Reprojection class
*               is responsible for providing correct behavior for the reprojection 
*               of all possible cases of source/target GCS pairing.
*
*               For more complex reprojection behavior or for display purpose, user may 
*               access underlying BaseGCS, units and local transform using provided 
*               accessors.
*
* NOTE:     - Not designed to be a base class. 
*           - Optimized with copy on write.
*
* @see WKT
* @see Unit
* @see Reprojection
* @see LocalTransform
*
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct GCS
    {


private:
    friend struct                           GCSFactory;

    struct                                  Impl;
    typedef SharedPtrTypeTrait<Impl>::type  ImplPtr;
    ImplPtr                                 m_implP;

   
public:
    GEOCOORDS_DLLE static const GCS&        GetNull                            ();

    GEOCOORDS_DLLE                          ~GCS                               ();

    GEOCOORDS_DLLE                          GCS                                (const GCS&                  rhs);
    GEOCOORDS_DLLE GCS&                     operator=                          (const GCS&                  rhs);

    GEOCOORDS_DLLE bool                     IsNull                             () const;
    
    GEOCOORDS_DLLE bool                     HasGeoRef                          () const;
    GEOCOORDS_DLLE const GeoRef&            GetGeoRef                          () const;

    // TDORAY: Add geo spatial reference Edit accessors here. These may trigger object copy if shared.


    GEOCOORDS_DLLE bool                     HasUniformUnits                    () const; 

    // Synonymous to GetHorizontalUnit when non-uniform vertical/horizontal units
    GEOCOORDS_DLLE const Unit&              GetUnit                            () const; 
    explicit                                GCS                                (Impl*                       implP);

    GEOCOORDS_DLLE const Unit&              GetHorizontalUnit                  () const;
    GEOCOORDS_DLLE const Unit&              GetVerticalUnit                    () const;

    GEOCOORDS_DLLE bool                     HasLocalTransform                  () const;
    GEOCOORDS_DLLE const LocalTransform&    GetLocalTransform                  () const;

    GEOCOORDS_DLLE void                     SetLocalTransform                  (const LocalTransform&       localTransform);
    GEOCOORDS_DLLE SMStatus                   AppendLocalTransform(const LocalTransform&       localTransform);

    GEOCOORDS_DLLE WKT                      GetWKT                             () const;
    GEOCOORDS_DLLE WKT                      GetWKT(SMStatus&                     status) const;
    
    // TDORAY: Consider possibility of adding WKB accessors

    GEOCOORDS_DLLE bool                     IsEquivalent                       (const GCS&                  rhs) const;


    GEOCOORDS_DLLE friend void              swap                               (GCS&                        lhs,
                                                                                GCS&                        rhs);
    };

GEOCOORDS_DLLE Unit                         GetUnitFor                         (const BaseGCS&              baseGCS);


GEOCOORDS_DLLE bool                         HaveCompatibleUnits                (const GCS&                  lhs,
                                                                                const GCS&                  rhs);

GEOCOORDS_DLLE bool                         HaveEquivalentUnits                (const GCS&                  lhs,
                                                                                const GCS&                  rhs);


/*---------------------------------------------------------------------------------**//**
* @description  Geo spatial reference. Currently using BaseGCS as implementation.
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct GeospatialReference
    {


private:
    friend struct                           GCS;
    friend struct                           GCSFactory;

    const void*                             m_implP; // Reserved some space for further use
    BaseGCSCPtr                             m_basePtr;
    
    static const GeospatialReference&       GetNull ();

    bool                                    IsNull                             () const { return 0 == m_basePtr.get(); }
    explicit                                GeospatialReference                (const BaseGCSCPtr&          basePtr);

public:
    GEOCOORDS_DLLE                          ~GeospatialReference               ();

    GEOCOORDS_DLLE                          GeospatialReference                (const GeospatialReference&  rhs);
    GEOCOORDS_DLLE GeospatialReference&     operator=                          (const GeospatialReference&  rhs);

    const BaseGCS&                          GetBase                            () const { assert(!IsNull()); return *m_basePtr; }
    const BaseGCSCPtr&                      GetBasePtr                         () const { return m_basePtr; }


    // TDORAY: Consider adding some accessors here so that user can access advanced stuff such as 
    // datum, etc.. User should normally not have to access the base once this object is created.
    // This is only temporary as we have no time to invest in doing this for the moment.

    };



/*---------------------------------------------------------------------------------**//**
* @description  Class for describing a unit. A unit is always defined relatively to
*               an international base. This base could be either angular (radian based)
*               or linear (meter based). User may retrieve the ratio for converting the
*               unit to its base using the ratio to base attribute. 
*
*               When creating a new unit, user must provide an indicative name and
*               a ratio to its base. Name is optional but we strongly recommend
*               providing one in order to simplify debugging.
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Unit
    {
    enum BaseID
        {
        BASEID_METER,
        BASEID_RADIAN,
        BASEID_QTY,
        };

private:
    std::auto_ptr<WString>             m_nameP;
    BaseID                                  m_baseID;
    double                                  m_ratioToBase;

    explicit                                Unit                               (const WChar*              name,
                                                                                BaseID                      baseID,
                                                                                double                      ratioToBase);
public:
    GEOCOORDS_DLLE static const Unit&       GetMeter                           ();
    GEOCOORDS_DLLE static const Unit&       GetRadian                          ();
    GEOCOORDS_DLLE static const Unit&       GetDegree                          ();

    GEOCOORDS_DLLE static Unit              CreateLinearFrom                   (const WChar*              name,
                                                                                double                      ratioToMeter);
    GEOCOORDS_DLLE static Unit              CreateLinearFrom                   (const char*                 name,
                                                                                double                      ratioToMeter);

    GEOCOORDS_DLLE static Unit              CreateAngularFrom                  (const WChar*              name,
                                                                                double                      ratioToRadian);
    GEOCOORDS_DLLE static Unit              CreateAngularFrom                  (const char*                 name,
                                                                                double                      ratioToRadian);

    GEOCOORDS_DLLE static Unit              CreateFromDegreeBased              (const WChar*              name,
                                                                                double                      ratioToDegree);
    GEOCOORDS_DLLE static Unit              CreateFromDegreeBased              (const char*                 name,
                                                                                double                      ratioToDegree);

    GEOCOORDS_DLLE                          ~Unit                              ();

    GEOCOORDS_DLLE                          Unit                               (const Unit&                 rhs);
    GEOCOORDS_DLLE Unit&                    operator=                          (const Unit&                 rhs);

    GEOCOORDS_DLLE const WChar*             GetNameCStr                        () const;
    GEOCOORDS_DLLE const WString&           GetName                            () const;

    BaseID                                  GetBaseID                          () const { return m_baseID; }
    GEOCOORDS_DLLE const Unit&              GetBase                            () const;

    GEOCOORDS_DLLE bool                     IsLinear                           () const;
    GEOCOORDS_DLLE bool                     IsAngular                          () const;

    double                                  GetRatioToBase                     () const { return m_ratioToBase; }

    GEOCOORDS_DLLE bool                     IsEquivalent                       (const Unit&                 rhs) const;

    };



GEOCOORDS_DLLE double                       GetUnitRectificationScaleFactor    (const Unit&                 sourceUnit,
                                                                                const Unit&                 targetUnit,
                                                                                double                      angularToLinearRatio = 1.0);

GEOCOORDS_DLLE TransfoModel                 GetUnitRectificationTransfoModel   (const Unit&                 sourceUnit,
                                                                                const Unit&                 targetUnit,
                                                                                double                      angularToLinearRatio = 1.0);

GEOCOORDS_DLLE TransfoModel                 GetUnitRectificationTransfoModel   (const Unit&                 sourceHorizontalUnit,
                                                                                const Unit&                 sourceVerticalUnit,
                                                                                const Unit&                 targetHorizontalUnit,
                                                                                const Unit&                 targetVerticalUnit,
                                                                                double                      angularToLinearRatio = 1.0);

GEOCOORDS_DLLE TransfoMatrix                GetUnitRectificationTransfoMatrix  (const Unit&                 sourceUnit,
                                                                                const Unit&                 targetUnit,
                                                                                double                      angularToLinearRatio = 1.0);

GEOCOORDS_DLLE TransfoMatrix                GetUnitRectificationTransfoMatrix  (const Unit&                 sourceHorizontalUnit,
                                                                                const Unit&                 sourceVerticalUnit,
                                                                                const Unit&                 targetHorizontalUnit,
                                                                                const Unit&                 targetVerticalUnit,
                                                                                double                      angularToLinearRatio = 1.0);


inline double                               GetAngularToLinearRatio            (const Unit&                 linearEquivalent,
                                                                                const Unit&                 angularEquivalent)
    {
    assert(linearEquivalent.IsLinear() && angularEquivalent.IsAngular()); 
    return linearEquivalent.GetRatioToBase() / angularEquivalent.GetRatioToBase();
    }



/*---------------------------------------------------------------------------------**//**
* @description  Class for holding a geographic coordinate system using the
*               well-known-text format.     
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct WKT
    {
private:
    std::auto_ptr<WString>             m_wktP;    
public:
    GEOCOORDS_DLLE explicit                 WKT                                ();

    GEOCOORDS_DLLE explicit                 WKT                                (const WChar*              wkt);    
    GEOCOORDS_DLLE                          ~WKT                               ();

    GEOCOORDS_DLLE                          WKT                                (const WKT&                  rhs);
    GEOCOORDS_DLLE WKT&                     operator=                          (const WKT&                  rhs);

    GEOCOORDS_DLLE WKT&                     operator=                          (const WChar*              wkt);    

    GEOCOORDS_DLLE bool                     IsEmpty                            () const;

    GEOCOORDS_DLLE const WChar*           GetCStr                            () const;
    GEOCOORDS_DLLE const WString&      Get                                () const;
    };


enum WKTSupport
    {
    WKTSUPPORT_NONE,        // No support at all
    WKTSUPPORT_ONLY_TO,     // Can only write a wkt representation of the specified gcs
    WKTSUPPORT_ONLY_FROM,   // Can only read from a wkt representation of the specified gcs
    WKTSUPPORT_FULL,        // Full support. Can read/write a wkt representation of the specified gcs
    WKTSUPPORT_QTY,
    };

GEOCOORDS_DLLE WKTSupport                   GetWKTSupportFor                   (const GCS&                  gcs);

GEOCOORDS_DLLE WKTSupport                   GetWKTSupportFor                   (const GCS&                  gcs, 
                                                                                BaseGCS::WktFlavor          wktFlavor);

END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
