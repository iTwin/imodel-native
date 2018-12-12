// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCXPTHEME_H
#define __GWCXPTHEME_H

#include "GWCPropertiesLink.h"

#ifndef	HTHEME
#define	HTHEME	HANDLE
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED		0x031A
#endif

class GWCPROPCTRL_LINKAGE GWCXPTheme  
{
	// Construction
	public:
		GWCXPTheme();
		virtual ~GWCXPTheme();

	// Attributes
	protected:
		// Handle to the opened DLL "UxTheme.dll". It is NULL on non XP systems where this file doesn't exist.
		HMODULE m_hThemeDll;

		// Handle to a theme opened by OpenThemeData.
		HTHEME m_hThemeTreeview;
		HTHEME m_hThemeCombobox;
		HTHEME m_hThemeButton;

		// Handle to the window that displays a theme.
		HWND m_hWnd;

		// True if a manifest is used by the client application (embedded in the executable resources, or
		// present in the file system along the executable file).
		bool m_hasManifest;

		// True if the program runs under Windows XP, false otherwise.
		bool m_isXP;

	// Operations
	public:
		void InitializeTheme(HWND hWnd);
		bool DrawPlusMinusSign(CDC* pDC, CRect& signRect, bool drawPlus);
		bool DrawCheckBox(CDC* pDC, CRect& frameRect, bool checked, bool enabled);
		bool DrawRadioButton(CDC* pDC, CRect& buttonRect, bool checked, bool enabled);
		bool DrawButton(CDC* pDC, CRect& buttonRect, bool pushed);
		bool DrawDropDownArrow(CDC* pDC, CRect& buttonRect, bool pushed);
		bool IsAppThemed();
		TCHAR GetPasswordChar();

	protected:
		HTHEME OpenThemeData(HWND hWnd, LPCWSTR pszClassList);
		void CloseThemeData(HTHEME hTheme);
		void DrawThemeParentBackground(CDC* pDC, HTHEME hTheme, RECT *pRect);
		void DrawThemeBackground(CDC* pDC, HTHEME hTheme, int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect);
		void DrawThemeText(CDC* pDC, HTHEME hTheme, int iPartId, int iStateId, LPCTSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect);
		BOOL IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, int iPartId, int iStateId);
};

#endif // __GWCXPTHEME_H
