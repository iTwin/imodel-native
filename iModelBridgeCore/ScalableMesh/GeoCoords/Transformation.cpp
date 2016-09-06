/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/Transformation.cpp $
|    $RCSfile: Transformation.cpp,v $
|   $Revision: 1.13 $
|       $Date: 2011/11/07 14:26:49 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include <ScalableMesh/GeoCoords/Transformation.h>
#include <ScalableMesh/Foundations/Exception.h>
#include <STMInternal/Foundations/FoundationsPrivateTools.h>
#include "TransformationUtils.h"
#include "TransformationInternal.h"
#include "TransfoModelMixinBase.h"

#define INCLUDED_FROM_TRANSFORMATION_CPP 1
#include "TransfoModelAffine.hpp"
#include "TransfoModelCombined.hpp"
#include "TransfoModelIdentity.hpp"
#include "TransfoModelScaling.hpp"
#include "TransfoModelScalingTranslating.hpp"
#include "TransfoModelTranslating.hpp"

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE

using namespace Internal;
 

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const TransfoModel& TransfoModel::GetIdentity ()
    {
    static const TransfoModel IDENTITY(TransfoModelIdentity::CreateFrom());
    return IDENTITY;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel TransfoModel::CreateFrom  (const TransfoMatrix&    m,
                                        SMStatus&                 status)
    {
    // TDORAY: Solve the problem by creating matrix properties analyzer that
    // returns a bitfield specifying all these.

    const bool hasOnlyScalingAndTranslatingProperties = 
          EqZeroEps(m[0][1]) && EqZeroEps(m[0][2]) && 
          EqZeroEps(m[1][0]) && EqZeroEps(m[1][2]) &&
          EqZeroEps(m[2][0]) && EqZeroEps(m[2][1]);

    if (!hasOnlyScalingAndTranslatingProperties)
        {
        status = S_SUCCESS;

        // TDORAY: Validate here if it is an affine.
        return TransfoModel(TransfoModelAffine::CreateFrom(m));
        }

    const bool hasIdentityScaling = EqOneEps( m[0][0]) && EqOneEps(m[1][1]) && EqOneEps(m[2][2]);
    const bool hasIdentityTranslating = EqZeroEps(m[0][3]) && EqZeroEps(m[1][3]) && EqZeroEps(m[2][3]);

    if (hasIdentityScaling && hasIdentityTranslating)
        {
        status = S_SUCCESS;
        return TransfoModel::GetIdentity();
        }
    else if (hasIdentityScaling)
        {
        status = S_SUCCESS;
        return TransfoModel(TransfoModelTranslating::CreateFrom(m[0][3], m[1][3], m[2][3]));
        }
    else if (hasIdentityTranslating)
        {
        status = S_SUCCESS;
        return TransfoModel(TransfoModelScaling::CreateFrom(m[0][0], m[1][1], m[2][2]));
        }
    else
        {
        status = S_SUCCESS;
        return TransfoModel(TransfoModelScalingTranslating::CreateFrom(m[0][0], m[1][1], m[2][2],
                                                                       m[0][3], m[1][3], m[2][3]));

        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel TransfoModel::CreateFrom (const TransfoMatrix& transfoMatrix)
    {
    SMStatus status;
    const TransfoModel resulting(CreateFrom(transfoMatrix, status));

    if (S_SUCCESS != status)
        throw CustomException(L"Could not create from specified matrix");

    return resulting;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel TransfoModel::CreateScalingFrom (double scale)
    {
    return CreateScalingFrom(scale, scale, scale);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel TransfoModel::CreateScalingFrom (double xScale, double yScale, double zScale)
    {
    if (EqOneEps(xScale) && EqOneEps(yScale) && EqOneEps(zScale))
        return TransfoModel::GetIdentity();

    return TransfoModel(TransfoModelScaling::CreateFrom(xScale, yScale, zScale));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel TransfoModel::CreateTranslatingFrom (double xTrans, double yTrans, double zTrans)
    {
    if (EqZeroEps(xTrans) && EqZeroEps(yTrans) && EqZeroEps(zTrans))
        return TransfoModel::GetIdentity();

    return TransfoModel(TransfoModelTranslating::CreateFrom(xTrans, yTrans, zTrans));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel TransfoModel::CreateScalingTranslatingFrom    (double scale,
                                                            double xTrans, double yTrans, double zTrans)
    {
    return CreateScalingTranslatingFrom(scale, scale, scale, 
                                        xTrans, yTrans, zTrans);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel TransfoModel::CreateScalingTranslatingFrom    (double xScale, double yScale, double zScale,
                                                            double xTrans, double yTrans, double zTrans)
    {
    const bool identityScale = EqOneEps(xScale) && EqOneEps(yScale) && EqOneEps(zScale);
    const bool identityTranslate = EqZeroEps(xTrans) && EqZeroEps(yTrans) && EqZeroEps(zTrans);

    if (identityScale && identityTranslate)
        return TransfoModel::GetIdentity();
    if (identityTranslate)
        return TransfoModel(TransfoModelScaling::CreateFrom(xScale, yScale, zScale));
    if (identityScale)
        return TransfoModel(TransfoModelTranslating::CreateFrom(xTrans, yTrans, zTrans));


    return TransfoModel(TransfoModelScalingTranslating::CreateFrom(xScale, yScale, zScale, 
                                                                   xTrans, yTrans, zTrans));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel TransfoModel::CreateAffineFrom (const TransfoMatrix& affineMatrix)
    {
    // TDORAY: Make debug validation that ensure matrix is an affine
    SMStatus status;
    return CreateFrom(affineMatrix, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel::TransfoModel (Impl* implP)
    :   m_implP(implP),
        m_classID(implP->_GetClassID())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel::TransfoModel (const ImplPtr& implPtr)
    :   m_implP(implPtr),
        m_classID(implPtr->_GetClassID())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel::~TransfoModel ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel::TransfoModel (const TransfoModel& rhs)
    :   m_implP(rhs.m_implP),
        m_classID(rhs.m_classID)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel& TransfoModel::operator= (const TransfoModel& rhs)
    {
    m_implP = rhs.m_implP;
    m_classID = rhs.m_classID;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TransfoModel::IsIdentity () const
    {
    return TransfoModelIdentity::s_GetClassID() == m_classID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TransfoModel::IsConvertibleToMatrix () const
    {
    return m_implP->_IsConvertibleToMatrix();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix TransfoModel::ConvertToMatrix () const
    {
    return m_implP->_ConvertToMatrix();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool TransfoModel::IsEquivalent (const TransfoModel& rhs) const
    {
    if (m_classID != rhs.m_classID)
        return false;

    // TDORAY: Implement better behavior?
    

    if (!m_implP->_IsConvertibleToMatrix() || !rhs.m_implP->_IsConvertibleToMatrix())
        {
        assert(!"Bad");
        return false;
        }

    return m_implP->_ConvertToMatrix() == rhs.m_implP->_ConvertToMatrix();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus TransfoModel::Transform(const DPoint3d& sourcePt,
                                                DPoint3d&       targetPt) const
    {
    return m_implP->_Transform(sourcePt, targetPt);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus TransfoModel::Transform(const DPoint3d* sourcePtP,
                                                size_t          sourcePtQty,
                                                DPoint3d*       targetPtP) const
    {
    return m_implP->_Transform(sourcePtP, sourcePtQty, targetPtP);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus TransfoModel::Append(const TransfoModel& rhs)
    {
    SMStatus status;
    TransfoModel result(Combine(*this, rhs, status));

    if (S_SUCCESS != status)
        return S_ERROR;
    
    Handler::Swap(*this, result);
    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus TransfoModel::Inverse()
    {
    SMStatus status;
    TransfoModel result(InverseOf(*this, status));

    if (S_SUCCESS != status)
        return S_ERROR;

    Handler::Swap(*this, result);
    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void swap  (TransfoModel& lhs,
            TransfoModel& rhs)
    {
    Handler::Swap(lhs, rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel InverseOf (const TransfoModel& transform)
    {
    SMStatus status;
    TransfoModel resulting(InverseOf(transform, status));

    if (SMStatus::S_SUCCESS != status)
        throw CustomException(L"Inverse of specified transform does not exist");

    return resulting;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel InverseOf (const TransfoModel&     transform,
                        SMStatus&   status)
    {
    std::auto_ptr<TransfoModelBase> baseP(Handler::CreateInverse(transform));
    if (0 == baseP.get())
        {
        status = SMStatus::S_ERROR;
        return TransfoModel::GetIdentity();
        }

    status = SMStatus::S_SUCCESS;
    return Handler::CreateFrom(baseP.release());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel Combine   (const TransfoModel& lhs,
                        const TransfoModel& rhs)
    {
    SMStatus status;
    TransfoModel result(Combine(lhs, rhs, status));

    if (SMStatus::S_SUCCESS != status)
        throw CustomException(L"Error combining transfo models!"); 
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoModel Combine   (const TransfoModel&     lhs,
                        const TransfoModel&     rhs,
                        SMStatus&   status)
    {
    if (Handler::IsConvertibleToMatrix(lhs) && Handler::IsConvertibleToMatrix(rhs))
        return TransfoModel::CreateFrom(Handler::ConvertToMatrix(rhs)*Handler::ConvertToMatrix(lhs), status);

    status = SMStatus::S_SUCCESS;

    if (Handler::OfType<TransfoModelIdentity>(lhs))
        return rhs;

    if (Handler::OfType<TransfoModelIdentity>(rhs))
        return lhs;

    return TransfoModel(TransfoModelCombined::CreateFrom(lhs, rhs));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix& TransfoMatrix::operator*= (const TransfoMatrix& rhs)
    {
    Transform lhsGeom(ToTransform3d(*this));
    const Transform rhsGeom(ToTransform3d(rhs));


    bsiTransform_multiplyTransformTransform(&lhsGeom, &lhsGeom, &rhsGeom);

    *this = FromTransform3d(lhsGeom);
    return *this;
    }   

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus TransfoMatrix::Inverse()
    {
    Transform transform(ToTransform3d(*this));
    if (!transform.InverseOf(transform))
        return S_ERROR;

    *this = FromTransform3d(transform);
    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool operator==    (const TransfoMatrix&    lhs,
                    const TransfoMatrix&    rhs)
    {
    struct EqualWithEpsilon
        {
        bool operator () (double lhs, double rhs) const
            {
            return EqEps(lhs, rhs);
            }
        };

    return std::equal(lhs.m_parameters[0], lhs.m_parameters[0] + 12, 
                      rhs.m_parameters[0],
                      EqualWithEpsilon());
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TransfoMatrix   operator*  (const TransfoMatrix&    lhs,
                            const TransfoMatrix&    rhs)
    {
    Transform lhsGeom(ToTransform3d(lhs));
    const Transform rhsGeom(ToTransform3d(rhs));


    bsiTransform_multiplyTransformTransform(&lhsGeom, &lhsGeom, &rhsGeom);

    return FromTransform3d(lhsGeom);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GEOCOORDS_DLLE TransfoMatrix InverseOf (const TransfoMatrix&    matrix)
    {
    Transform transform(ToTransform3d(matrix));
    if (!transform.InverseOf(transform))
        throw CustomException(L"Could not invert matrix");

    return FromTransform3d(transform);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GEOCOORDS_DLLE TransfoMatrix InverseOf (const TransfoMatrix&    matrix,
                                        SMStatus&  status)
    {
    Transform transform(ToTransform3d(matrix));
    transform.InverseOf(transform) ? SMStatus::S_SUCCESS : SMStatus::S_ERROR;

    return FromTransform3d(transform);
    }


END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
