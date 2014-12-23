#include <pt/datatreeNode.h>
namespace pt
{
namespace datatree
{

Node * Node::create( Variant v )
{
	return new Node(v);
}

Node * Node::create( const Node &n )
{
	return new Node(n);
}

Node * Node::create()
{
	return new Node;
}
void Node::destroy( Node * n )
{
	delete n;
}
}
}