/*--------------------------------------------------------------------------*/ 
/*  Geometry.h																*/ 
/*	Geometry base class definition											*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 24 Nov 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_SCENEGRAPH_H
#define POINTOOLS_SCENEGRAPH_H

#include <pt/classes.h>

#include <pt/variant.h>
#include <pt/BoundingBox.h>
#include <pt/BoundingSphere.h>
#include <pt/DisplayInfo.h>
#include <pt/Transform.h>
#include <pt/ObjectBounds.h>
#include <pt/Flags.h>
#include <pt/Guid.h>

#include <ptfs/filepath.h>

#include <set>
#include <map>
#include <string>

#pragma warning (disable:4355) // 'this' : used in base member initializer list

namespace pt
{

enum DrawMode
{
	Interactive = 1,
	Front = 2,
	Back = 4,
	Safest = 8,
	Nicest = 16,
	Fastest = 32
};
typedef uint32 ObjectKey;
}
namespace ptgl { class Viewport; }
namespace pt
{
/* 120 so that object is on 16 byte boundary allowing 4bytes for vftptr and 4 for key*/ 
#ifdef PT_IDENTIFIER_LENGTH
#undef PT_IDENTIFIER_LENGTH
#endif
#define PT_IDENTIFIER_LENGTH 60

//-------------------------------------------------------------------------
// Output
/// Output base class for diagnostics and object composition display
//-------------------------------------------------------------------------
class CCLASSES_API Output
{
public:
	Output() { _level = 0; };
	virtual ~Output() { _level = 0; };
	virtual void CDECL_ATTRIBUTE outputf(const char* fmt ...) const;
	virtual void CDECL_ATTRIBUTE outputLinef(const char* fmt ...) const;
	void push() { _level++; }
	void pop() { _level--; }
	void root() { _level = 0; }
	int level() const { return _level; }

protected:
	int _level;
	virtual void tab() const;
};
enum PropertyType
{
	PropertyTypeValue = 0,
	PropertyTypeLocation = 1,
	PropertyTypeVector = 2,
	PropertyTypeSize = 3,
	PropertyTypeAngle = 4,
	PropertyTypeCount = 5,
	PropertyTypeSwitch = 6,
	PropertyTypeName = 7,
	PropertyTypeColor = 8
};
//-------------------------------------------------------------------------
// Property
//-------------------------------------------------------------------------
/*!
*	Property class
*/
#define DEFAULT_PROPERTY_COLOR  0x808080 // RGB(128,128,128)

struct Property
{
	template <class T>
	Property(const char *id_, const char *desc_, PropertyType type_, const T &value_, 
		unsigned int color_=DEFAULT_PROPERTY_COLOR)
	: value(value_), id(id_), desc(desc_), type(type_), color(color_) {}

	const char*		id;
	const char*		desc;
	PropertyType	type;
	unsigned int	color;
	Variant			value;
};
//-------------------------------------------------------------------------
// abstract class that visits object information / methods ie interface
//
class InterfaceVisitor
{
public:
	virtual void info(const Property &v)=0;
	virtual void property(const Property &v)=0;
	virtual void method(const char* id)=0;
	virtual void diagnostic(const char* id){};
};
//-------------------------------------------------------------------------
// Interface 
//-------------------------------------------------------------------------
/*!
*  Derived class may implement these using ParameterMap or any other
*  method - no contraints implementation
*/
class CCLASSES_API Interface
{
public:
	virtual void visitInterface(InterfaceVisitor *visitor) const	{};
	virtual int	setProperty(const char *id, const Variant &v)		{ return 0; };
	virtual int	getProperty(const char *id, Variant &v)				{ return 0; };
	virtual int invoke(const char *method)							{ return 0; };

protected:
	template <class T>
	static int _setProperty(const char *match_property, const char *property, const pt::Variant &v, T &val)
	{
		if (strcmp(match_property, property) ==0)
		{
			try { val = ttl::var::get<T>(v); } catch (const ttl::var::exception &) { return 0; }
			return 1;
		}
		return 0;
	}
};
//-------------------------------------------------------------------------
/// Base scriptable Object class for objects and modules
//-------------------------------------------------------------------------
/*!
 *  Identified object class
 */
class Object : public Interface
{
public:
	virtual ~Object(){};
	virtual void diagnostic(Output *output, bool recursive) const {};
	virtual void composition(Output *output, bool recursive) const {}; 

	inline const wchar_t* identifier() const { return m_identifier; }
	
	void setIdentifier(const wchar_t*id) { 
		wcsncpy_s(m_identifier, PT_IDENTIFIER_LENGTH, id, PT_IDENTIFIER_LENGTH-1); 
		m_identifier[PT_IDENTIFIER_LENGTH-1] = L'\0'; 
	}

	virtual const wchar_t *typeDescriptor() const { return L"_Object"; }
	virtual const char *objectClass() const { return "Object"; }
	virtual const char *className() const	{ return "Object"; }

	const ObjectKey	&key() const		{ return m_key; }
	void setKey(const ObjectKey &key)	{ m_key = key; }

	CCLASSES_API virtual void visitInterface(InterfaceVisitor *visitor) const;
	CCLASSES_API virtual int setProperty(const char *id, const Variant &v);
	CCLASSES_API virtual int getProperty(const char *id, Variant &v);

protected:
	wchar_t		m_identifier[PT_IDENTIFIER_LENGTH];
	ObjectKey	m_key;

	Object(const wchar_t*id) { setIdentifier(id ? id : L"Object"); };
};
//-------------------------------------------------------------------------
// Object3D
/// base class for all 3D object types
//-------------------------------------------------------------------------
class Object3D : public Object
{
public:
	virtual ~Object3D() {};

