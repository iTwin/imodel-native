#include "PointoolsVortexAPIInternal.h"

#include <ptengine/VoxelLODSet.h>

#include <algorithm>



namespace pointsengine
{

VoxelLOD::VoxelLOD(void)
{
	setClientID(ClientID_Null);
	setRequestLOD(LOD_Null);
}


VoxelLOD::VoxelLOD(ClientID id)
{
	setClientID(id);
	setRequestLOD(LOD_Null);
}


void VoxelLOD::setClientID(ClientID id)
{
	clientID = id;
}


VoxelLOD::ClientID VoxelLOD::getClientID(void) const
{
	return clientID;
}


void VoxelLOD::setRequestLOD(LOD lod)
{
	requestLOD = lod;
}


VoxelLOD::LOD VoxelLOD::getRequestLOD(void) const
{
	return requestLOD;
}


bool VoxelLOD::operator==(const VoxelLOD &voxelLOD) const
{
	return (getClientID() == voxelLOD.getClientID() && getRequestLOD() == voxelLOD.getRequestLOD());
}


bool VoxelLOD::operator==(ClientID id) const
{
	return (getClientID() == id);
}


VoxelLODSet::VoxelLODSet(void)
{
	clear();
}


void VoxelLODSet::clear(void)
{
    std::lock_guard<std::recursive_mutex> lock(getMutex());

	voxelLODSet.clear();

	setRequestLODMax(LOD_Null);
}


unsigned int VoxelLODSet::getNumEntries(void)
{
	return static_cast<uint>(voxelLODSet.size());
}

VoxelLOD *VoxelLODSet::createVoxelLOD(ClientID id)
{
    std::lock_guard<std::recursive_mutex> lock(getMutex());

	voxelLODSet.push_back(VoxelLOD(id));

	return &(voxelLODSet.back());
}


const VoxelLOD *VoxelLODSet::getVoxelLOD(ClientID id) const
{
    std::lock_guard<std::recursive_mutex> lock(mutex);

	if(isValidClientID(id))
	{
		VoxelLODArray::const_iterator it;
		
		if((it = std::find(voxelLODSet.begin(), voxelLODSet.end(), id)) != voxelLODSet.end())
		{
			return &(*it);
		}
	}

	return NULL;
}


VoxelLOD *VoxelLODSet::getVoxelLOD(ClientID id)
{
    std::lock_guard<std::recursive_mutex> lock(getMutex());

	if(isValidClientID(id))
	{
		VoxelLODArray::iterator it;

		if((it = std::find(voxelLODSet.begin(), voxelLODSet.end(), id)) != voxelLODSet.end())
		{
			return &(*it);
		}
	}

	return NULL;
}


bool VoxelLODSet::setRequestLOD(ClientID id, LOD requestLOD)
{
	VoxelLOD *	voxelLOD;
	bool		updateMax = false;

    std::lock_guard<std::recursive_mutex> lock(getMutex());

															// Validate client ID
	if(isValidClientID(id) == false)
		return false;
															// Get the existing entry
	if((voxelLOD = getVoxelLOD(id)) == NULL)
	{
															// If it doesn't exist, create it
		if((voxelLOD = createVoxelLOD(id)) == NULL)
			return false;
	}

															// If request LOD is highest, remember the highest
	if(requestLOD > getRequestLODMax())
	{
		setRequestLODMax(requestLOD);
	}
	else													// If client is higest and LOD is reducing
	if(voxelLOD->getRequestLOD() == getRequestLODMax() && requestLOD < voxelLOD->getRequestLOD())
	{
															// Recalculate max LOD
		updateMax = true;
	}

	voxelLOD->setRequestLOD(requestLOD);
															// If ID's LOD is zero, remove it
	if(requestLOD == LOD_Null)
	{
		remove(id);
	}
															// Recalculate max request LOD if necessary
	if(updateMax)
	{
		updateRequestLODMax();
	}
															// Return OK
	return true;
}


VoxelLODSet::LOD VoxelLODSet::getRequestLOD(ClientID id) const
{
	const VoxelLOD *voxelLOD;

    std::lock_guard<std::recursive_mutex> lock(mutex);

	if(isValidClientID(id))
	{
															// Get entry for client ID
		if((voxelLOD = getVoxelLOD(id)) == NULL)
			return LOD_Null;
															// Return entry's request LOD
		return voxelLOD->getRequestLOD();
	}
															// Return not found
	return LOD_Null;
}


bool VoxelLODSet::isValidClientID(ClientID id) const
{
	return id != ClientID_Null;
}


void VoxelLODSet::setRequestLODMax(LOD lod)
{
	requestLODMax = lod;
}

void VoxelLODSet::clipRequestLODMax(LOD maxLOD)
{
    std::lock_guard<std::recursive_mutex> lock(getMutex());

	bool clipped = false;

	VoxelLODArray::iterator begin, end, it;

	begin	= voxelLODSet.begin();
	end		= voxelLODSet.end();

	for(it = begin; it != end; it++)
	{
															// If LOD is larger
		if(it->getRequestLOD() > maxLOD)
		{
			it->setRequestLOD(maxLOD);
			clipped = true;
		}
	}
															// If the LOD range has been clipped
	if(clipped)
	{
															// Record the new maximum LOD
		setRequestLODMax(maxLOD);
	}
}


VoxelLODSet::LOD VoxelLODSet::getRequestLODMax(void) const
{
	return requestLODMax;
}


bool VoxelLODSet::remove(ClientID id)
{
	VoxelLOD *voxelLOD;

	bool updateMax = false;

    std::lock_guard<std::recursive_mutex> lock(getMutex());


	if(isValidClientID(id))
	{
		if((voxelLOD = getVoxelLOD(id)) == NULL)
			return false;
															// If removed item is highest request LOD, remember to update
		if(voxelLOD->getRequestLOD() == getRequestLODMax())
			updateMax = true;

		VoxelLODArray::iterator item = std::find(voxelLODSet.begin(), voxelLODSet.end(), id);
															// If found
		if(item != voxelLODSet.end())
		{
			voxelLODSet.erase(item);
															// If largest item removed, update now
			if(updateMax)
			{
				updateRequestLODMax();
			}

			return true;
		}
															// Return not found														
		return false;
	}

	return false;
}


void VoxelLODSet::updateRequestLODMax(void)
{
	LOD	max = LOD_Null;

    std::lock_guard<std::recursive_mutex> lock(getMutex());

	VoxelLODArray::iterator begin, end, it;

	begin	= voxelLODSet.begin();
	end		= voxelLODSet.end();

	for(it = begin; it != end; it++)
	{
		if(it->getRequestLOD() > max)
		{
			max = it->getRequestLOD();
		}
	}

	setRequestLODMax(max);
}






} // End ptengine namespace