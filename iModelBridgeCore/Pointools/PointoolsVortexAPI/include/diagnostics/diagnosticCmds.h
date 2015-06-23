#pragma once

#include <pt/datatree.h>

namespace pt
{
enum DiagnosticCmd
{
	GetEngineData,
	SelectVoxel,
	GetVoxelData,
};

struct DiagnosticData
{
	DiagnosticData( DiagnosticCmd cmd ) 
	{ 
		in_cmd = cmd; in_props = out_props = 0; num_out = 0; num_in = 0; size = sizeof(this);
	}

	size_t			size;
	DiagnosticCmd	in_cmd;

	pt::vector3d	in_pnts[16];
	double			in_double[4];
	__int64			in_int[4];
	bool			in_bool[4];

	pt::vector3d	out_pnts[16];
	double			out_double[4];
	__int64			out_int[4];
	bool			out_bool[4];

	datatree::Branch *in_props;
	datatree::Branch *out_props;

	int				num_out;
	int				num_in;
};
}	