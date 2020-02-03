/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/UnitComboBox.h $
|    $RCSfile: UnitComboBox.h,v $
|   $Revision: 1.3 $
|       $Date: 2006/02/20 16:00:20 $
|     $Author: BENTLEY\Simon.Normand $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "afxwin.h"
#include "Unit.h"
#include "SepComboBox.h"


#define NUMBERUNITS 34

class UnitComboBox :    public SepComboBox
    {
    public:
        UnitComboBox(void);
        ~UnitComboBox(void);

        void        Fill (const TCHAR* pDefaultUnit = NULL);
        Unit* const GetUnit ();

    private:

        Unit m_Units[NUMBERUNITS];

    
    protected:
        //{{AFX_MSG(UnitComboBox)

        //}}AFX_MSG

        DECLARE_MESSAGE_MAP()
    };
