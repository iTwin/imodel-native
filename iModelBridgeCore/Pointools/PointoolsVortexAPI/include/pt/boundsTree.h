/*--------------------------------------------------------------------------*/ 
/*  Hierarchical Bounds Tree                                                 */ 
/*  (C) 2011 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*--------------------------------------------------------------------------*/ 
#pragma once

#include <vector>
#include <pt/typedefs.h>
#include <pt/OBBox.h>
#include <pt/flags.h>
#include <math/matrix_math.h>
#include <queue>
#include <vortexobjects/IClashNode.h>


namespace pt
{
template <class B>
class BoundsTreeNode : public vortex::IClashNode
{
public:
	enum Flags
	{
		Null=0,
		LeftNode=1,			// is a left node
		RightNode=2,		// is a right node
		NodeInterferance=4,	// interface test positive flag
		FromLeaf=8,			// extract tree created this node from a leaf 
		HasGeometry=16,		// Superclass contains geometry
		NodeDifference=32,	// difference positive flag
		Complete=64			// is a final node - ie leaf will branch further 
	};

	BoundsTreeNode(const B &b, int depth=0, ubyte flags=0, uint elementCount=0) 
		: m_bounds(b), m_left(0), m_right(0), m_depth(depth), m_flags(flags),
		m_userFlags(0), m_elementCount(elementCount)
	{}

	~BoundsTreeNode()	// avoid virtual destructor to prevent vtable ptr for mem efficiency
	{
		if (m_left) delete m_left;
		if (m_right) delete m_right;
	}

	const BoundsTreeNode<B> * left() const 	{ return m_left; }
	const BoundsTreeNode<B> * right() const { return m_right; }

	BoundsTreeNode<B> * left() 				{ return m_left; }
	BoundsTreeNode<B> * right()  			{ return m_right; }

	// Generic IO, must be used carefully, does not provide
	// recursion to enable extension of node data
	template <class Writer>
	void writeNode( Writer &w, mmatrix4d &transform)
	{
		OBBox<float>	floatBounds(m_bounds);
		vector3			floatCenterLocalized;

		w.write( (int)m_depth );
		w.write( m_flags );
		w.write( floatBounds.extents() );
		w.write( floatBounds.axis(0) );
		w.write( floatBounds.axis(1) );
		w.write( floatBounds.axis(2) );

		vector3d center = m_bounds.center();
		vector3d centerLocalized;
		transform.vec3_multiply_mat4(center, centerLocalized);
		floatCenterLocalized.set(static_cast<float>(centerLocalized.x), static_cast<float>(centerLocalized.y), static_cast<float>(centerLocalized.z));

		floatBounds.center(floatCenterLocalized);

		w.write( floatBounds.center() );
		w.write( m_elementCount );
	}
	template <class Reader>
	void readNode( Reader &r )
	{
		OBBox<float>	floatBounds;

		int depth;
		ubyte flags = 0;
		pt::vector3 v;
		
		r.read( depth );
		m_depth = depth;

		r.read( flags );
		m_flags = flags;

		r.read( v );
		floatBounds.extents( v );

		r.read( v );
		floatBounds.axis( 0, v );

		r.read( v );
		floatBounds.axis( 1, v );

		r.read( v );
		floatBounds.axis( 2, v );

		r.read( v );
		floatBounds.center( v );

		m_bounds = floatBounds;

		r.read( m_elementCount );
	}
	// returns flags
	template <class Reader>
	int readAndAddChildNode( Reader &r, OBBoxd &bounds )
	{
		OBBox<float>	floatBounds;

		int d=-2;
		ubyte flags = 0;
		uint elementCount = 0;

		pt::vector3 v;
		r.read( d );

		if (d == m_depth + 1)
		{
			r.read( flags );
			r.read( v );
			floatBounds.extents( v );

			r.read( v );
			floatBounds.axis( 0, v );

			r.read( v );
			floatBounds.axis( 1, v );

			r.read( v );
			floatBounds.axis( 2, v );

			r.read( v );
			floatBounds.center( v );
			
			r.read( elementCount );

			bounds = floatBounds;
			
			if (flags & LeftNode)
			{
				insertLeft(bounds, flags, elementCount );
			}
			else if (flags & RightNode)
			{
				insertRight(bounds, flags, elementCount );
			}
			return flags;
		}		
		return -1;
	}

