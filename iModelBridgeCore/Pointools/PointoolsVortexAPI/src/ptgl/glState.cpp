#include "PointoolsVortexAPIInternal.h"

#include <ptgl\glstate.h>

#include <iostream>

#include <loki\assocvector.h>
#include <map>

/*this doesn't deal with different GL contexts*/ 
using namespace ptgl;
namespace ptgl
{
	RenderStateSet _currentRenderStateSet[10];
	int _currentContext = 0;
	unsigned int _enableStateOverrides = 0;
	unsigned int _disableStateOverrides = 0;
	unsigned int _setParamOverrides = 0;
	unsigned int _unsetParamOverrides = 0;
}
void RenderStateSet::applyGlobal()	
{	
	apply(&_currentRenderStateSet[_currentContext]);	
}
RenderStateSet* RenderStateSet::globalSet()
{
	return &_currentRenderStateSet[_currentContext];
}
const RenderStateSet* ptgl::currentRenderStateSet()		
{	
	return &_currentRenderStateSet[_currentContext]; 
}
void ptgl::setContext(int context)
{
	_currentContext = context;
}
unsigned int RenderStateSet::overrideStateEnable()	{	return _enableStateOverrides;	}
unsigned int RenderStateSet::overrideStateDisable()	{	return _disableStateOverrides;	}
unsigned int RenderStateSet::overrideParamSet()		{	return _setParamOverrides;		}
unsigned int RenderStateSet::overrideParamUnset()	{	return _unsetParamOverrides;	}

void RenderStateSet::overrideEnable(RenderEnableState state)	{ _enableStateOverrides |= state; }
void RenderStateSet::overrideDisable(RenderEnableState state)	{ _disableStateOverrides |= state; }
void RenderStateSet::overrideSet(RenderParam state)				{ _setParamOverrides |= state; }
void RenderStateSet::overrideUnset(RenderParam state)			{ _unsetParamOverrides |= state; }	

void RenderStateSet::removeOverrideEnable(RenderEnableState state)	{ _enableStateOverrides &= ~state;	}
void RenderStateSet::removeOverrideDisable(RenderEnableState state)	{ _disableStateOverrides &= ~state; }
void RenderStateSet::removeOverrideSet(RenderParam state)			{ _setParamOverrides &= ~state;		}
void RenderStateSet::removeOverrideUnset(RenderParam state)			{ _unsetParamOverrides &= ~state;	}	

void RenderStateSet::resetOverrides()
{
	_enableStateOverrides = 0;
	_disableStateOverrides = 0;
	_setParamOverrides = 0;
	_unsetParamOverrides = 0;
}
void resetEnableOverrides()
{
	_enableStateOverrides = 0;
}
void resetDisableOverrides()
{
	_disableStateOverrides = 0;
}
void resetSetOverrides()
{
	_setParamOverrides = 0;
}
void resetUnsetOverrides()
{
	_unsetParamOverrides = 0;
}
