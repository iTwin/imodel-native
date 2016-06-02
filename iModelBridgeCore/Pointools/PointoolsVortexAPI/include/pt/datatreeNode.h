#pragma once

#include <pt/ptmath.h>
#include <pt/variant.h>

#define NODE_ID_LENGTH		32
#define NODE_ID_SIZE		(NODE_ID_LENGTH * sizeof(NODEID_CHARTYPE))
#define NODEID_CHARTYPE		char

namespace pt
{
namespace datatree
{

//! NodeID
//! Fixed size, single byte char strings
class NodeID
{
public:
	NodeID(const char* id)
												{
													assert(id);
													assert(strlen(id)>0);
													assert(strlen(id)<NODE_ID_LENGTH);

													(*this) = id;
												}
											
	NodeID(const char *id, int index)			// append an index to the id, ie "viewport1"
												{			
                                                BeStringUtilities::Snprintf(_id, NODE_ID_LENGTH, "%s%i", id, index);
												}
	NodeID()									{ clear(); }

	bool operator !  () const					{ return isEmpty(); }
	bool operator == (const NodeID &b) const	{ return memcmp(_id, b._id, sizeof(_id)) == 0;	}
	bool operator <	 (const NodeID &b) const	{ return memcmp(_id, b._id, sizeof(_id)) < 0;	}
	bool operator >	 (const NodeID &b) const	{ return memcmp(_id, b._id, sizeof(_id)) > 0;	}
	bool operator >= (const NodeID &b) const	{ return memcmp(_id, b._id, sizeof(_id)) >= 0;	}
	bool operator <= (const NodeID &b) const	{ return memcmp(_id, b._id, sizeof(_id)) <= 0;	}

	void operator = (const NODEID_CHARTYPE *id)	
												{
													clear();
													assert(strlen(id) <= sizeof(_id));
													memcpy(_id, id, strlen(id));
												}
	void operator = (const NodeID &id)
												{
													clear();
													memcpy(_id, id._id, sizeof(_id));			
												}
	void clear()								{ memset(_id, 0, sizeof(_id)); }

	bool isEmpty() const						{ return _id[0] == 0 ? true : false; }

	void get(NODEID_CHARTYPE *id) const
												{
													id[NODE_ID_LENGTH-1] = '\0';
													memcpy(id, _id, sizeof(_id));
												}
	const NODEID_CHARTYPE *get() const			{ return _id; }

private:

	char	_id[NODE_ID_LENGTH];
};	
//! Node
//! Single tagged values
struct Node
{
	static Node *create( Variant v );
	static Node *create( const Node &n );
	static Node *create();

	template <class T> 
	static Node *create(const T &v )			{ return create( Variant(v) ); }

	static void	destroy( Node * );

	template <class Visitor>
	void visitType(Visitor &V) const
	{
		ttl::var::apply_visitor(V, _v);
	}

	template<class T>
	bool getValue(T &v) const
	{
		if (Variant(v).which() == _v.which() )
		{
			v = ttl::var::get<T>(_v);
			return true;
		}
		return false;
	}

	template<class T>
	bool setValue(T &v)
	{
		if (Variant(v).which() == _v.which() )
		{
			_v = Variant(v);
			return true;
		}
		return false;
	}

	const Variant& var() const
	{
		return _v;
	}

	int		typeId() const	{ return _v.which(); }


private:

	Node() : _v(0)			{}
	Node(const Node &n)		{ _v = n._v; }
	Node(const Variant &v)	{ _v = v; }

	Variant					_v;
};

}	// namespace datatree
}	// namespace pt