	int findMaxLeafDepth( int _max=0 ) const
	{
		if (isLeaf())
		{
			if (_max < m_depth)
				return m_depth;
		}
		else 
		{
			if (m_left) 
			{
				int d = m_left->findMaxLeafDepth(_max);
				if (d>_max) _max = d;
			}
			if (m_right) 
			{
				int d = m_right->findMaxLeafDepth(_max);
				if (d>_max) _max = d;
			}
		}
		return _max;
	}

	int findMinLeafDepth( int _min=0 ) const
	{
		if (isLeaf())
		{
			if (_min > m_depth)
				return m_depth;
		}
		else 
		{
			if (m_left) 
			{
				int d = m_left->findMinLeafDepth(_min);
				if (d<_min) _min = d;
			}
			if (m_right) 
			{
				int d = m_right->findMinLeafDepth(_min);
				if (d<_min) _min = d;
			}
		}
		return _min;
	}

	void clearFlags(bool recursive) 
	{
		m_flags &= ~NodeInterferance;
		m_userFlags = 0;

		if (recursive)
		{
			if (m_left) m_left->clearFlags(true);
			if (m_right) m_right->clearFlags(true);
		}
	}
	const ubyte &depth() const
	{	
		return m_depth;
	}
	const ubyte &flags() const 
	{ 
		return m_flags; 
	}
	const unsigned short &userFlags() const
	{
		return m_userFlags;
	}
	void setUserFlag( unsigned short flag, bool on=true, bool recursive=false)
	{
		if (on) m_userFlags |= flag;
		else m_userFlags &= ~flag;

		if (recursive)
		{
			if (m_left)		m_left->setUserFlag(flag,on,recursive);
			if (m_right)	m_right->setUserFlag(flag,on,recursive);
		}
	}
	void setUserFlagsAsNumber( unsigned short num, bool recursive=false)
	{
		m_userFlags = num;
		if (recursive)
		{
			if (m_left) m_left->setUserFlagsAsNumber(num,recursive);
			if (m_right) m_right->setUserFlagsAsNumber(num,recursive);
		}
	}
	unsigned short getUserFlagsAsNumber() const
	{
		return m_userFlags;
	}

	void  setFlag( Flags flag, bool on=true, bool recursive=false )
	{
		if (on) m_flags |= (ubyte)flag;
		else m_flags &= ~(ubyte)flag;

		if (recursive)
		{
			if (m_left) m_left->setFlag( flag, on, recursive );
			if (m_right) m_right->setFlag( flag, on, recursive );
		}
	}
	
	void setElementCount( uint count )
	{
		m_elementCount = count;
	}

	uint getElementCount( void ) const
	{
		return m_elementCount;
	}

	void insertLeft( const B &b, ubyte flags=LeftNode, uint elementCount=0 )
	{
		if (m_left) removeLeft();
		m_left=new BoundsTreeNode<B>(b,m_depth+1, flags, elementCount );
	}

	void insertRight( const B &b, ubyte flags=RightNode, uint elementCount=0  )
	{
		if (m_right) removeRight();
		m_right=new BoundsTreeNode<B>(b,m_depth+1, flags, elementCount );
	}

	void removeLeft()
	{
		if (m_left) 
		{
			delete m_left;
			m_left = 0;
		}
	}
	void removeRight()
	{
		if (m_right) 
		{
			delete m_right;
			m_right = 0;
		}
	}

	void replaceLeft( BoundsTreeNode<B> *replacement )
	{
		if (m_left)
		{
			replacement->m_left = m_left->left();
			replacement->m_right = m_left->right();

			// detach to prevent branch deletion			
			m_left->m_left = 0;
			m_left->m_right = 0;

			delete m_left;
		}
		m_left = replacement;
	}

	void replaceRight( BoundsTreeNode<B> *replacement )
	{
		if (m_right)
		{
			replacement->m_left = m_right->left();
			replacement->m_right = m_right->right();

			// detach to prevent branch deletion
			m_right->m_left = 0;
			m_right->m_right = 0;

			delete m_right;
		}
		m_right = replacement;
	}

