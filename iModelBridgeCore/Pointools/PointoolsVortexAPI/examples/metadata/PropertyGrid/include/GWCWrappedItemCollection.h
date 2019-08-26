// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCWRAPPEDITEMCOLLECTION_H
#define __GWCWRAPPEDITEMCOLLECTION_H

#if _MSC_VER < 1300
#include "listfix"	// Fixed std::list
#else
#include <list>
#endif
#include "GWCPropertiesLink.h"

class GWCItemWrapper;
class GWCDeepIterator;
class GWCWrappedItemCollection;
class GWCPropertyItem;
class GWCVisibleDeepIterator;
class GWCInternalGrid;
class GWCSiblingIterator;
class GWCVisibleSiblingIterator;

typedef std::list<GWCItemWrapper*> SiblingItemCollection;

class GWCPROPCTRL_LINKAGE GWCWrappedItemCollection  
{
	// Types
	public:
		typedef SiblingItemCollection::iterator iterator;
		typedef GWCDeepIterator deepIterator;
		typedef GWCVisibleDeepIterator visibleDeepIterator;

	// Construction
	public:
		GWCWrappedItemCollection();
		virtual ~GWCWrappedItemCollection();

	// Attributes
	protected:
		SiblingItemCollection m_wItemCollection;	// The collection maintained by this collection
		iterator m_iterParentWItem;	// An iterator to the parent item wrapper (in the collection of the parent collection)
		bool m_rootItem;

		GWCVisibleDeepIterator* m_vend;
		GWCDeepIterator* m_dend;
		SiblingItemCollection::iterator m_end;

	// Operations
	public:
		void SetParentIterWItem(GWCWrappedItemCollection::iterator& iterParentWItem);
		GWCWrappedItemCollection::iterator GetParentIterWItem();

		GWCWrappedItemCollection* GetParentWItemCollection();
		GWCPropertyItem* GetParentProperty();
		GWCItemWrapper* GetParentWItem();

		iterator begin();
		iterator end();
		GWCItemWrapper* operator[](int index);

		UINT size();
		bool empty();

		void clear();

		deepIterator dbegin(bool restrictToThisLevel = false);
		deepIterator& dend();

		visibleDeepIterator vbegin();
		visibleDeepIterator& vend();

		GWCSiblingIterator sbegin();
		GWCSiblingIterator send();
		GWCVisibleSiblingIterator svbegin();
		GWCVisibleSiblingIterator svend();

		deepIterator find(int id);

		GWCWrappedItemCollection::deepIterator Insert(iterator& beforeSiblingIter, GWCPropertyItem* pItem);
		GWCDeepIterator Delete(GWCDeepIterator& iter);
		void sort();

	protected:
		GWCWrappedItemCollection::deepIterator GetDeepIterator(iterator iter);
};

class GWCPROPCTRL_LINKAGE GWCDeepIterator
{
	// Construction
	public:
		GWCDeepIterator();
		GWCDeepIterator(GWCWrappedItemCollection::iterator& iterWItem, GWCWrappedItemCollection* pWItemCollection = NULL, bool restrictToThisLevel = false);
		GWCDeepIterator(GWCSiblingIterator& iter);
		virtual ~GWCDeepIterator();

	// Attributes
	protected:
		GWCWrappedItemCollection::iterator m_iterWItem;
		GWCWrappedItemCollection* m_pWItemCollection;
		GWCWrappedItemCollection* m_restrictAtAndUnderCollection;

	// Operations
	public:
		bool HasParent();
		GWCDeepIterator& operator++();
		GWCDeepIterator& operator++(int nb);
		GWCDeepIterator& operator--();
		GWCDeepIterator& operator--(int nb);
		GWCDeepIterator operator+(int inc);
		GWCDeepIterator operator-(int dec);
		GWCPropertyItem* operator*();
		bool operator==(GWCDeepIterator& iter);
		bool operator!=(GWCDeepIterator& iter);
		GWCWrappedItemCollection* GetCollection();
		bool operator <(GWCDeepIterator& iter);
		int GetDepth();
		GWCDeepIterator GetParentIterItem();
		GWCWrappedItemCollection* GetChildren();

	protected:
		GWCWrappedItemCollection::iterator GetIterator();
		GWCItemWrapper* GetItemWrapper();
		GWCDeepIterator& operator>>(int);

	friend GWCWrappedItemCollection;
	friend GWCInternalGrid;
	friend GWCVisibleDeepIterator;
	friend GWCSiblingIterator;
};

class GWCPROPCTRL_LINKAGE GWCSiblingIterator
{
	// Construction
	public:
		GWCSiblingIterator();
		GWCSiblingIterator(GWCDeepIterator iter);
		virtual ~GWCSiblingIterator();

	// Attributes
	protected:
		GWCWrappedItemCollection::iterator m_iterWItem;
		GWCWrappedItemCollection* m_pWItemCollection;

	// Operations
	public:
		bool HasParent();
		GWCSiblingIterator& operator++();
		GWCSiblingIterator& operator++(int nb);
		GWCSiblingIterator& operator--();
		GWCSiblingIterator& operator--(int nb);
		GWCSiblingIterator operator+(int inc);
		GWCSiblingIterator operator-(int dec);
		GWCPropertyItem* operator*();
		bool operator==(GWCSiblingIterator& iter);
		bool operator!=(GWCSiblingIterator& iter);
		GWCWrappedItemCollection* GetCollection();
		GWCDeepIterator GetParentIterItem();
		GWCWrappedItemCollection* GetChildren();

	protected:
		GWCItemWrapper* GetItemWrapper();

	friend GWCDeepIterator;
	friend GWCVisibleSiblingIterator;
};


class GWCPROPCTRL_LINKAGE GWCVisibleDeepIterator : public GWCDeepIterator
{
	// Construction
	public:
		GWCVisibleDeepIterator();
		GWCVisibleDeepIterator(GWCWrappedItemCollection::iterator iter, GWCWrappedItemCollection* pWItemCollection);
		GWCVisibleDeepIterator(GWCDeepIterator& iter);
		virtual ~GWCVisibleDeepIterator();

	// Operations
	public:
		GWCVisibleDeepIterator& operator=(GWCDeepIterator& iter);
		GWCVisibleDeepIterator operator+(int inc);
		GWCVisibleDeepIterator operator-(int dec);
		GWCVisibleDeepIterator& operator++();
		GWCVisibleDeepIterator& operator++(int nb);
		GWCVisibleDeepIterator& operator--();
		GWCVisibleDeepIterator& operator--(int nb);

	friend GWCVisibleSiblingIterator;
};

class GWCPROPCTRL_LINKAGE GWCVisibleSiblingIterator : public GWCSiblingIterator
{
	// Construction
	public:
		GWCVisibleSiblingIterator();
		GWCVisibleSiblingIterator(GWCVisibleDeepIterator iter);
		virtual ~GWCVisibleSiblingIterator();

	// Operations
	public:
		GWCVisibleSiblingIterator& operator=(GWCSiblingIterator& iter);
		GWCVisibleSiblingIterator operator+(int inc);
		GWCVisibleSiblingIterator operator-(int dec);
		GWCVisibleSiblingIterator& operator++();
		GWCVisibleSiblingIterator& operator++(int nb);
		GWCVisibleSiblingIterator& operator--();
		GWCVisibleSiblingIterator& operator--(int nb);
};

#endif // __GWCWRAPPEDITEMCOLLECTION_H
