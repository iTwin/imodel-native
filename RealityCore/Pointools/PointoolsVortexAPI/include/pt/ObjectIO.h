#ifndef POINTOOLS_OBJECT_OI_HELPERS
#define POINTOOLS_OBJECT_OI_HELPERS

#include <pt/variant.h>
#include <pt/scenegraph.h>
#include <pt/datatree.h>

namespace pt
{
struct PropertyWriteVisitor : public InterfaceVisitor
{
	PropertyWriteVisitor(const Object *obj, datatree::Branch *tree)
	{
		_tree = tree->addBranch(Unicode2Ascii::convert(obj->identifier()).c_str());
	};
	PropertyWriteVisitor(datatree::Branch *tree) : _tree(tree) {};
	PropertyWriteVisitor(const char*branch, datatree::Branch *tree)
	{
		_tree = tree->addBranch(branch);
	}
	struct WriteParam
	{
		WriteParam(const char *id, datatree::Branch *tree) : 
			_tree(tree), _id(id) {};

		template <class T> operator ()(const T &v)
		{
			_tree->addNode(_id, v);
		}
		datatree::Branch *_tree;
		const char *_id;
	};
	void info(const pt::Property &p) {}
	void property(const pt::Property &p)
	{
		WriteParam vis(p.id, _tree);
		ttl::var::apply_visitor(vis, p.value);		
	}
	void method(const char* id){}	
	datatree::Branch *_tree;
};
//
// Property Read Visitor
//
struct PropertyReadVisitor : public InterfaceVisitor
{
	PropertyReadVisitor(Object *obj, datatree::Branch *tree)
	{
		_tree = tree->findBranch(Unicode2Ascii::convert(obj->identifier()).c_str());
		_obj = obj;
	};
	PropertyReadVisitor(const char *branch, Interface *obj, datatree::Branch *tree)
	{
		_tree = _tree->findBranch(branch);
		_obj = obj;
	};	
	struct ReadParam
	{
		ReadParam(const char *id, datatree::Branch *tree, Interface *obj) :
			_tree(tree), _id(id), _obj(obj) {};

		template <class T> operator ()(const T &v)
		{
			Variant val(v);
			
			if (_tree->getNodeV(_id, val))
			{
				_obj->setProperty(_id, val);
			}
		}
		datatree::Branch *_tree;
		const char *_id;
		Interface *_obj;
	};
	void info(const pt::Property &p) {}
	void property(const pt::Property &p)
	{
		if (_tree)
		{
			ReadParam vis(p.id, _tree, _obj);
			ttl::var::apply_visitor(vis, p.value);		
		}
		assert(_tree);
	}
	void method(const char* id){}	
	datatree::Branch *_tree;
	Interface *_obj;
};
}
#endif