	// split node, does not split bounds
	void split()
	{
		m_left = new BoundsTreeNode<B>(m_bounds, m_depth+1, LeftNode);
		m_right = new BoundsTreeNode<B>(m_bounds, m_depth+1, RightNode);
	}
	
	struct IntersectionTest
	{
		bool test( const BoundsTreeNode<B> *nodeA, const BoundsTreeNode<B> *nodeB ) const
		{
			return nodeA->intersects(nodeB);
		}
	};

	template<class Test>
	int		recursiveTest( Test &t, const BoundsTreeNode<B> *node, uint addFlag=0, uint remFlag=0 ) const
	{
		// comparisons
		bool pass=false;

		if (m_left)
		{
			if (node->left() && t.test( left(), node->left() ))
			{
				left()->recursiveTest( node->left() );
				pass=true;
			}
			if (node->right() && t.test( left(), node->right() ))
			{
				left()->recursiveTest( node->right() );
				pass=true;
			}
		}
		if (m_right)
		{
			if (node->left() && t.test( right(), node->left() ))
			{
				right()->recursiveTest( node->left() );
				pass=true;
			}
			if (node->right() && t.test( right(), node->right() ))
			{
				right()->recursiveTest( node->right() );
				pass=true;
			}
		}
		if (isLeaf() && t.test( this, node ))
		{
			if (!node->isLeaf())
			{
				if (node->left()) recursiveTest(node->left());
				if (node->right()) recursiveTest(node->right());
			}
			pass=true;
		}	
		if (pass)
		{
			const_cast<BoundsTreeNode<B> *>(this)->m_flags |= addFlag;
			const_cast<BoundsTreeNode<B> *>(this)->m_flags &= ~remFlag;
		}
		
		return pass;		
	}

	// recursive intersection test
	bool 	computeIntersection( const BoundsTreeNode<B> *node ) const
	{
		return recursiveTest( IntersectionTest(), node, NodeInterferance );

		// comparisons
		bool doesIntersect=false;
		if (m_left)
		{
			if (node->left() && left()->intersects( node->left() ))
			{
				left()->computeIntersection( node->left() );
				doesIntersect=true;
			}
			if (node->right() && left()->intersects( node->right() ))
			{
				left()->computeIntersection( node->right() );
				doesIntersect=true;
			}
		}
		if (m_right)
		{
			if (node->left() && right()->intersects( node->left() ))
			{
				right()->computeIntersection( node->left() );
				doesIntersect=true;
			}
			if (node->right() && right()->intersects( node->right() ))
			{
				right()->computeIntersection( node->right() );
				doesIntersect=true;
			}
		}
		if (isLeaf() && intersects(node))
		{
			if (!node->isLeaf())
			{
				if (node->left()) computeIntersection(node->left());
				if (node->right()) computeIntersection(node->right());
			}
			doesIntersect=true;
		}	
		if (doesIntersect)
			const_cast<BoundsTreeNode<B> *>(this)->m_flags |= NodeInterferance;
		
		return doesIntersect;
	}

	bool	isLeaf() const 		
	{ 	
		return !m_left && !m_right ? true : false; 
	}

	bool	isLeft() const
	{
		return m_flags & LeftNode ? true : false;
	}

	template <class Geom>
	bool	intersectsLeaf( const Geom &b ) const 
	{
		if (m_bounds.intersects(b))
		{
			if (isLeaf()) return true;
			if (m_left && m_left->intersectsLeaf(b)) return true;
			if (m_right && m_right->intersectsLeaf(b)) return true;
		}
		return false;
	}

	template <class PointType>
	bool	containedInLeaf( const PointType &p ) const
	{
		if (m_bounds.contains(p))
		{
			if (m_left && m_left->containedInLeaf(p)) 
				return true;
			if (m_right && m_right->containedInLeaf(p)) 
				return true;
			if (isLeaf()) return true;
		}
		return false;
	}

	template <class PointType>
	const BoundsTreeNode<B> *findContainingLeaf( const PointType &p ) const
	{
		if (m_bounds.contains(p))
		{
			if (isLeaf()) return this;

			if (m_left)
			{
				const BoundsTreeNode<B>* node = m_left->findContainingLeaf(p);
				if (node) return node;
			}
			if (m_right)
			{
				const BoundsTreeNode<B>* node = m_right->findContainingLeaf(p);
				if (node) return node;
			}
		}
		return 0;
	}
	
