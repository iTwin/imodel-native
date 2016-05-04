#pragma once

#include <pt/geomtypes.h>

namespace pt
{
enum DiagnosticCmd
{
	GetEngineData,
	SelectVoxel,
	GetVoxelData
};

struct VoxelData
{
	PTuint	id;
	PTubyte fullLayers;
	PTubyte partLayers;
	PTbool	wholeSelected;
	PTbool	partSelected;
	PTuint  editPoint;
	PTbool  editChannel;

	PTfloat	lod;
	PTfloat requestLod;
};
struct DiagnosticData
{
	DiagnosticData( DiagnosticCmd cmd ) 
	{ 
		in_cmd = cmd; num_out = 0; num_in = 0; size = sizeof(DiagnosticData);
	}

	size_t			size;
	DiagnosticCmd	in_cmd;

	pt::vector3d	in_pnts[16];
	double			in_double[4];
	int64_t			in_int[4];
	bool			in_bool[4];
	PThandle		in_handles[4];

	pt::vector3d	out_pnts[16];
	double			out_double[4];
	int64_t			out_int[4];
	bool			out_bool[4];
	
	int				num_out;
	int				num_in;

	VoxelData		voxel_data;
};
}	