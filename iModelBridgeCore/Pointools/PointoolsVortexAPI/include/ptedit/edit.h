#pragma once

#define SELECTED_PNT_BIT 128
#define EDT_MAX_THREADS 4

namespace ptedit
{
enum SelectionResult
{
	FullyOutside = 0,
	PartiallyInside = 1,
	FullyInside = 2
};
enum FilterType
{
	SelectionFilterType = 0,
	PaintFilterType = 1,
	ClonePointsFilterType = 2
};
/* point buffer */ 
/*
ffffffff - code indicates voxel index next
284784 - voxel ptr
ffffff00 - code indicates offset
x - offset float cast
y - offset float cast
z - offset float cast
382 - point index
*/
#define PNT_BUFFER_VOXEL_CODE 0xffffffff
#define PNT_BUFFER_OFFSET_CODE 0xffffff00
#define MIN_FILTER 0.0625f
}