	bool 	intersects( const BoundsTreeNode *node ) const
	{
		return node ? node->m_bounds.intersects( m_bounds ) : false;
	}

	bool 	intersects( const B &b ) const
	{
		return b.intersects( m_bounds );
	}

	bool 	intersectsLeaf( const B &b ) const
	{
		if (isLeaf())
			return (b.intersects( m_bounds ));
		else 
		{
			return (m_left ? m_left->intersectsLeaf(b) : false)
				|| (m_right ? m_right->intersectsLeaf(b) : false);
		}
	}

	void	collectNodes( std::vector<B> &nodes, int depth ) const
	{
		if (m_depth==depth)
		{
			nodes.push_back(bounds());
		}
		else
		{
			if (m_left)		m_left->collectNodes( nodes, depth );
			if (m_right)	m_right->collectNodes( nodes, depth );
		}
	}
#ifdef NEEDS_WORK_VORTEX_DGNDB
	void	collectNodes( std::vector<B> &nodes, ubyte flags) const
	{
		if (m_flags & flags)
		{
			nodes.push_back(bounds());
		}
		else
		{
			++_d;
			if (m_left)		m_left->collectNodes( nodes, flags );
			if (m_right)	m_right->collectNodes( nodes, flags );
		}
	}
#endif

	void	collectLeaves( std::vector<B> &leaves, ubyte flags=0xff ) const
	{
		if (isLeaf() && m_flags & flags) 
			leaves.push_back(m_bounds);
		else
		{
			if (m_left) m_left->collectLeaves(leaves);
			if (m_right) m_right->collectLeaves(leaves);
		}
	}
	
	struct NullCriteria
	{
		static bool test( const BoundsTreeNode *b ) { return true; }
	};

	void	collectLeaves( std::set< const BoundsTreeNode<B>* > &leaves, ubyte flags=0xff ) const
	{
		if (isLeaf() && m_flags & flags) 
			leaves.insert(this);
		else
		{
			if (m_left) m_left->collectLeaves(leaves, flags);
			if (m_right) m_right->collectLeaves(leaves, flags);
		}
	}

	void _countLeaves( int &res ) const
	{
		if (isLeaf())
			res++;
		else
		{
			if (m_left) m_left->_countLeaves(res);
			if (m_right) m_right->_countLeaves(res);
		}
	}

	void _countElements( int &res ) const
	{
		if (isLeaf()) 
			res += getElementCount();			
		else
		{
			if (m_left)		m_left->_countElements(res);
			if (m_right)	m_right->_countElements(res);
		}
	}

	int countLeaves( void ) const
	{
		int res = 0;
		_countLeaves(res);
		return res;
	}

	int countElements( void ) const
	{
		int res = 0;
		_countElements(res);
		return res;
	}

	void	collectIntersectingLeaves( std::set< const BoundsTreeNode<B>* > &leaves, const B &b, ubyte flags=0xff ) const
	{
		if (isLeaf() && m_flags & flags && intersects(b)) 
			leaves.insert(this);
		else
		{
			if (m_left && m_left->intersects(b)) 
				m_left->collectLeaves(leaves, flags);
			if (m_right && m_right->intersects(b)) 
				m_right->collectLeaves(leaves, flags);
		}
	}
	// not_flags are only evaluated on the leaf node
	BoundsTreeNode<B> *extractTree( ubyte flags, ubyte not_flags_at_leaf=0, 
				BoundsTreeNode<B> *parent=0, bool mustHaveFlagAtLeaves=true) const
	{
		bool isRoot = !parent ? true : false;

		if (m_flags & flags || flags==0)
		{
			if (!parent)
			{
				parent = new BoundsTreeNode<B>(m_bounds, m_depth);
			}
			else
			{
				BoundsTreeNode<B> *n=new BoundsTreeNode<B>(m_bounds, m_depth, m_flags, m_elementCount);
				
				parent = (m_flags & LeftNode) ? 
					parent->m_left = n : parent->m_right = n;

				if (isLeaf() && !(m_flags & not_flags_at_leaf)) 
					n->setFlag(FromLeaf);	// or else it will be culled
			}
			if (m_left)		m_left->extractTree( flags, not_flags_at_leaf, parent );
			if (m_right)	m_right->extractTree( flags, not_flags_at_leaf, parent );

			// cull false leaves that were created from non-leaves
			if ((mustHaveFlagAtLeaves || not_flags_at_leaf) && isRoot)
			{
				int cullCount = 0;
				do 
				{
					cullCount = parent->cullFalseLeaves();
				} 
				while(cullCount);	
			}
			return parent;
		}
		return 0;
	}


