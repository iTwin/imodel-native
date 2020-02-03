/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/NumEditCtrl.h $
|    $RCSfile: NumEditCtrl.h,v $
|   $Revision: 1.2 $
|       $Date: 2006/02/14 15:15:03 $
|     $Author: BENTLEY\Simon.Normand $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

class NumEditCtrl : public CEdit
{
    public:
        NumEditCtrl();
        virtual ~NumEditCtrl();

       double GetNumericValue() {return m_NumericValue;}
       void   SetNumericValue(double   NumericValue);
       void   SetAttributes  (short   NumDecimalPlaces);
       void   SetRange       (double low, double high);

       bool   HasBeenInitialized();
   
        // Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(NumEditCtrl)
        protected:
        //}}AFX_VIRTUAL

    protected:
        //{{AFX_MSG(NumEditCtrl)
        afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg BOOL OnChange();
        afx_msg BOOL OnKillfocus();
        //}}AFX_MSG

        DECLARE_MESSAGE_MAP()
    private :
   
       short   m_NumDecimalPlaces;   
       double  m_NumericValue;
       TCHAR    m_DecimalSymbol;
   
       double  m_LowRange;
       double  m_HighRange;

       BOOL    m_DecimalAlreadyUsed;   
       BOOL    m_HasValue;
       bool    m_Initialized;

       void   SetNumericValue(double   NumericValue, BOOL FormatValue);
       void   SetNumericText   (BOOL InsertFormatting);
       double RoundToDecimal (double Value, short DecimalPlaces);
       BOOL   DoesCharacterPass(UINT nChar, int CharacterPosition);
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
