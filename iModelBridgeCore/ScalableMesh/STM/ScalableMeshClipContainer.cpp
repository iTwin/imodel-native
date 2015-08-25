#include <ScalableMeshPCH.h>

#include "ScalableMeshClipContainer.h"
#include "ScalableMeshQuery.h"


/*------------------------------------------------------------------------------
IScalableMeshClipInfo Interface
--------------------------------------------------------------------------------*/
IScalableMeshClipInfoPtr IScalableMeshClipInfo::Create(DPoint3d* clipPointsP, size_t numberOfPoints, bool isClipMask)
    {
    return new ScalableMeshClipInfo(clipPointsP, numberOfPoints, isClipMask);
    }

IScalableMeshClipInfo::IScalableMeshClipInfo()
    {
    }

IScalableMeshClipInfo::~IScalableMeshClipInfo()
    {
    }

DPoint3d* IScalableMeshClipInfo::GetClipPoints()
    {
    return _GetClipPoints();
    }

size_t IScalableMeshClipInfo::GetNbClipPoints() const
    {
    return _GetNbClipPoints();
    }

bool IScalableMeshClipInfo::IsClipMask() const
    {
    return _IsClipMask();
    }
/*------------------------------------------------------------------------------
IScalableMeshClipInfo Interface End
--------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
IScalableMeshClipContainer Interface
--------------------------------------------------------------------------------*/
IScalableMeshClipContainerPtr IScalableMeshClipContainer::Create()
    {
    return new ScalableMeshClipContainer();
    }

IScalableMeshClipContainer::IScalableMeshClipContainer()
    {
    }

IScalableMeshClipContainer::~IScalableMeshClipContainer()
    {
    }

 int IScalableMeshClipContainer::AddClip(IScalableMeshClipInfoPtr& clipInfo) 
    {
    return _AddClip(clipInfo);
    }

size_t IScalableMeshClipContainer::GetNbClips() const
    {
    return _GetNbClips();
    }

int IScalableMeshClipContainer::GetClip(IScalableMeshClipInfoPtr& clipInfo, size_t clipInd) const
    {
    return _GetClip(clipInfo, clipInd);
    }        

int IScalableMeshClipContainer::RemoveClip(size_t toRemoveClipInd) 
    {
    return _RemoveClip(toRemoveClipInd);
    }
/*------------------------------------------------------------------------------
IScalableMeshClipContainer Interface End
--------------------------------------------------------------------------------*/
ScalableMeshClipInfo::ScalableMeshClipInfo (DPoint3d* clipPointsP, size_t numberOfPoints, bool isClipMask)
    {
    assert(clipPointsP != 0 && numberOfPoints > 0);

    m_isClipMask = isClipMask;

    for (size_t pointInd = 0; pointInd < numberOfPoints; pointInd++)
        {
        m_points.push_back(clipPointsP[pointInd]);
        }
    }

ScalableMeshClipInfo::~ScalableMeshClipInfo()
    {   
    }

DPoint3d* ScalableMeshClipInfo::_GetClipPoints()
    {
    return &m_points[0];
    }

size_t ScalableMeshClipInfo::_GetNbClipPoints() const
    {
    return m_points.size();
    }

bool ScalableMeshClipInfo::_IsClipMask() const
    {
    return m_isClipMask;
    }

ScalableMeshClipContainer::ScalableMeshClipContainer()
    {
    }

ScalableMeshClipContainer::~ScalableMeshClipContainer()
    {
    }

int ScalableMeshClipContainer::_AddClip(IScalableMeshClipInfoPtr& clipInfo) 
    {
    m_clipInfoVector.push_back(clipInfo);

    return SUCCESS;
    }
        
size_t ScalableMeshClipContainer::_GetNbClips() const 
    {
    return m_clipInfoVector.size();
    }

int ScalableMeshClipContainer::_GetClip(IScalableMeshClipInfoPtr& clipInfo, size_t clipInd) const 
    {
    int status = ERROR; 

    if (clipInd < m_clipInfoVector.size())
        {
        clipInfo = m_clipInfoVector[clipInd];
        status = SUCCESS;
        }
    
    return status; 
    }

int ScalableMeshClipContainer::_RemoveClip(size_t toRemoveClipInd)
    {
    int status = ERROR; 

    if (toRemoveClipInd < m_clipInfoVector.size())
        {
        ScalableMeshClipInfoVector::iterator clipIter(m_clipInfoVector.begin());    
    
        for (size_t clipInd = 0; clipInd < toRemoveClipInd; clipInd++)
            {
            clipIter++;
            }

        m_clipInfoVector.erase(clipIter);        

        status = SUCCESS;
        }

    return status;
    }
