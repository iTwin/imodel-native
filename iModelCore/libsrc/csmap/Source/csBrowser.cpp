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

#include "cs_map.h"

#if defined (__MFC__)

#include "cs_mfc.h"
#include "cs_hlp.h"

/**********************************************************************
**	CcsBrowse csBrowse;
**		elSelect.SetInitialKeyName (initialName);
**		finalName = GetSelectedKeyName ();
**********************************************************************/

extern "C" int EXP_LVL1 CS_csBrowser (char* keyName)
{
	INT_PTR status;
	CcsBrowse csBrowse;
	TCHAR tcKeyName [32];

	CSt_strNCpy (tcKeyName,keyName,tchrCount (tcKeyName));
	csBrowse.SetInitialKeyName (tcKeyName);
	status = csBrowse.DoModal ();
	if (status == IDOK)
	{
		CSt_strNCpy (tcKeyName,csBrowse.GetSelectedKeyName (),cs_KEYNM_DEF);
		CSt_strNCpy (keyName,tcKeyName,cs_KEYNM_DEF);
	}
	return (int)(status);
}

/////////////////////////////////////////////////////////////////////////////
// CcsBrowse dialog
CcsBrowse::CcsBrowse(CWnd* pParent /*=NULL*/)
	: CDialog(CcsBrowse::IDD, pParent)
{
	//{{AFX_DATA_INIT(CcsBrowse)
	m_Description = _T("");
	m_RefTo = _T("");
	m_Source = _T("");
	m_Unit = _T("");
	//}}AFX_DATA_INIT
	m_KeyName = _T("");
	m_Group = _T("");
}
void CcsBrowse::SetInitialKeyName (const TCHAR* keyName)
{
	char kyTemp [cs_KEYNM_DEF + 2];
	TCHAR tcTemp [cs_KEYNM_DEF + 2];

	CSt_strNCpy (kyTemp,keyName,sizeof (kyTemp));
	CS_nampp (kyTemp);
	if (CS_csIsValid (kyTemp))
	{
		CSt_strNCpy (tcTemp,kyTemp,tchrCount (tcTemp));
		m_KeyName = tcTemp;
	}
}
BOOL CcsBrowse::OnInitDialog ()
{
	int ii, idx, st;
	CWnd *wp;
	struct cs_Csdef_ *csPtr;
	TCHAR grpName [24];
	TCHAR grpDescr [64];
	CString select;

	CDialog::OnInitDialog ();

	// Determine the appropriate group from the initial key name.
	// Set up the group combo, and select the appropriate group.
	// Set up the key name combo for the appropriate group, and
	// select the desired key name.

	if (!CSt_csIsValid ((LPCTSTR)m_KeyName))
	{
		m_KeyName = _T("LL");
	}
	csPtr = CSt_csdef ((LPCTSTR)(m_KeyName));
	if (csPtr == NULL)
	{
		TCHAR errBufr [256];
		CSt_errmsg (errBufr,tchrCount (errBufr));
		AfxMessageBox (errBufr,MB_OK,0);
		return FALSE;
	}
	CSt_strNCpy (grpName,csPtr->group,tchrCount (grpName));
	m_Group = grpName;
	CS_free (csPtr);
	csPtr = 0;

	select.Empty ();
	m_GroupCombo.ResetContent ();
	for (ii = 0;
		 CSt_csGrpEnum (ii,grpName,tchrCount (grpName),grpDescr,tchrCount (grpDescr)) > 0;
		 ii += 1)
	{
		if (ii == 0) select = grpDescr;
		idx = m_GroupCombo.AddString (grpDescr);
		m_GroupCombo.SetItemData (idx,ii);
		if (!_tcscmp (grpName,m_Group)) select = grpDescr;
	}
	if (ii > 0) m_GroupCombo.SelectString (-1,select);
	// Now, the Key Name combo
	RebuildKeynameCombo (m_Group);
	st = m_KeyNameCombo.SelectString (-1,m_KeyName);
	if (st == CB_ERR) m_KeyNameCombo.SetCurSel (0);
	OnSelchangeBrcsKeynm();

	// Disable the Help button if help is not available.
	wp = GetDlgItem (ID_BRWS_HELP);
	if (wp != NULL) wp->EnableWindow (CS_isHlpAvailable ());
	return TRUE;
}
void CcsBrowse::RebuildKeynameCombo (const TCHAR* grpName)
{
	int ii, idx;
	struct csT_Csgrplst_ csDescr;

	m_KeyNameCombo.ResetContent ();
	for (ii = 0;
		 CSt_csEnumByGroup (ii,grpName,&csDescr) > 0;
		 ii += 1)
	{
		idx = m_KeyNameCombo.AddString (csDescr.key_nm);
		m_KeyNameCombo.SetItemData (idx,ii);
	}
	if (ii == 0)
	{
		idx = m_KeyNameCombo.AddString (_T("<none>"));
		m_KeyNameCombo.SetItemData (idx,9999999);
	}
}
const TCHAR* CcsBrowse::GetSelectedKeyName (void)
{
	return (LPCTSTR)m_KeyName;
}
void CcsBrowse::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CcsBrowse)
	DDX_Control(pDX, IDC_BRCS_KEYNM, m_KeyNameCombo);
	DDX_Control(pDX, IDC_BRCS_GROUP, m_GroupCombo);
	DDX_Text(pDX, IDC_BRCS_DSCR, m_Description);
	DDV_MaxChars(pDX, m_Description, 63);
	DDX_Text(pDX, IDC_BRCS_REFTO, m_RefTo);
	DDV_MaxChars(pDX, m_RefTo, 48);
	DDX_Text(pDX, IDC_BRCS_SOURCE, m_Source);
	DDV_MaxChars(pDX, m_Source, 63);
	DDX_Text(pDX, IDC_BRCS_UNIT, m_Unit);
	DDV_MaxChars(pDX, m_Unit, 24);
	//}}AFX_DATA_MAP
}
BEGIN_MESSAGE_MAP(CcsBrowse, CDialog)
	//{{AFX_MSG_MAP(CcsBrowse)
	ON_BN_CLICKED(ID_BRWS_HELP, OnHelp)
	ON_CBN_SELENDOK(IDC_BRCS_GROUP, OnSelendokBrcsGroup)
	ON_CBN_SELCHANGE(IDC_BRCS_KEYNM, OnSelchangeBrcsKeynm)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CcsBrowse message handlers
