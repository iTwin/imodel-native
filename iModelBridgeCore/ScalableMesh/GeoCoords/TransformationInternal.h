/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/TransformationInternal.h $
|    $RCSfile: TransformationInternal.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/11/07 14:26:47 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TransfoModelBase.h"

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE             

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransfoModelBaseHandler
    {
    static TransfoModelBase::ClassID    GetClassID                     (const TransfoModelBase&         tranform)
        { return tranform._GetClassID(); }

    template <typename T>
    static bool                         OfType                         (const TransfoModelBase&         tranform)
        {
        return typename T::s_GetClassID() == GetClassID(tranform);
        }

    static bool                         IsConvertibleToMatrix          (const TransfoModelBase&         tranform)
        { return tranform._IsConvertibleToMatrix(); }

    static TransfoMatrix                ConvertToMatrix                (const TransfoModelBase&         tranform)
        { return tranform._ConvertToMatrix(); }

    static TransfoModel::Status         Transform                      (const TransfoModelBase&         tranform,
                                                                        const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt)
        { return tranform._Transform(sourcePt, targetPt); }


    static TransfoModel::Status         Transform                      (const TransfoModelBase&         tranform,
                                                                        const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP)
        { return tranform._Transform(sourcePtP, sourcePtQty, targetPtP); }


    static TransfoModelBase*            CreateInverse                  (const TransfoModelBase&         tranform)
        { return tranform._CreateInverse(); }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransfoModelHandler
    {
    static const TransfoModelBase&      GetBase                        (const TransfoModel&             tranform)
        { return *tranform.m_implP; }

    static TransfoModelBase&            GetBase                        (TransfoModel&                   tranform)
        { return *tranform.m_implP; }

    static TransfoModelBaseCPtr         GetBasePtr                     (const TransfoModel&             tranform)
        { return tranform.m_implP; }


    static TransfoModelBasePtr          GetBasePtr                     (TransfoModel&                   tranform)
        { return tranform.m_implP; }

    static TransfoModel                 CreateFrom                     (TransfoModelBase*               tranformBaseP)
        { return TransfoModel(tranformBaseP); }

    static TransfoModel                 CreateFrom                     (const TransfoModelBaseCPtr&     tranformBasePtr)
        { return TransfoModel(tranformBasePtr.get()); }

    static TransfoModel::ClassID        GetClassID                     (const TransfoModel&             tranform)
        { return tranform.m_classID; }

    template <typename T>
    static bool                         OfType                         (const TransfoModel&             tranform)
        {
        return typename T::s_GetClassID() == GetClassID(tranform);
        }

    static bool                         IsConvertibleToMatrix          (const TransfoModel&             tranform)
        { return tranform.m_implP->_IsConvertibleToMatrix(); }

    static TransfoMatrix                ConvertToMatrix                (const TransfoModel&             tranform)
        { return tranform.m_implP->_ConvertToMatrix(); }

    static TransfoModel::Status         Transform                      (const TransfoModel&             tranform,
                                                                        const DPoint3d&                 sourcePt,
                                                                        DPoint3d&                       targetPt)
        { return tranform.m_implP->_Transform(sourcePt, targetPt); }


    static TransfoModel::Status         Transform                      (const TransfoModel&             tranform,
                                                                        const DPoint3d*                 sourcePtP,
                                                                        size_t                          sourcePtQty,
                                                                        DPoint3d*                       targetPtP)
        { return tranform.m_implP->_Transform(sourcePtP, sourcePtQty, targetPtP); }


    static TransfoModelBase*            CreateInverse                  (const TransfoModel&             tranform)
        { return tranform.m_implP->_CreateInverse(); }

    static void                         Swap                           (TransfoModel&                   lhs,
                                                                        TransfoModel&                   rhs)
        {
        using namespace std;

        swap(lhs.m_implP, rhs.m_implP);
        std::swap(lhs.m_classID, rhs.m_classID);
        }
    };


namespace {

// TDORAY: Consider removing these
typedef TransfoModelHandler             Handler;
typedef TransfoModelBaseHandler         BaseHandler;

static_assert(sizeof(Transform) == sizeof(TransfoMatrix), "Transform and TransfoMatrix Types have incompatible layout!");

inline const Transform&                 ToBSI                          (const TransfoMatrix&            transform)
    {
    return reinterpret_cast<const Transform&>(transform);
    }

inline const TransfoMatrix&             FromBSI                        (const Transform&                transform)
    {
    return reinterpret_cast<const TransfoMatrix&>(transform);
    }


inline Transform                     ToTransform3d                  (const TransfoMatrix&            m)
    {
    Transform transform;
    bsiTransform_initFromRowValues(&transform, m[0][0], m[0][1], m[0][2],  m[0][3],
                                                  m[1][0], m[1][1], m[1][2],  m[1][3],
                                                  m[2][0], m[2][1], m[2][2],  m[2][3]);

    return transform;
    }

inline TransfoMatrix                    FromTransform3d                (const Transform&             m)
    {
    RotMatrix mat;
    m.GetMatrix(mat);
    DVec3d col1, col2, col3,trans; 
    mat.GetColumns(col1, col2, col3);
    m.GetTranslation(trans);
    return TransfoMatrix(col1.x, col2.x, col3.x, trans.x,
                         col1.y, col2.y, col3.y, trans.y,
                         col1.z, col2.z, col3.z, trans.z);
    }
}


END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