	const B	&bounds() const 
	{ 
		return m_bounds; 
	}

	void setBounds( const B &b )
	{
		m_bounds = b;
	}

	void transform( const mmatrix4d &xform )
	{
		m_bounds.transform(xform);
		if (m_left) m_left->transform(xform);
		if (m_right) m_right->transform(xform);
	}	

	template<class Visitor>
	void visit(Visitor &v, bool includeEmpty=false, BoundsTreeNode<B> *parent=0)
	{
		if (v.node(this, parent))
		{
			if (m_left)		m_left->visit(v, includeEmpty, this);
			else if (includeEmpty) v.node(0, this);

			if (m_right)	m_right->visit(v, includeEmpty, this);
			else if (includeEmpty) v.node(0, this);
		}
	}

	uint collateChildFlags( uint flags )
	{
		if (m_left)		m_flags |= m_left->collateChildFlags( flags );
		if (m_right)	m_flags |= m_right->collateChildFlags( flags );

		return (flags & m_flags);
	}

	//--------------------------------------------------------------------
	// IClashNode
	//--------------------------------------------------------------------		
	virtual vortex::IClashNode*	_getLeft()
	{
		return left();
	}

	virtual vortex::IClashNode*	_getRight()
	{
		return right();
	}

	virtual PTbool				_isLeaf()
	{
		return (!left() && !right());
	}

	virtual PTvoid				_getBounds(PTfloat* extents3, PTdouble* center3, PTfloat* xAxis3, PTfloat* yAxis3, PTfloat* zAxis3)
	{		
		extents3[0] = static_cast<float>(m_bounds.extents().x);
		extents3[1] = static_cast<float>(m_bounds.extents().y);
		extents3[2] = static_cast<float>(m_bounds.extents().z);
		
		center3[0] = m_bounds.center().x;
		center3[1] = m_bounds.center().y;
		center3[2] = m_bounds.center().z;
		
		xAxis3[0] = static_cast<float>(m_bounds.axis(0).x);
		xAxis3[1] = static_cast<float>(m_bounds.axis(0).y);
		xAxis3[2] = static_cast<float>(m_bounds.axis(0).z);

		yAxis3[0] = static_cast<float>(m_bounds.axis(1).x);
		yAxis3[1] = static_cast<float>(m_bounds.axis(1).y);
		yAxis3[2] = static_cast<float>(m_bounds.axis(1).z);

		zAxis3[0] = static_cast<float>(m_bounds.axis(2).x);
		zAxis3[1] = static_cast<float>(m_bounds.axis(2).y);
		zAxis3[2] = static_cast<float>(m_bounds.axis(2).z);
	}


private:

	int cullFalseLeaves()
	{
		int count = 0;
		// was this node created from a leaf, ie. should it be final (keep)
		// or is it final because the extraction stopped on a branch (remove)
		if (m_left && m_left->isLeaf() && !(m_left->flags() & FromLeaf))	
		{
			removeLeft();
			++count;
		}
		if (m_right && m_right->isLeaf() && !(m_right->flags() & FromLeaf))	
		{
			++count;
			removeRight();
		}
		if (m_left) count += m_left->cullFalseLeaves();
		if (m_right) count += m_right->cullFalseLeaves();
		
		return count;
	}

	B						m_bounds;
	BoundsTreeNode<B> 		*m_left;
	BoundsTreeNode<B>		*m_right;
	ubyte					m_depth;
	ubyte					m_flags;
	unsigned short			m_userFlags;
	uint					m_elementCount;
};

template <class B>
class BoundsTree
{
	public:	
		typedef BoundsTreeNode<B> Node;

