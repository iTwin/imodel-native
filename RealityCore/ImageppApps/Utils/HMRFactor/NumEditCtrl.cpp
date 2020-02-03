/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/NumEditCtrl.cpp $
|    $RCSfile: NumEditCtrl.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2011/07/18 21:12:30 $
|     $Author: Donald.Morissette $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// NumEditC.cpp : implementation file
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "NumEditCtrl.h"
#include <math.h>
#include <afxole.h>
#include <locale.h>
#include <Imagepp/h/HmrMacro.h>

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
static TCHAR THIS_FILE[] = _TEXT(__FILE__);
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
NumEditCtrl::NumEditCtrl()
    {
    m_DecimalAlreadyUsed = false;
    m_NumDecimalPlaces   = 3;
    m_NumericValue       = 0;
    m_Initialized        = false;
    m_LowRange           = -(double)MAXDWORD;
    m_HighRange          = (double)MAXDWORD;
    m_DecimalSymbol      =  *localeconv ()->decimal_point;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
NumEditCtrl::~NumEditCtrl()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(NumEditCtrl, CEdit)
    //{{AFX_MSG_MAP(NumEditCtrl)
    ON_WM_CHAR()
    ON_CONTROL_REFLECT_EX(EN_KILLFOCUS, OnKillfocus)
    ON_CONTROL_REFLECT_EX(EN_CHANGE, OnChange)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// NumEditCtrl message handlers
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void NumEditCtrl::OnChar
(
UINT nChar,
UINT nRepCnt,
UINT nFlags
) 
    {
    int start_sel;
    INT end_sel;

    GetSel(start_sel, end_sel);

    // if character passes for the position, continue
    if(DoesCharacterPass(nChar, start_sel))
        {
        CEdit::OnChar(nChar, nRepCnt, nFlags);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
BOOL NumEditCtrl::DoesCharacterPass
(
UINT nChar,
int CharacterPosition
)
    { 
    bool allow_char = true;

    // if the character is not allowed, return without doing anything
    // check isprint to ensure that unprinted characters are passed 
    // to cedit (ctrl-v,etc.)

    if(!isdigit(nChar) && isprint(nChar))
        {
        allow_char = false;

        if (nChar == m_DecimalSymbol && !m_DecimalAlreadyUsed)
            {
            allow_char = true;
            }
        }
    return allow_char;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
BOOL NumEditCtrl::OnChange() 
    {
    CString workstr;
    GetWindowText(workstr);

    // use change to keep up with whether the decimal has been used
    if(workstr.Find(m_DecimalSymbol) != -1)
        {
        m_DecimalAlreadyUsed = true;
        }
    else
        {
        m_DecimalAlreadyUsed = false;
        }

    SetNumericValue (_tstof (workstr), false);

    return false; // Return false to send a EN_CHAMGE msg to our owner..
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void NumEditCtrl::SetNumericText
(
BOOL ShowFormatting
)
    {
    CString workstr1, workstr2 = _T("");
    int  decimal, sign, loop;
    int wholenum_place, string_length, num_whole_nums;

    // take value from edit control's double value
    workstr1 = _fcvt( m_NumericValue, m_NumDecimalPlaces, &decimal, &sign );

    string_length = workstr1.GetLength();
    num_whole_nums = string_length - m_NumDecimalPlaces;

    if(sign == 0)
        {
        workstr2 = _T("");
        }
    else
        {
        workstr2 = _T("-");
        }

    if (decimal < 0)
        {
        workstr2 = workstr2 + _T("0") + m_DecimalSymbol;  // 0.  or 0,)
        for (int i=decimal; i <0 ; i++)
            {
            workstr2 += _T("0");
            }
        }

    for(loop = 0; loop < string_length; loop++)
        {
        wholenum_place = num_whole_nums - loop;

        if(decimal == loop)
            {
            if (decimal == 0)
                workstr2 = workstr2 +  _T("0") + m_DecimalSymbol;  // 0.  or 0,)
            else
                workstr2 = workstr2 + m_DecimalSymbol;

            }

        workstr2 = workstr2 + workstr1[loop];
        }

    CString currentstr;
    GetWindowText(currentstr);

    // If formating, always set the value, else if the value is different than the current, set the new value
    if (ShowFormatting || !HDOUBLE_EQUAL_EPSILON (_tstof(currentstr), _tstof (workstr2)))
        SetWindowText(workstr2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void NumEditCtrl::SetNumericValue
(
double NumericValue
)
    {
    SetNumericValue (NumericValue, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void NumEditCtrl::SetNumericValue
(
double NumericValue,
BOOL FormatValue
) 
    {
    if (HDOUBLE_GREATER_OR_EQUAL_EPSILON (NumericValue, m_LowRange) && HDOUBLE_SMALLER_OR_EQUAL_EPSILON (NumericValue, m_HighRange))
        {
        m_NumericValue = NumericValue;
        m_Initialized = true;

        // Set text with formatting if selected
        SetNumericText(FormatValue);
        }

    RedrawWindow();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
BOOL NumEditCtrl::OnKillfocus() 
    {
    CString workstr;
    double value;

    GetWindowText(workstr);

    if(workstr.GetLength()>0)
        {
        // properly round and convert
        _tscanf(workstr, "%lf", &value);
        value = RoundToDecimal(value, m_NumDecimalPlaces);

        // Set member value
        if (HDOUBLE_GREATER_OR_EQUAL_EPSILON (value, m_LowRange) && HDOUBLE_SMALLER_OR_EQUAL_EPSILON (value, m_HighRange))
            m_NumericValue = value;

        // Set text with formatting if selected
        SetNumericText(true);
        }
    else
        {
        SetNumericText(true);
        }
    return false; // Return false to send a kill focus msg to our owner..
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
double NumEditCtrl::RoundToDecimal
(
double Value,
short DecimalPlaces
)
    {
    double ret_value;
    long   temp_value;
    double raise_to;
    double tolerence;
    double final_diff;

    raise_to = pow(10.0,DecimalPlaces);
    tolerence = 1.0 / raise_to;
    if (Value >= 0)
        {
        // add tolerence/2 to round up since putting it into a 
        // long truncates the value
        temp_value = (long)((Value + (tolerence/2.0)) * raise_to);
        }
    else
        {
        // subtract tolerence/2 to round down since putting 
        // it into a long truncates the value
        temp_value = (long)((Value - (tolerence/2.0)) * raise_to);
        }

    ret_value = (double)temp_value / raise_to;

    final_diff = Value - ret_value;
    if ((final_diff < -tolerence) ||  (final_diff > tolerence))
        {
        // Removing assert because it is annoying
        // assert(false);
        ret_value = Value;
        }
    return(ret_value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void NumEditCtrl::SetAttributes
(
short NumDecimalPlaces
)
    {
    m_Initialized      = false;
    m_NumDecimalPlaces = NumDecimalPlaces;

    // Set text with formatting if selected
    SetNumericText(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void NumEditCtrl::SetRange
(
double low,
double high
)
    {
    m_LowRange = low;
    m_HighRange = high;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool NumEditCtrl::HasBeenInitialized()
    {
    return m_Initialized;
    }