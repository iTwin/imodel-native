#include <pt/ParameterMap.h>
#include <ptappdll/functionality.h>

#include <map>

using namespace ptapp;

namespace ptapp
{
	struct Functionality
	{
		FuncCode _function;
		FuncCode _authentication;
		pt::ParameterMap *_data;
	};
	typedef std::map<FuncCode, pt::ParameterMap*> FUNCMAP;
	FUNCMAP _funcs;
}

void ptapp::addFunctionality(void *_f)
{
	Functionality *f = (Functionality*)_f;
	/* authenticate */ 

	/*insert */ 
	_funcs.insert(FUNCMAP::value_type(f->_function, f->_data));	
}
pt::ParameterMap *ptapp::functionality(FuncCode f)
{
	FUNCMAP::iterator i = _funcs.find(f);
	return (i == _funcs.end()) ? 0 : i->second;
}