		BoundsTree<B>(Node *root=0): m_root(root)
		{
		}
		BoundsTree<B>(const B &b) 
		{
			setRootBounds(b);
		}

		Node		*root() 		{ return m_root; }
		const Node	*root()  const 	{ return m_root; }
	
		struct CompareSet
		{
			CompareSet() 
			{ a[0] = a[1] = b[0] = b[1] = 0; };

			CompareSet( Node *a0, Node *a1, Node *b0, Node *b1) 
			{ a[0] = a0; a[1] = a1; b[0] = b0; b[1] = b1; };

			void clear()				{ a[0]=0;a[1]=0;b[0]=0;b[1]=0; }
			bool empty() const			{ a[0]==0 || b[0]==0 ? true : false; }

			Node *a[2];
			Node *b[2];
		};
/*
// WIP
		struct ClosestBoxesComparision
		{
			bool	closest[4];
			float	maxDist;
			float	minDist;
			int		iterations;

			int compare( pt::OBBoxf boxesA[2], pt::OBBoxf boxesB[2], 
									  bool closest[4], float maxDist, float minDist, 
									  int iterations )
			{
				int c = 0;
				bool valid[4], last_valid[4];

				OBBoxd boxA, boxB;

				for (int it=0; it<iterations; it++)
				{
					if (maxDist < minDist) break;

					bool hasIntersection = false;
					
					int numValid = 0;
					
					for (int v=0;v<4;v++)
						closest[v] = it ? valid[v] : false;

					for (int i=0; i<2; i++)
					{
						for (int j=0; j<2; j++)
						{
							valid[c] = false;

							// expand boxes
							boxA = boxesA[i];
							boxB = boxesB[j];
							boxA.expand(maxDist);
							boxB.expand(maxDist);

							// compute the closest pair of boxes, exclude intersection
							if (boxA.intersects(boxB))
							{
								valid[c] = true;
								++numValid;
							}
							++c;
						}
					}
					if (!numValid)
					{
						// the last valid results	
						break;
					}
					if (numValid == 1)
					{
						for (int v=0;v<4;v++)
							closest[v] = valid[v];

						break;	// current valid is the closest
					}
					maxDist *= 0.5;
				}
				//put the final pairs in the queue
				if (closest[0]) comps.push_back( a[0], 
			}
		}*/
		struct IntersectionFunc
		{			
			static bool compare( Node** a, Node** b, std::queue<CompareSet> &comps)
			{
				uint refine = 0;
				bool ins = false;

				for (int i=0; i<2; i++)
				{
					for (int j=0; j<2; j++)
					{
						if (!a[i] || !b[j]) continue;

						if (a[i]->intersects(b[j]))
						{
							if (a[i]->isLeaf())
							{
								if (b[j]->isLeaf())
								{
									a[i]->setFlag(Node::NodeInterferance);
									b[j]->setFlag(Node::NodeInterferance);					
								}
								else
								{
									comps.push( CompareSet(a[i],0, b[j]->left(), b[j]->right() ));
									ins = true;
									b[j]->setFlag(Node::NodeInterferance);					
								}
							}
							else
							{
								if (b[j]->isLeaf())
								{
									comps.push( CompareSet(a[i]->left(),a[i]->right(), b[j], 0));	
									a[i]->setFlag(Node::NodeInterferance);
								}
								else
								{
									comps.push( CompareSet(a[i]->left(),a[i]->right(), b[j]->left(), b[j]->right() ));
									a[i]->setFlag(Node::NodeInterferance);
									b[j]->setFlag(Node::NodeInterferance);
								}
								ins = true;
							}
						}
					}
				}
				return ins;
			}
		};


