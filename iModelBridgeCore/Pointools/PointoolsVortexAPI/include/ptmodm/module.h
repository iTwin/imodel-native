/*--------------------------------------------------------------------------*/ 
/*	Pointools module class definition										*/ 
/*  (C) 2003 Copyright Faraz Ravi - All Rights Reserved						*/ 
/*																			*/ 
/*  Last Updated 18 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef _POINTOOLS_MODULE_INTERFACE
#define _POINTOOLS_MODULE_INTERFACE

//
// Module Request Architecture
//
// Ensures that the module being requested is that being held. ie the interfaces
// are the same and therefore a void pointer to the object can be safely
// cast to the module interface. This is important in the plugin architecture
// to ensure that the plugin's module target's are consistant with the application.
//
// Translation interfaces should be provided for backward compatibility of
// a modules interface when it is updated - overwise plugin code will break
// and requires a re-compile with the new module headers.
//

namespace pt
{
struct RequestObj
{
	unsigned int identifier;
	unsigned int version;
	void* _module;
};
/*base module overide to provide id info					*/ 
struct _Module
{
	virtual unsigned int moduleID() const =0;
	virtual unsigned int versionID() const =0;
	bool operator == (const _Module &m)
	{
		return (moduleID() == m.moduleID()
			&& versionID() == m.versionID());
	}
	bool operator == (const RequestObj &r)
	{
		return (moduleID() == r.identifier
			&& versionID() == r.version);
	}
};
struct _NullModule : _Module
{
	virtual unsigned int moduleID() const { return 0; }
	virtual unsigned int versionID() const { return 0; }
};
/*module implementation, sets id and casts void pointer		*/ 
template <class MODULE, unsigned int ID, unsigned int VER, class BASE_MODULE = _Module>
struct Module : public BASE_MODULE
{
	unsigned int moduleID() const { return ID; }
	unsigned int versionID() const { return VER; }

	/*request obj*/ 
	class RequestObj : public pt::RequestObj
	{
	public:
		RequestObj() { identifier = ID ; version = VER; _module = 0; };
		~RequestObj() {};

		MODULE* module() { 
			if (_module) return reinterpret_cast<MODULE*>(_module);
				else return 0; }
	};
};
}
#endif