//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HTGFFSubDirIdIter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFSubDirIdIter.h>

namespace HTGFF
{

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SubDirIdIter::SubDirIdIter ()
    :   m_pCurrentSubDirID(0),
        m_SubDirIDBegin(0),
        m_SubDirIDEnd(0)

    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SubDirIdIter::SubDirIdIter (const DirectoryID*          pi_CurrentSubDirID,
                            const DirectoryID* const    pi_SubDirIDBegin,
                            const DirectoryID* const    pi_SubDirIDEnd)
    :   m_pCurrentSubDirID(pi_CurrentSubDirID),
        m_SubDirIDBegin(pi_SubDirIDBegin),
        m_SubDirIDEnd(pi_SubDirIDEnd)
    {
    HPRECONDITION(pi_SubDirIDBegin == pi_CurrentSubDirID || pi_SubDirIDEnd == pi_CurrentSubDirID);

    if (0 == pi_CurrentSubDirID)
        {
        HASSERT(pi_SubDirIDBegin == pi_SubDirIDEnd);
        return;
        }

    // Ensure that a begin iterator does not start on an unused directory slot.
    if (pi_SubDirIDBegin == pi_CurrentSubDirID && 0 == *pi_CurrentSubDirID)
        Increment();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SubDirIdIter::Increment ()
    {
    HPRECONDITION(m_pCurrentSubDirID < m_SubDirIDEnd);
    while (m_pCurrentSubDirID < m_SubDirIDEnd && 0 == *++m_pCurrentSubDirID) {}
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SubDirIdIter::Decrement ()
    {
    HPRECONDITION(m_pCurrentSubDirID > m_SubDirIDBegin);
    while (m_pCurrentSubDirID > m_SubDirIDBegin && 0 == *++m_pCurrentSubDirID) {}
    }


} //End namespace HTGFF
