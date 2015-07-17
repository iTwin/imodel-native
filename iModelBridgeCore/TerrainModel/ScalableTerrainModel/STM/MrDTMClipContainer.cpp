#include <ScalableTerrainModelPCH.h>

#include "MrDTMClipContainer.h"


/*------------------------------------------------------------------------------
IMrDTMClipInfo Interface
--------------------------------------------------------------------------------*/
IMrDTMClipInfoPtr IMrDTMClipInfo::Create(DPoint3d* clipPointsP, size_t numberOfPoints, bool isClipMask)
    {
    return new MrDTMClipInfo(clipPointsP, numberOfPoints, isClipMask);
    }

IMrDTMClipInfo::IMrDTMClipInfo()
    {
    }

IMrDTMClipInfo::~IMrDTMClipInfo()
    {
    }

DPoint3d* IMrDTMClipInfo::GetClipPoints()
    {
    return _GetClipPoints();
    }

size_t IMrDTMClipInfo::GetNbClipPoints() const
    {
    return _GetNbClipPoints();
    }

bool IMrDTMClipInfo::IsClipMask() const
    {
    return _IsClipMask();
    }
/*------------------------------------------------------------------------------
IMrDTMClipInfo Interface End
--------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
IMrDTMClipContainer Interface
--------------------------------------------------------------------------------*/
IMrDTMClipContainerPtr IMrDTMClipContainer::Create()
    {
    return new MrDTMClipContainer();
    }

IMrDTMClipContainer::IMrDTMClipContainer()
    {
    }

IMrDTMClipContainer::~IMrDTMClipContainer()
    {
    }

 int IMrDTMClipContainer::AddClip(IMrDTMClipInfoPtr& clipInfo) 
    {
    return _AddClip(clipInfo);
    }

size_t IMrDTMClipContainer::GetNbClips() const
    {
    return _GetNbClips();
    }

int IMrDTMClipContainer::GetClip(IMrDTMClipInfoPtr& clipInfo, size_t clipInd) const
    {
    return _GetClip(clipInfo, clipInd);
    }        

int IMrDTMClipContainer::RemoveClip(size_t toRemoveClipInd) 
    {
    return _RemoveClip(toRemoveClipInd);
    }
/*------------------------------------------------------------------------------
IMrDTMClipContainer Interface End
--------------------------------------------------------------------------------*/
MrDTMClipInfo::MrDTMClipInfo (DPoint3d* clipPointsP, size_t numberOfPoints, bool isClipMask)
    {
    assert(clipPointsP != 0 && numberOfPoints > 0);

    m_isClipMask = isClipMask;

    for (size_t pointInd = 0; pointInd < numberOfPoints; pointInd++)
        {
        m_points.push_back(clipPointsP[pointInd]);
        }
    }

MrDTMClipInfo::~MrDTMClipInfo()
    {   
    }

DPoint3d* MrDTMClipInfo::_GetClipPoints()
    {
    return &m_points[0];
    }

size_t MrDTMClipInfo::_GetNbClipPoints() const
    {
    return m_points.size();
    }

bool MrDTMClipInfo::_IsClipMask() const
    {
    return m_isClipMask;
    }

MrDTMClipContainer::MrDTMClipContainer()
    {
    }

MrDTMClipContainer::~MrDTMClipContainer()
    {
    }

int MrDTMClipContainer::_AddClip(IMrDTMClipInfoPtr& clipInfo) 
    {
    m_clipInfoVector.push_back(clipInfo);

    return SUCCESS;
    }
        
size_t MrDTMClipContainer::_GetNbClips() const 
    {
    return m_clipInfoVector.size();
    }

int MrDTMClipContainer::_GetClip(IMrDTMClipInfoPtr& clipInfo, size_t clipInd) const 
    {
    int status = ERROR; 

    if (clipInd < m_clipInfoVector.size())
        {
        clipInfo = m_clipInfoVector[clipInd];
        status = SUCCESS;
        }
    
    return status; 
    }

int MrDTMClipContainer::_RemoveClip(size_t toRemoveClipInd)
    {
    int status = ERROR; 

    if (toRemoveClipInd < m_clipInfoVector.size())
        {
        MrDTMClipInfoVector::iterator clipIter(m_clipInfoVector.begin());    
    
        for (size_t clipInd = 0; clipInd < toRemoveClipInd; clipInd++)
            {
            clipIter++;
            }

        m_clipInfoVector.erase(clipIter);        

        status = SUCCESS;
        }

    return status;
    }
