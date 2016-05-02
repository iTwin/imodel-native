#include "PointoolsVortexAPIInternal.h"

#include <gl/glew.h>
#include <ptedit/boxfilter.h>
#include <Winbase.h>

using namespace ptedit;
using namespace pt;


bool BoxSelect::writeState(pt::datatree::Branch *b) const 
{
	return b->addBlob("box", sizeof(box), static_cast<const void *>(&box), true);

	//return (b->addBlob("corners", sizeof(corners), (const void*)corners, true)
	//	&& b->addBlob("planes", sizeof(planes), (const void*)planes, true));
};

bool BoxSelect::readState(const pt::datatree::Branch *b)
{
	return memcpy(&box, b->getBlob("box")->_data, sizeof(box)) != 0;

	//memcpy(corners, b->getBlob("corners")->_data, sizeof(corners));
	//memcpy(planes, b->getBlob("planes")->_data, sizeof(planes));
	//valid = true;
	//return true;
}


bool OrientedBoxSelect::writeState(pt::datatree::Branch *b) const 
{
	bool result = true;

	result &= b->addBlob("box", sizeof(box), static_cast<const void *>(&box), true);

	result &= b->addBlob("pos", sizeof(position), static_cast<const void *>(&position), true);
	result &= b->addBlob("uAxis", sizeof(uAxis), static_cast<const void *>(&uAxis), true);
	result &= b->addBlob("vAxis", sizeof(vAxis), static_cast<const void *>(&vAxis), true);

	return result;
};

bool OrientedBoxSelect::readState(const pt::datatree::Branch *b)
{
	bool result = true;

	result &= memcpy(&box, b->getBlob("box")->_data, sizeof(box)) ? true : false;
	result &= memcpy(&position, b->getBlob("pos")->_data, sizeof(position)) ? true : false;
	result &= memcpy(&uAxis, b->getBlob("uAxis")->_data, sizeof(uAxis)) ? true : false;
	result &= memcpy(&vAxis, b->getBlob("vAxis")->_data, sizeof(vAxis)) ? true : false;

	setTransform(position.data(), uAxis.data(), vAxis.data());

	return result;
}
