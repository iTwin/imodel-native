#include <ptengine/pointlayers.h>

using namespace pointsengine;

static pt::String null("");

bool PointLayersState::initialize()
{
	_visibleLayersBitMask = 1;
	_currentLayerBitMask = 1;

	_names[0] = "Layer 1";
	_names[1] = "Layer 2";
	_names[2] = "Layer 3";
	_names[3] = "Layer 4";
	_names[4] = "Layer 5";
	_names[5] = "Layer 6";
	_names[6] = "Layer 7";
	_names[7] = "Layer 8";
	_names[8] = "Layer 9";
	_names[9] = "Layer 10";
	_names[10] = "Layer 11";
	_names[11] = "Layer 12";
	_names[12] = "Layer 13";
	_names[13] = "Layer 14";
	_names[14] = "Layer 15";
	_names[15] = "Layer 16";

	return true;
}
void PointLayersState::setLayerName( int lyr, const pt::String &name )
{
	_names[lyr] = name;
}
const pt::String &PointLayersState::getLayerName( int lyr ) const
{
	if (lyr < 32 && lyr >= 0)
		return _names[lyr];
	return null;
}