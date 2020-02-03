// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCPROPERTIESFACTORYBASE_H
#define __GWCPROPERTIESFACTORYBASE_H

#include "GWCPropertiesLink.h"

class GWCPropertyValue;

class GWCPROPCTRL_LINKAGE GWCPropertiesFactoryBase
{
	// Construction
	public:
		GWCPropertiesFactoryBase() : m_refCounter(0) {}
		virtual ~GWCPropertiesFactoryBase() {}

	// Attributes
	protected:
		int m_refCounter;

	// Operations
	public:
		void AddRef();
		bool Release();
		virtual int InitializePropertyIds(int startPropertyId) = 0;
		virtual GWCPropertyValue* CreatePropertyValue(int id, int type, void* data) = 0;
};

#endif // __GWCPROPERTIESFACTORYBASE_H
