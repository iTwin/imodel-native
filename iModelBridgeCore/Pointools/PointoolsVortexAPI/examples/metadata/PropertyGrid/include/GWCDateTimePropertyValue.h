// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCDATETIMEPROPERTYVALUE_H
#define __GWCDATETIMEPROPERTYVALUE_H

#include "GWCPropertiesLink.h"
#include "GWCPropertyValue.h"

class GWCPROPCTRL_LINKAGE GWCDateTimePropertyValue : public GWCPropertyValue  
{
	// Types
	public:
		static int DATE;
		static int TIME;

	// Construction
	public:
		GWCDateTimePropertyValue(int id, int type);
		virtual ~GWCDateTimePropertyValue();

	protected:
		static int CALLBACK DateTimeFmtEnumProc(LPTSTR lpTimeFormatString);

	// Attributes
	protected:
		COleDateTime* m_time;		// A pointer to the time object monitored by this object.

	public:
		static CString m_timeFormat;
		static CString m_dateFormat;

	// Operations
	public:
		COleDateTime* GetTime();
		bool DoesDisplayTime();

		virtual void OnDraw(CDC* pDC, CRect valueRect, GWCDeepIterator& iter, bool drawString);
		virtual CString GetStringValue(int* enumValue = NULL);
		virtual void ChangeDataPointer(void* data, bool managed);
};

#endif // __GWCDATETIMEPROPERTYVALUE_H
