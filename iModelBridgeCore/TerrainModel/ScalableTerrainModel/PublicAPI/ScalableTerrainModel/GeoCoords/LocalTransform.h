/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/GeoCoords/LocalTransform.h $
|    $RCSfile: LocalTransform.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/12/01 18:51:34 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/GeoCoords/Transformation.h>

BEGIN_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE


// TDORAY: Would it be better to have a transfo model factory so that we may setup warning log and policy?

/*---------------------------------------------------------------------------------**//**
* @description  Class defining the relation between a contextual global world and a local 
*               world defined within this global world. It is basically composed of two 
*               transformation model defining the bi-directional (which we named "duplex")
*               relation between these two worlds. 
*               
*               ToGlobal refers to the transformation model that transform local 
*               coordinates into global coordinates.
*
*               ToLocal refers to the exact opposite: the transformation model that 
*               transform global coordinates into local coordinate.
*
*               It is to be noted to "Global"/"Local" keywords are only given a sense/meaning
*               by the user and/or the context in which he chooses to use this class.
*               Sometime, meaning can be easily deduced from the context (e.g.: a geographic
*               coordinate system is evidently the "Global" world) whereas other times, 
*               user need to explicitly specify the meaning he has given to each.
*
*               This class allows for one-sided only relation whose reverse may be 
*               computed in a lazy fashion only when required (using either explicit 
*               ComputeDuplex which allow error detection via a status or transformation
*               model accessors GetToGlobal/GetToLocal). Computing the inverse of 
*               transformation model is not always feasible. In these cases, this interface
*               may find itself unable to fulfill its promises an has to report an error
*               to the user (see the error handling section). In cases of full duplex 
*               (bi-directional) relations, error free behavior can be expected.
*
*               Local transformations may also be combined. Combination may be achieved
*               by using either Append member or Combine non-member. In both cases,
*               right side's global world is attached to left side's local world and
*               in doing so are specified to be the same world. User is left with 
*               the responsibility to make sure that this is indeed the case.
*
*               Error handling: it is to be understood by the user that every method 
*               of this interface that need to compute inverse transformation model 
*               in order to accomplish its responsibility may throw. In order to 
*               achieve no throw behavior, user may either:
*                   - Ensure that local transforms he works with are duplex and if 
*                     not make them. All operation on duplex local transforms are
*                     guaranteed not to throw.
*                   - Use an overload returning a status if available. When a status
*                     is returned, no throw behavior is also guaranteed.
*                   - Be confident that inverse transformation model is computable 
*                     without fuss (e.g: most if not all linear transformations)
*               
*               
* @see TransfoModel     
* @see Foundations::Exception
* 
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalTransform
    {
private:
    struct                                  Impl;
    typedef SharedPtrTypeTrait<Impl>::type  ImplPtr;
    ImplPtr                                 m_implP;

    explicit                                LocalTransform                     (Impl*                       implP);
public:
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        };
    
    GEOCOORDS_DLLE static const LocalTransform&                     
                                            GetIdentity                        ();


    GEOCOORDS_DLLE static LocalTransform    CreateFrom                         (const TransfoModel&         localToGlobal,
                                                                                const TransfoModel&         globalToLocal);

    GEOCOORDS_DLLE static LocalTransform    CreateFromToGlobal                 (const TransfoModel&         localToGlobal);
    GEOCOORDS_DLLE static LocalTransform    CreateFromToLocal                  (const TransfoModel&         globalToLocal);


    GEOCOORDS_DLLE static LocalTransform    CreateDuplexFromToGlobal           (const TransfoModel&         localToGlobal);
    GEOCOORDS_DLLE static LocalTransform    CreateDuplexFromToLocal            (const TransfoModel&         globalToLocal);

    GEOCOORDS_DLLE static LocalTransform    CreateDuplexFromToGlobal           (const TransfoModel&         localToGlobal,
                                                                                Status&                     status);
    GEOCOORDS_DLLE static LocalTransform    CreateDuplexFromToLocal            (const TransfoModel&         globalToLocal,
                                                                                Status&                     status);

    GEOCOORDS_DLLE                          ~LocalTransform                    ();

    GEOCOORDS_DLLE                          LocalTransform                     (const LocalTransform&       rhs);
    GEOCOORDS_DLLE LocalTransform&          operator=                          (const LocalTransform&       rhs);

    

    GEOCOORDS_DLLE bool                     IsIdentity                         () const;
    GEOCOORDS_DLLE bool                     IsDuplex                           () const;

    GEOCOORDS_DLLE bool                     IsEquivalent                       (const LocalTransform&       rhs) const;




    GEOCOORDS_DLLE bool                     HasToGlobal                        () const;
    GEOCOORDS_DLLE const TransfoModel&      GetToGlobal                        () const;

    GEOCOORDS_DLLE bool                     HasToLocal                         () const;
    GEOCOORDS_DLLE const TransfoModel&      GetToLocal                         () const;

    GEOCOORDS_DLLE Status                   ComputeDuplex                      ();


    GEOCOORDS_DLLE Status                   Append                             (const LocalTransform&       rhs);

    GEOCOORDS_DLLE friend LocalTransform    Combine                            (const LocalTransform&       lhs,
                                                                                const LocalTransform&       rhs,
                                                                                LocalTransform::Status&     status);

    GEOCOORDS_DLLE friend void              swap                               (LocalTransform&             lhs,
                                                                                LocalTransform&             rhs);
    };


GEOCOORDS_DLLE LocalTransform               Combine                            (const LocalTransform&       lhs,
                                                                                const LocalTransform&       rhs);


END_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE