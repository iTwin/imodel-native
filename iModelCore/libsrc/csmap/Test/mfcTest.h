/*
 * Copyright (c) 2008, Autodesk, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Autodesk, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Autodesk, Inc. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Autodesk, Inc. OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**********************************************************************
**	Defines MFC classes for the MFS Test Module.
**********************************************************************/

// Codes for the menu options.  The values are arbitrary, but unique.

#define IDM_DIRECTORIES  11
#define IDM_ED_COORDSYS  12
#define IDM_ED_DATUMS    13
#define IDM_ED_ELIPSOID  14
#define IDM_ED_GDC       15
#define IDM_TEST_CS      16
#define IDM_EXIT         17
#define IDM_CS_BROWSE    18
#define IDM_TEST_MG      19
#define IDM_CS_BROWSE2   20
#define IDM_RECOVER      21
#define IDM_TESTFUNC     23

class CmfcTest: public CWinApp
{
public:
	CmfcTest (void);
	BOOL InitInstance (void);
	int ExitInstance (void);

	HINSTANCE m_Instance;
	bool m_IsWindowsNT;
	char m_FullPath [260];
	char m_Directory [260];
	char m_FileName [260];
	CFrameWnd *m_MainFrame;
};
class CmfcTestWnd : public CFrameWnd
{
public:
	CmfcTestWnd ();
protected:
	afx_msg void OnDirectories ();
	afx_msg void OnCsEdit ();
	afx_msg void OnDtEdit ();
	afx_msg void OnElEdit ();
	afx_msg void OnGdcEdit ();
	afx_msg void OnCsTest ();
	afx_msg void OnMgTest ();
	afx_msg void OnExit   ();
	afx_msg void OnBrowse ();
	afx_msg void OnDualBrowse ();
	afx_msg void OnRecover ();
	afx_msg void OnTestFunc ();

	DECLARE_MESSAGE_MAP ();

private:
	char m_CsKeyName [24];
	char m_DtKeyName [24];
	char m_ElKeyName [24];
	char m_SrcSystem [24];
	char m_TrgSystem [24];
	double m_SrcXYZ [3];
	char m_BrowseKeyNm [24];
	char m_MgrsEllipsoid [24];
	char m_SrcKeyName [24];
	char m_TrgKeyName [24];
	char m_GdcName [64];
	long m_FtoaFrmt;
};

