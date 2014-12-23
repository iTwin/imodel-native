#ifndef POINTOOLS_SINGLETON_IMPLEMENTATION
#define POINTOOLS_SINGLETON_IMPLEMENTATION

#include <vector>
//-------------------------------------------------
// Globally (trans-dll) accessable Singleton
//-------------------------------------------------

namespace pt
{
	std::vector <void*> __singletons;

template 
<class T> SingletonHolder
{
	SingletonHolder() : __idx(0) {}

	inline T* instance()
	{
        if (!_instanceIndex)
        {
            makeInstance();
        }
        return *pInstance_;
	}
private:
	inline makeInstance()
	{
		__singletons.push_back(new T);
	}
	int __idx;
};
}
#endif