		struct DifferenceFunc
		{			
			static bool compare( Node** a, Node** b, std::queue<CompareSet> &comps)
			{
				uint refine = 0;
				bool res = false;

				for (int j=0; j<2; j++)
				{
					bool no_ins = false;

					for (int i=0; i<2; i++)
					{
						if (!a[i] || !b[j]) continue;

						if (!b[j]->intersects(a[i]))
						{
							if (i==1 && no_ins) 
							{
								b[j]->setFlag( Node::NodeInterferance, true, true );
								res = true;
							}
							no_ins = true;
						}
						else
						{
							if (a[i]->isLeaf())
							{
								if (!b[j]->isLeaf())
								{
									comps.push( CompareSet(a[i],0, b[j]->left(), b[j]->right() ));
								}
							}
							else
							{
								if (b[j]->isLeaf())
								{
									comps.push( CompareSet(a[i]->left(),a[i]->right(), b[j], 0));	
								}
								else
								{
									comps.push( CompareSet(a[i]->left(),a[i]->right(), b[j]->left(), b[j]->right() ));
								}
							}
						}
					}
				}
				return res;
			}
		};

		template<class InterferanceFunc>
		BoundsTree<B> *computeInterferance( BoundsTree *bt, InterferanceFunc &funcInstance, bool preserveExistingResults ) const
		{
			if (!preserveExistingResults)
			{
				bt->root()->clearFlags(true);
				const_cast<Node*>(root())->clearFlags(true);
			}	

			std::queue<CompareSet> comps;
			comps.push(CompareSet(const_cast<Node*>(root()), 0, bt->root(), 0));

			do
			{
				CompareSet c = comps.front();
				comps.pop();

				funcInstance.compare(c.a, c.b, comps);
			}
			while (comps.size());
			
			Node *root=0;
			
			bt->root()->collateChildFlags( Node::NodeInterferance );

/*			if (bt->root()->left() && Node::NodeInterferance
				|| bt->root()->right() && Node::NodeInterferance)
			{
				bt->root()->setFlag( Node::NodeInterferance, true );
			}*/	
			root = bt->root()->extractTree( Node::NodeInterferance );

			return root ? new BoundsTree<B>(root) : 0;
		}

		BoundsTree<B> *computeInterferance( BoundsTree *bt, bool preserveExistingResults ) const
		{
			IntersectionFunc f;
			return computeInterferance( bt, f, preserveExistingResults );						
		}

		BoundsTree<B> *computeDifference( BoundsTree *bt, bool preserveExistingResults ) const
		{
			DifferenceFunc f;
			BoundsTree<B> *tree = computeInterferance( bt, f, preserveExistingResults );
			
			return tree;
		}

		int computeInterferance( BoundsTree *bt, std::vector<B> &boxes, bool preserveExistingResults ) const			
		{
			if (!preserveExistingResults)
				bt->root()->clearFlags(true);

			bt->root()->left()->compare( root() );
			bt->root()->right()->compare( root() );
		
			std::set< const Node* > results;
			bt->root()->collectLeaves( results, Node::NodeInterferance );
			
			typename std::set< const Node* >::const_iterator i = results.begin();

			while (i!=results.end())
			{
				boxes.push_back( (*i)->bounds() );
				++i;
			}
			return boxes.size();
		}

		
#ifdef NEEDS_WORK_VORTEX_DGNDB
		template <class GeometryObject, class GeometryCompare>
		BoundsTree<B> *computeIntersectionGeneric( GeometryObject &obj )
		{
			root()->clearFlags();
					
			std::stack< Node *> compare_stack;
			compare_stack.push(root());

			while (compare_stack.size())
			{
				Node *n = compare_stack.top();
				compare_stack.pop();

				int test = GeometryCompare::compare(obj, n->bounds());
				// 1 for contained, 2 for intersection
				if (test==1)
				{
					n->setFlag( Node::NodeInterferance, true );
				}
				else if (test==2)
				{
					n->setFlag( Node::NodeInterferance );
					
					if (n->left())
					{
						compare_stack.push(n->left());
					}
					if (n->right())
					{
						compare_stack.push(n->right);
					}
				}
			}
			Node *rt=0;
			bt->root()->setFlag( Node::NodeInterferance );

			rt = root()->extractTree( Node::NodeInterferance );
			
			return rt ? new BoundsTree<B>(rt) : 0;
		}
#endif

	
	protected:
		void setRootBounds( const B &bounds )	
		{
			if (!m_root)
			{
				m_root = new Node(bounds);
			}	
		}
	private:
		Node 			*m_root;
};

//typedef BoundsTree<OBBoxf> OrientedBoxBoundsTreef;
typedef BoundsTree<OBBoxd> OrientedBoxBoundsTreed;
}