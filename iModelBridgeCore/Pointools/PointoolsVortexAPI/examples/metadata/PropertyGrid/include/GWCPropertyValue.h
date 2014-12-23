// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCPROPERTYVALUE_H
#define __GWCPROPERTYVALUE_H

#include "GWCPropertiesLink.h"
#include "GWCWrappedItemCollection.h"
#include "bitmagic\bm.h"
#include <map>

class GWCPropertyLook;
class GWCPropertiesCtrl;

typedef std::map<stlstring,int> EnumMap;

class GWCPROPCTRL_LINKAGE GWCPropertyValue  
{
	// Constants
	public:
		static int CSTRING;
		static int STLSTRING;
		static int BOOL;
		static int INT;
		static int UINT;
		static int DOUBLE;
		static int FLOAT;
		static int SHORT;
		static int USHORT;
		static int CHAR;
		static int ENUM;
		static int BITSET;

	// Construction
	public:
		GWCPropertyValue(int id, int type);
		virtual ~GWCPropertyValue();

	// Attributes
	protected:
		// An iterator to the property that contains this value
		GWCDeepIterator m_iterItem;

		// Type of this value. It can be any of the above constants plus any constant
		// that you define in derived classes. Take care not to define the same values
		// to your own constants (built-in types are defined in GWCPropertyValue body).
		int m_type;

		// A pointer to the user data
		union {
			CString* m_strData;
			stlstring* m_stlStrData;
			bool* m_bData;
			int* m_nData;
			unsigned int* m_unData;
			double* m_dData;
			float* m_fData;
			short* m_sData;
			unsigned short* m_usData;
			unsigned char* m_cData;
			bm::bvector<>* m_bsData;
		};

		// Map used for enumeration type restricted values (string -> enum value)
		EnumMap m_enumMap;

		// Set of constraints for this value, ie this value can't have other values
		// than these constraints.
		CTypedPtrArray<CPtrArray,GWCPropertyValue*> m_restrictedValues;

		// Unique identifier of this value object. It has the same value as the owner property.
		int m_id;

		// Default strings used for boolean Value objects in in-place listboxes.
		static CString m_defaultBool[2];

		// Optional look attached to this value.
		GWCPropertyLook* m_look;

		// True if this Value object creates its own data. False if it just points to
		// a user data.
		bool m_manageData;

		// Number of digits after the decimal point for values of type DOUBLE and FLOAT.
		int m_precision;

		// A pointer to the parent PropertyGrid.
		GWCPropertiesCtrl* m_pPropCtrl;

		// Character used to separate child property string values in a grouped property.
		TCHAR m_groupedValueSeparator;

		CString m_label;

	// Operations
	public:
		GWCPropertiesCtrl* GetPropCtrl();
		void SetGroupedValueSeparator(TCHAR separator);
		TCHAR GetGroupedValueSeparator();
		void SetPropCtrl(GWCPropertiesCtrl* pPropCtrl);
		int GetID();
		int GetType();
		void SetPrecision(int precision);
		int GetPrecision();

		void UpdateStringValue(CString value);
		void SetStringValue(CString value);

		void DrawValue(CDC* pDC, CRect& valueRect, GWCDeepIterator& iter, bool drawString);
		virtual void OnDraw(CDC* pDC, CRect valueRect, GWCDeepIterator& iter, bool drawString);

		bool IsManaged();
		void RestrictValues(int type, void* arg, ...);
		GWCPropertyValue* AddRestrictedValue(int type, void* arg, LPCTSTR label = NULL);
		void AddRestrictedValue(LPCTSTR idStr, int enumValue);
		int GetRestrictedValueCount();
		GWCPropertyValue* GetRestrictedValue(int index);
		void ClearRestrictedValues();

		virtual CString GetStringValue(int* enumValue = NULL);
		virtual CString GetDisplayString();
		virtual void _SetStringValue(CString value);
		virtual CRect GetStringValueRect(CDC* pDC, CFont* font, CRect& valueRect, CPoint& point);
		virtual CSize GetTooltipStringSize(CDC* pDC, CRect& valueRect, CPoint& point);
		virtual void ChangeDataPointer(void* data, bool managed);
		virtual CString GetTooltipString(int line);

		void SetLook(GWCPropertyLook* pLook);
		GWCPropertyLook* GetLook();

		void SetLabel(LPCTSTR label);
		const CString& GetLabel();

	protected:
		void NotifySetStringValueToPropCtrl();
		void SetIterItem(GWCDeepIterator iter);
		void CheckGroupedProperty(CString& value);

	friend GWCInternalGrid;
};

#endif // __GWCPROPERTYVALUE_H
