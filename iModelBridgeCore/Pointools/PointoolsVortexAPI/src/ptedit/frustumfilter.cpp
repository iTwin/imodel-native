#include "PointoolsVortexAPIInternal.h"

#include <ptedit/frustumfilter.h>

using namespace ptedit;
using namespace pt;

void FrustumSelect::buildFromScreenRect(const pt::Recti &r, const pt::ViewParams &vparams, double units)
{
    vector3 x1y1_n(static_cast<float>(r.lower(0)), static_cast<float>(r.lower(1)), 0.005f);
	vector3 x1y1_f(static_cast<float>(r.lower(0)), static_cast<float>(r.lower(1)), 0.9995f);
	vector3 x1y2_n(static_cast<float>(r.lower(0)), static_cast<float>(r.upper(1)), 0.005f);
	vector3 x1y2_f(static_cast<float>(r.lower(0)), static_cast<float>(r.upper(1)), 0.9995f);
	vector3 x2y2_n(static_cast<float>(r.upper(0)), static_cast<float>(r.upper(1)), 0.005f);
	vector3 x2y2_f(static_cast<float>(r.upper(0)), static_cast<float>(r.upper(1)), 0.9995f);
	vector3 x2y1_n(static_cast<float>(r.upper(0)), static_cast<float>(r.lower(1)), 0.005f);
	vector3 x2y1_f(static_cast<float>(r.upper(0)), static_cast<float>(r.lower(1)), 0.9995f);
	
	vparams.unproject3vTV(corners[0], x1y1_n);
	vparams.unproject3vTV(corners[1], x2y1_n);
	vparams.unproject3vTV(corners[2], x2y2_n);
	vparams.unproject3vTV(corners[3], x1y2_n);

	vparams.unproject3vTV(corners[4], x1y1_f);
	vparams.unproject3vTV(corners[5], x2y1_f);
	vparams.unproject3vTV(corners[6], x2y2_f);
	vparams.unproject3vTV(corners[7], x1y2_f);

	for (int i=0; i<8; i++)
		corners[i] /= units;

	planes[0].from3points(corners[4], corners[7], corners[3]);
	planes[1].from3points(corners[2], corners[6], corners[5]);
	planes[2].from3points(corners[2], corners[3], corners[7]);
	planes[3].from3points(corners[1], corners[5], corners[4]);
	planes[4].from3points(corners[0], corners[3], corners[2]);
	planes[5].from3points(corners[5], corners[6], corners[7]);

	valid = true;
}

bool FrustumSelect::writeState( pt::datatree::Branch *b) const 
{
	return (b->addBlob("corners", sizeof(corners), (const void*)corners, true)
		&& b->addBlob("planes", sizeof(planes), (const void*)planes, true));
};
bool FrustumSelect::readState(const pt::datatree::Branch *b)
{
	memcpy(corners, b->getBlob("corners")->_data, sizeof(corners));
	memcpy(planes, b->getBlob("planes")->_data, sizeof(planes));
	valid = true;
	return true;
}
