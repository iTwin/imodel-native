//---------------------------------------------------------------------------+
// $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

//---------------------------------------------------------------------------
// Macros and Constants
//---------------------------------------------------------------------------

#define GUID_STRING_MAX_SIZE 40

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

HRESULT aecGuid_generate   ( InroadsGuid *newGuid );
int     aecGuid_compare    ( const InroadsGuid *g1, const InroadsGuid *g2 );    // designed for use with qsort
BOOL    aecGuid_equal      ( const InroadsGuid *g1, const InroadsGuid *g2 );
HRESULT aecGuid_fromString ( InroadsGuid *guid, LPCWSTR str );
HRESULT aecGuid_toString   ( LPWSTR str, const InroadsGuid *guid );
HRESULT aecGuid_clear      ( InroadsGuid *g );
HRESULT aecGuid_copy       ( InroadsGuid *g1, const InroadsGuid *g2 );
BOOL    aecGuid_isClear    ( const InroadsGuid *g );
