//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFSubDirIdIter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/h/HIterators.h>

namespace HTGFF {

/*---------------------------------------------------------------------------------**//**
* @description  Utility iterator that facilitate sub directory iteration. Conforms to
*               STL bidirectional iterator implicit interface.
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SubDirIdIter : public BentleyApi::ImagePP::BidirectionalIterator<SubDirIdIter, uint32_t>
    {
public:
    friend class                    TagFile;

    typedef value_type              DirectoryID;

    const DirectoryID*              m_pCurrentSubDirID;
    const DirectoryID*              m_SubDirIDBegin;
    const DirectoryID*              m_SubDirIDEnd;

    explicit                        SubDirIdIter                   (const DirectoryID*          pi_CurrentSubDirID,
                                                                    const DirectoryID* const    pi_SubDirIDBegin,
                                                                    const DirectoryID* const    pi_SubDirIDEnd);

    value_type                      Dereference                    () const;

     void                     Increment                      ();
     void                     Decrement                      ();

    bool                            EqualTo                        (const iterator_t&           pi_rRight) const;

public:
     explicit                 SubDirIdIter                   ();

    // Hide base class version in order to return by value instead than by reference
    value_type                      operator*                      ()
        {
        return Dereference();
        }
    value_type                      operator*                      () const             
        {
        return Dereference();
        }

    size_t                          ConvertToIndex                 () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline SubDirIdIter::value_type SubDirIdIter::Dereference () const
    {
    HPRECONDITION(0 != m_pCurrentSubDirID);
    return *m_pCurrentSubDirID;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool SubDirIdIter::EqualTo (const iterator_t& pi_rRight) const
    {
    return m_pCurrentSubDirID == pi_rRight.m_pCurrentSubDirID;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline size_t SubDirIdIter::ConvertToIndex () const
    {
    return distance(m_SubDirIDBegin, m_pCurrentSubDirID);
    }

} //End namespace HTGFF
