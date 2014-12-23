#ifndef POINTOOLS_POINTS_ENGINE_MODULE
#define POINTOOLS_POINTS_ENGINE_MODULE

namespace pointsengine
{
class Module
{
public:
	Module() : _paused(false) {};
	virtual ~Module(){};

	virtual bool pause()	{ _paused = true; return true; }
	virtual bool unpause()	{ _paused = false; return true; }
	inline bool paused() const { return _paused; }

	virtual bool initialize() { return false; }

protected:
	bool _paused;
	bool _flags[3];
};
}

#endif