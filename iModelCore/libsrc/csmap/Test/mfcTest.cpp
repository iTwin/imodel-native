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
**    Calling Sequence:	mfcTest	data_directory
**********************************************************************/

#include <afxwin.h>
#include "mfcTest.h"
#include "cs_map.h"
#include "cs_mfc.h"

extern "C" short cs_Protect;
extern "C" char cs_Unique;

///////////////////////////////////////////////////////////////////////////////
// Application class implementation.
CmfcTest::CmfcTest()
{
	// We don't really need to do much here. The CWinApp constructor
	// does most everything.
	m_Instance = 0;
	m_IsWindowsNT = false;
	m_FullPath [0] = '\0';
	m_Directory [0] = '\0';
	m_FileName [0] = '\0';
}
BOOL CmfcTest::InitInstance ()
{
	int st;
	char *cp;
	OSVERSIONINFO version;
	char ctemp [1024];
	TCHAR tcTemp [1024];

	cs_Protect = -1;
	cs_Unique = '\0';

	// Here we simply capture some of the environment.  We do, however,
	// create our own menu. This enables us to have a menu without
	// having to have a resource file, and a class wizard file, and
	// all of that crap.

	m_Instance = AfxGetInstanceHandle ();

	// Get a full path name to the EXE file we came from.
	if (GetModuleFileName (m_Instance,tcTemp,chrCount (tcTemp)) < 0) return (FALSE);
	CSt_strNCpy (ctemp,tcTemp,chrCount (ctemp));
	
	strncpy (m_FullPath,ctemp,sizeof (m_FullPath));
	m_FullPath [sizeof (m_FullPath) - 1] = '\0';

	// Extract the directory portion only.
	cp = strrchr (ctemp,'\\');
	if (cp == NULL) return (FALSE);
	*cp++ = '\0';
	strncpy (m_Directory,ctemp,sizeof (m_Directory));
	m_Directory [sizeof (m_Directory) - 1] = '\0';

	// Capture the file name.
	strncpy (m_FileName,cp,sizeof (m_FileName));
	m_FileName [sizeof (m_FileName) - 1] = '\0';

	// See if the directory whence we came has the mapping data.
	st = CS_altdr (m_Directory);
	if (st != 0)
	{
		// Nope, try another location.
		strcpy (ctemp,m_Directory);
		strcat (ctemp,"\\DATA");
		st = CS_altdr (ctemp);
	}
	if (st != 0)
	{
		// Still no luck, try a sub-directory of the whence we came.
		strcpy (ctemp,m_Directory);
		strcat (ctemp,"\\DIctionaries");
		st = CS_altdr (ctemp);
	}
	if (st != 0)
	{
		// Still no luck, try yet another place.
		strcpy (ctemp,m_Directory);
		strcat (ctemp,"\\..\\Dictionaries");
		st = CS_altdr (ctemp);
	}
	if (st != 0)
	{
		// Still no luck, try a sub-directory of the whence we came.
		strcpy (ctemp,m_Directory);
		strcat (ctemp,"\\dictnary");
		st = CS_altdr (ctemp);
	}
	if (st != 0)
	{
		// Still no luck, try yet another place.
		strcpy (ctemp,m_Directory);
		strcat (ctemp,"\\..\\dictnary");
		st = CS_altdr (ctemp);
	}
	if (st != 0)
	{
		// Still no luck, see if there is an environmental variable.
		st = CS_altdr (NULL);
	}
	if (st != 0)
	{
		// Still no luck, try the current working directory.
		st = CS_altdr ("");
	}
	if (st != 0)
	{
		// Still no luck, try the developement system case.
		strcpy (ctemp,"C:\\Program Files\\Common Files\\GeodeticData");
		st = CS_altdr (ctemp);
	}

	if (st != 0)
	{
		// No luck at all, issue a message box warning.  We do not
		// exit as this thing provides a directory dialog which can
		// be used to fix the problem.
		MessageBox (0,_T("Could not locate the mapping data directory."),_T("Mapping Data Error"),MB_OK | MB_ICONINFORMATION);
	}

	// Determine the opperating system in use.
	version.dwOSVersionInfoSize = sizeof (version);
	if (GetVersionEx (&version))
	{
		m_IsWindowsNT = (version.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	// Make the main window, such as it is, visible.
	m_pMainWnd = new CmfcTestWnd;
	m_pMainWnd->ShowWindow (m_nCmdShow);
	m_pMainWnd->UpdateWindow ();

	// Build a menu programmatically.  This way, we don't need a silly
	// resource file, for the menu anyway.
	CMenu mainMenu;
	CMenu popMenu;
	popMenu.CreatePopupMenu ();
	popMenu.AppendMenu (MF_STRING,IDM_DIRECTORIES,_T("Directories"));
	popMenu.AppendMenu (MF_STRING,IDM_CS_BROWSE,_T("csBrowser"));
	popMenu.AppendMenu (MF_STRING,IDM_CS_BROWSE2,_T("csDualBrowser"));
	popMenu.AppendMenu (MF_STRING,IDM_ED_COORDSYS,_T("csEdit"));
	popMenu.AppendMenu (MF_STRING,IDM_ED_DATUMS,_T("dtEdit"));
	popMenu.AppendMenu (MF_STRING,IDM_ED_ELIPSOID,_T("elEdit"));
//	popMenu.AppendMenu (MF_STRING,IDM_ED_GDC,_T("gdcEdit"));
	popMenu.AppendMenu (MF_STRING,IDM_TEST_CS,_T("csTest"));
	popMenu.AppendMenu (MF_STRING,IDM_TEST_MG,_T("mgTest"));
//	popMenu.AppendMenu (MF_STRING,IDM_FTOA_FRMT,_T("Ftoa Format"));
	popMenu.AppendMenu (MF_STRING,IDM_RECOVER,_T("Recover"));
	popMenu.AppendMenu (MF_STRING,IDM_TESTFUNC,_T("Test Function"));
	popMenu.AppendMenu (MF_STRING,IDM_EXIT,_T("E&xit"));
	mainMenu.CreateMenu ();
	mainMenu.AppendMenu (MF_POPUP,(UINT)popMenu.Detach (),_T("Test"));
	m_pMainWnd->SetMenu (&mainMenu);
	mainMenu.Detach ();

	return TRUE;
}

extern "C" struct csVertconUS_* csVertconUS;
extern "C" struct cs_Ostn97_ *cs_Ostn97Ptr;
extern "C" struct cs_Osgm91_ *cs_Osgm91Ptr;
extern "C" struct cs_Ostn02_ *cs_Ostn02Ptr;

extern "C" int csVertconUSCnt;

int CmfcTest::ExitInstance ()
{
	// Clean up before exit.
	CS_recvr ();

	CSdeleteVertconUS (csVertconUS);
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
// Main Window class implementation.
BEGIN_MESSAGE_MAP (CmfcTestWnd,CFrameWnd)
	ON_COMMAND (IDM_DIRECTORIES,OnDirectories)
	ON_COMMAND (IDM_CS_BROWSE  ,OnBrowse)
	ON_COMMAND (IDM_CS_BROWSE2 ,OnDualBrowse)
	ON_COMMAND (IDM_ED_COORDSYS,OnCsEdit)
	ON_COMMAND (IDM_ED_DATUMS,  OnDtEdit)
	ON_COMMAND (IDM_ED_ELIPSOID,OnElEdit)
	ON_COMMAND (IDM_ED_GDC,     OnGdcEdit)
	ON_COMMAND (IDM_TEST_CS,    OnCsTest)
	ON_COMMAND (IDM_TEST_MG,    OnMgTest)
	ON_COMMAND (IDM_RECOVER,    OnRecover)
	ON_COMMAND (IDM_TESTFUNC,   OnTestFunc)
	ON_COMMAND (IDM_EXIT,       OnExit)
END_MESSAGE_MAP ()


extern "C" struct cs_Ostn97_ *CSnewOstn97 (const char *filePath);
extern "C" double CStestOstn97 (struct cs_Ostn97_ *__This);

CmfcTestWnd::CmfcTestWnd ()
{
	Create (NULL,_T("CS-MAP MFC Test Application"),
			WS_OVERLAPPEDWINDOW,
			CRect (20,20,320,240));	
	strcpy (m_CsKeyName,"US48");
	strcpy (m_DtKeyName,"WGS84");
	strcpy (m_ElKeyName,"WGS84");
	strcpy (m_SrcSystem,"UTM27-13");
	strcpy (m_TrgSystem,"LL83");
	m_SrcXYZ [0] = 500000.0;
	m_SrcXYZ [1] = 3900000.0;
	m_SrcXYZ [2] = 0.0;
	strcpy (m_MgrsEllipsoid,"WGS84");
	strcpy (m_BrowseKeyNm,"LL");
	strcpy (m_SrcKeyName,"UTM27-13");
	strcpy (m_TrgKeyName,"LL83");
	strcpy (m_GdcName,"Nad27ToNad83.gdc");
	m_FtoaFrmt = cs_ATOF_LNGDFLT;
}
void CmfcTestWnd::OnDirectories ()
{
	// Non-sense stuff is for testing/debugging.
	int status;
	status = CS_csDataDir (0);
	return;
}
void CmfcTestWnd::OnBrowse ()
{
	CS_csBrowser (m_BrowseKeyNm);
}
void CmfcTestWnd::OnDualBrowse ()
{
	CS_csDualBrowser (m_SrcKeyName,m_TrgKeyName);
}
void CmfcTestWnd::OnCsEdit ()
{
	CS_csEditor (m_CsKeyName);
}
void CmfcTestWnd::OnDtEdit ()
{
	CS_dtEditor (m_DtKeyName);
}
void CmfcTestWnd::OnElEdit ()
{
	CS_elEditor (m_ElKeyName);
}
void CmfcTestWnd::OnGdcEdit ()
{
//	CS_gdcEditor (m_GdcName);
}
void CmfcTestWnd::OnCsTest ()
{
	CS_csTest (m_SrcSystem,m_TrgSystem,m_SrcXYZ);
}
int CS_geoCtrDlg (char *elKeyName,double geographic [3],double geoCtr [3]);
void CmfcTestWnd::OnMgTest ()
{
	CS_mgTest (m_MgrsEllipsoid);
}

void CmfcTestWnd::OnRecover ()
{
	CS_recvr ();

	CSdeleteVertconUS (csVertconUS);
	csVertconUS = NULL;
	csVertconUSCnt = 0;
	CSdeleteOstn97 (cs_Ostn97Ptr);
	cs_Ostn97Ptr = NULL;
	CSdeleteOsgm91 (cs_Osgm91Ptr);
	cs_Ostn97Ptr = NULL;
	CSdeleteOstn02 (cs_Ostn02Ptr);
	cs_Ostn02Ptr = NULL;
}
void CmfcTestWnd::OnTestFunc ()
{
}
void CmfcTestWnd::OnExit ()
{
	SendMessage (WM_CLOSE,0,0);
}
//////////////////////////////////////////////////////////////////////////////
// Declare the single instance of the application object.
CmfcTest mfcTest;

wchar_t* CS_strNCpy (wchar_t* trg,wchar_t* src,unsigned chCnt)
{
	wcsncpy (trg,src,chCnt);
	*(trg + chCnt - 1) = L'\0';
	return (trg + wcslen (trg));
}
wchar_t* CS_strNCpy (wchar_t* trg,char* src,unsigned chCnt)
{
	mbstowcs (trg,src,chCnt);
	*(trg + chCnt - 1) = L'\0';
	return (trg + wcslen (trg));
}
char* CS_strNCpy (char* trg,wchar_t* src,unsigned chCnt)
{
	wcstombs (trg,src,chCnt);
	*(trg + chCnt - 1) = '\0';
	return (trg + strlen (trg));
}
char* CS_strNCpy (char* trg,char* src,unsigned chCnt)
{
	strncpy (trg,src,chCnt);
	*(trg + chCnt - 1) = '\0';
	return (trg + strlen (trg));
}
