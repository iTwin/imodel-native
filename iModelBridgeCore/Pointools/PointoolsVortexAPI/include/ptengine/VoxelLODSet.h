
#pragma once

#include <vector>

#include <boost/thread/recursive_mutex.hpp>

namespace pointsengine
{

const float				LOD_Null		= 0;
const int				ClientID_Null	= -1;


class VoxelLOD
{
public:

		typedef int				ClientID;
		typedef float			LOD;

protected:

		ClientID				clientID;
		LOD						requestLOD;


public:

								VoxelLOD				(void);
								VoxelLOD				(ClientID id);

		void					setClientID				(ClientID id);
		ClientID				getClientID				(void) const;

		void					setRequestLOD			(LOD lod);
		LOD						getRequestLOD			(void) const;

		bool					operator ==				(const VoxelLOD &voxelLOD) const;

		bool					operator ==				(ClientID id) const;
};


class VoxelLODSet
{

public:

	typedef VoxelLOD::ClientID		ClientID;
	typedef VoxelLOD::LOD			LOD;
	typedef boost::recursive_mutex	Mutex;

protected:

	typedef std::vector<VoxelLOD>	VoxelLODArray;

protected:

		mutable Mutex		mutex;

		VoxelLODArray		voxelLODSet;

		VoxelLOD::LOD		requestLODMax;

protected:

		Mutex			&	getMutex			(void)			{return mutex;}

		void				setRequestLODMax	(LOD lod);

		VoxelLOD		*	createVoxelLOD		(ClientID id);
		const VoxelLOD	*	getVoxelLOD			(ClientID id) const;
		VoxelLOD		*	getVoxelLOD			(ClientID id);

		void				updateRequestLODMax	(void);
public:

							VoxelLODSet			(void);

		void				clear				(void);

		unsigned int		getNumEntries		(void);

		bool				setRequestLOD		(ClientID id, LOD requestLOD);
		LOD					getRequestLOD		(ClientID id) const;

		void				clipRequestLODMax	(LOD maxLOD);
		LOD					getRequestLODMax	(void) const;	

		bool				remove				(ClientID id);

		bool				isValidClientID		(ClientID id) const;

};



} // End ptengine namespace