void CcsBrowse::OnHelp() 
{
	CSwinhlp (m_hWnd,csHLPID_CSBROWSER);
}
void CcsBrowse::OnOK ()
{
	if (!UpdateData (TRUE)) return;
	if (!CSt_csIsValid ((LPCTSTR)m_KeyName))
	{
		AfxMessageBox (_T("Coordinate system selection is invalid; please select again or Cancel."),MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	CDialog::OnOK ();
	return;
}
void CcsBrowse::OnSelendokBrcsGroup() 
{
	int ii, idx, st;
	TCHAR grpName [24];
	TCHAR grpDescr [64];

	ii = m_GroupCombo.GetCurSel ();
	idx = static_cast<int>(m_GroupCombo.GetItemData (ii));
	st = CSt_csGrpEnum (idx,grpName,tchrCount (grpName),grpDescr,tchrCount (grpDescr));
	ASSERT (st > 0);
	m_Group = grpName;
	RebuildKeynameCombo (grpName);
	m_KeyNameCombo.SetCurSel (0);
	OnSelchangeBrcsKeynm ();
	return;
}
void CcsBrowse::OnSelchangeBrcsKeynm() 
{
	int ii, idx, st;
	struct csT_Csgrplst_ csDescr;

	ii = m_KeyNameCombo.GetCurSel ();
	idx = static_cast<int>(m_KeyNameCombo.GetItemData (ii));
	st = -1;
	if (idx < 9999999)
	{
		st = CSt_csEnumByGroup (idx,(LPCTSTR)m_Group,&csDescr);
		if (st > 0)
		{
			m_KeyName = csDescr.key_nm;
			m_Description = csDescr.descr;
			m_Source = csDescr.source;
			m_RefTo.Format (_T("%s: %s"),csDescr.ref_typ,csDescr.ref_to);
			m_Unit = csDescr.unit;
		}
	}
	if (st <= 0)
	{
		m_KeyName = _T("");
		m_Description = _T("");
		m_Source = _T("");
		m_RefTo = _T("");
		m_Unit = _T("");
	}
	UpdateData (FALSE);
}
#endif