	/*access to aggregated objects				*/ 
	//inline UserTransform& transform() 				{ return m_transform; }
	//inline const UserTransform& transform() const	{ return m_transform; }
	//CCLASSES_API commitTransformations();

	inline Transform&		registration() 					{ return m_registration; }
	inline const Transform& registration() const			{ return m_registration; }

	inline DisplayInfo		&displayInfo()					{ return m_displayInfo; }
	inline const DisplayInfo &displayInfo() const			{ return m_displayInfo; }

	inline const Bounds3D	&localBounds() const			{ return m_localBounds; }
	inline const Bounds3DD	&projectBounds() const			{ return m_projectBounds; }
	inline Bounds3D			&localBounds() 					{ return m_localBounds; }
	inline Bounds3DD		&projectBounds() 				{ return m_projectBounds; }
		
	void					computeBounds()					{	_computeBounds(); };
	virtual const wchar_t	*typeDescriptor() const			{ return L"_Object3D"; }
	virtual const char		*objectClass() const			{ return "Object3D"; }
	virtual const char		*className() const				{ return "Object3D"; }

	virtual void			drawGL(uint32 drawmode, int millisecs, const ptgl::Viewport *viewport) {};
	virtual double			findNearestPoint(const vector3d &pnt, vector3d &nearest, CoordinateSpace cs) const
	{ return -1; };

	virtual bool isGroup() const							{ return false; }
	CCLASSES_API virtual bool invoke(wchar_t*command, void*data);	//! Implementation of basic show / hide commands
	
	CCLASSES_API virtual void parent(const Object3D *par);
	virtual const Object3D* parent() const { return m_parent; }
	int	depth() const;

	CCLASSES_API virtual void diagnostic(Output *output, bool recursive) const;
	CCLASSES_API virtual void visitInterface(InterfaceVisitor *visitor) const;

	CCLASSES_API virtual int setProperty(const char *id, const Variant &v);
	CCLASSES_API virtual int getProperty(const char *id, Variant &v);
	
	virtual Guid		objectGuid() const					{ return Guid(); }	// Unique persistent identifier of object, not instance (not the same as a PointCloud guid)

protected:
	Object3D(const wchar_t *id=0, const Object3D* owner=0);
	virtual void			_computeBounds()=0;

	static	ObjectKey		generateKey(const Object3D *obj);				// new key value

	Transform				m_registration;

	DisplayInfo				m_displayInfo;

	Bounds3D				m_localBounds;
	Bounds3DD				m_projectBounds;

	const Object3D			*m_parent;
};
//-------------------------------------------------------------------------
/// Group, a composition of objects
//-------------------------------------------------------------------------
/*! This is meant to provide a structure for objects. Management of child
 *  objects is the resposibility of derived classes.
 *
 *  Ownership and removal of objects is managed through SceneClassManager
 */
//-------------------------------------------------------------------------
class CCLASSES_API Group3D : public Object3D
{
public:
	virtual ~Group3D() {}

	virtual int numObjects() const =0;
	virtual const Object3D	*object(int i) const = 0;
	virtual Object3D	  	*object(int i) = 0;
	
	virtual const wchar_t *typeDescriptor() const	{ return L"_Group3D"; }
	virtual const char *objectClass() const			{ return "Group3D"; }
	virtual const char *className() const			{ return "Group3D"; }

#ifdef HAVE_OPENGL
	virtual void drawGL(uint32 drawmode, int millisecs, const ptgl::Viewport *viewport);
#endif
	
	virtual void projectBoundsTransform(const mmatrix4d &m);
	virtual void resetCoordinateSpace();
	virtual bool moveChildTranslationsToParent( double min_translation, double roundto );

	virtual bool isGroup() const				{ return true; }

	virtual void diagnostic(Output *output, bool recursive) const;

	virtual double findNearestPoint(const pt::vector3d &pnt, pt::vector3d &nearest, CoordinateSpace cs) const;

	virtual int numBranchObjects() const
	{
		int nobj = 0;
		for (int i=0; i<numObjects(); i++)
		{
			const Object3D *obj = object(i);
            if (obj->isGroup()) nobj += static_cast<const Group3D*>(obj)->numBranchObjects();
			else ++nobj;
		}
		return nobj;
	}

	virtual void visibleBounds(Bounds3DD &bounds) const;

protected:
	virtual void _computeBounds();

	Group3D(const wchar_t* id = 0, const Object3D *parent=0) : Object3D(id, parent)
	{ m_registration.coordinateSpace(SceneSpace); };
};
//-------------------------------------------------------------------------
/// Scene a composition of objects of a single type
//-------------------------------------------------------------------------
/*! A scene represents a logical collection of related objects usually of the 
 *  and adjust properties
 *
 *  Ownership and removal of objects is managed through SceneClassManager
 */
//-------------------------------------------------------------------------
class Scene3D : public Group3D
{
public:
	virtual ~Scene3D() {};
	virtual const wchar_t *typeDescriptor() const	{ return L"_Scene3D"; }
	const char *objectClass() const					{ return "Scene3D"; }
	virtual const char *className() const			{ return "Scene3D"; }

	const ptds::FilePath &filepath() const { return m_filepath; }

protected:

	Scene3D(const wchar_t* id = 0, const Object3D *parent=0) : Group3D(id, parent) 
	{ m_registration.coordinateSpace(SceneSpace); };

	ptds::FilePath	m_filepath;

	virtual bool importFile(const ptds::FilePath &path) { return false; };
	virtual bool exportFile(const ptds::FilePath &path)	{ return false; };
};
}
#endif
