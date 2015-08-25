/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/GeoCoords/Transformation.h $
|    $RCSfile: Transformation.h,v $
|   $Revision: 1.13 $
|       $Date: 2011/11/07 14:27:12 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/GeoCoords/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE


struct TransfoMatrix;
struct TransfoModelBase;
struct Reprojection;


// TDORAY: Would it be better to have a transfo model factory so that we may setup warning log and policy?

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransfoModel
    {
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        S_ERROR_DOES_NOT_FIT_MATHEMATICAL_DOMAIN,
        S_QTY,
        };

private:
    typedef TransfoModelBase                Impl;
    typedef SharedPtrTypeTrait<Impl>::type  ImplPtr;
    ImplPtr                                 m_implP;

    typedef const std::type_info*           ClassID;
    ClassID                                 m_classID;

    friend struct                           TransfoModelHandler;

    explicit                                TransfoModel                       (Impl*                           implP);
    explicit                                TransfoModel                       (const ImplPtr&                  implPtr);

public:
    GEOCOORDS_DLLE static const TransfoModel&      
                                            GetIdentity                        ();               

    GEOCOORDS_DLLE static TransfoModel      CreateFrom                         (const TransfoMatrix&            transfoMatrix,
                                                                                Status&                         status);

    GEOCOORDS_DLLE static TransfoModel      CreateFrom                         (const TransfoMatrix&            transfoMatrix);

    GEOCOORDS_DLLE static TransfoModel      CreateScalingFrom                  (double scale);
    GEOCOORDS_DLLE static TransfoModel      CreateScalingFrom                  (double xScale, double yScale, double zScale);

    GEOCOORDS_DLLE static TransfoModel      CreateTranslatingFrom              (double xTrans, double yTrans, double zTrans);

    GEOCOORDS_DLLE static TransfoModel      CreateScalingTranslatingFrom       (double scale,
                                                                                double xTrans, double yTrans, double zTrans);
    GEOCOORDS_DLLE static TransfoModel      CreateScalingTranslatingFrom       (double xScale, double yScale, double zScale,
                                                                                double xTrans, double yTrans, double zTrans);



    GEOCOORDS_DLLE static TransfoModel      CreateAffineFrom                   (const TransfoMatrix&            affineMatrix);

    GEOCOORDS_DLLE                          TransfoModel                       (const TransfoModel&             rhs);
    GEOCOORDS_DLLE TransfoModel&            operator=                          (const TransfoModel&             rhs);

    GEOCOORDS_DLLE                          ~TransfoModel                      ();

    GEOCOORDS_DLLE bool                     IsIdentity                         () const;

    GEOCOORDS_DLLE bool                     IsConvertibleToMatrix              () const;
    GEOCOORDS_DLLE TransfoMatrix            ConvertToMatrix                    () const;

    GEOCOORDS_DLLE bool                     IsEquivalent                       (const TransfoModel&             rhs) const;


    GEOCOORDS_DLLE Status                   Transform                          (const DPoint3d&                 sourcePt,
                                                                                DPoint3d&                       targetPt) const;


    GEOCOORDS_DLLE Status                   Transform                          (const DPoint3d*                 sourcePtP,
                                                                                size_t                          sourcePtQty,
                                                                                DPoint3d*                       targetPtP) const;

    GEOCOORDS_DLLE Status                   Append                             (const TransfoModel&             rhs);


    GEOCOORDS_DLLE Status                   Inverse                            ();

    GEOCOORDS_DLLE friend TransfoModel      Combine                            (const TransfoModel&             lhs,
                                                                                const TransfoModel&             rhs,
                                                                                TransfoModel::Status&           status);

    };

GEOCOORDS_DLLE void                         swap                               (TransfoModel&                   lhs,
                                                                                TransfoModel&                   rhs);

GEOCOORDS_DLLE TransfoModel                 Combine                            (const TransfoModel&             lhs,
                                                                                const TransfoModel&             rhs);

GEOCOORDS_DLLE TransfoModel                 Combine                            (const TransfoModel&             lhs,
                                                                                const TransfoModel&             rhs,
                                                                                TransfoModel::Status&           status);

GEOCOORDS_DLLE TransfoModel                 InverseOf                          (const TransfoModel&             transform);

GEOCOORDS_DLLE TransfoModel                 InverseOf                          (const TransfoModel&             transform,
                                                                                TransfoModel::Status&           status);




/*---------------------------------------------------------------------------------**//**
* @description     
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransfoMatrix
    {
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        S_QTY,
        };

private:
    double                                  m_parameters[3][4];
public:
    typedef const double*                   CRowProxy;
    struct                                  RowProxy;

    static const TransfoMatrix&             GetIdentity                        ();

    explicit                                TransfoMatrix                      ();
    explicit                                TransfoMatrix                      (const double                    parameters[][4]);

    explicit                                TransfoMatrix                      (double r0c0, double r0c1, double r0c2, double r0c3,
                                                                                double r1c0, double r1c1, double r1c2, double r1c3,
                                                                                double r2c0, double r2c1, double r2c2, double r2c3);

    // Use default copy behaviour

    CRowProxy                               operator[]                         (size_t                          idx) const;
    RowProxy                                operator[]                         (size_t                          idx);

    GEOCOORDS_DLLE friend bool              operator==                         (const TransfoMatrix&            lhs,
                                                                                const TransfoMatrix&            rhs);


    GEOCOORDS_DLLE TransfoMatrix&           operator*=                         (const TransfoMatrix&            rhs);

    GEOCOORDS_DLLE friend TransfoMatrix     operator*                          (const TransfoMatrix&            lhs,
                                                                                const TransfoMatrix&            rhs);

    friend DPoint3d                         operator*                          (const TransfoMatrix&            lhs,
                                                                                const DPoint3d&                 rhs);

    GEOCOORDS_DLLE Status                   Inverse                            ();

    // TDORAY: Add a *= operator
    };


GEOCOORDS_DLLE TransfoMatrix                InverseOf                          (const TransfoMatrix&            matrix);

GEOCOORDS_DLLE TransfoMatrix                InverseOf                          (const TransfoMatrix&            matrix,
                                                                                TransfoMatrix::Status&          status);




#include <ScalableMesh/GeoCoords/Transformation.hpp>

END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
