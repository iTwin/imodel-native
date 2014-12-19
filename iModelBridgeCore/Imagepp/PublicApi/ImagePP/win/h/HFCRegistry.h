//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCRegistry.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

namespace HFCRegistry
{
HKEY        OpenRegistryKey(HKEY           pi_BaseKey,
                            const WString& pi_rKey);

HKEY        CreateRegistryKey(HKEY           pi_BaseKey,
                              const WString& pi_rKey);


bool       WriteString (HKEY           pi_Key,
                         const WString& pi_rValueName,
                         const WString& pi_rValue);
bool       ReadString(HKEY           pi_Key,
                       const WString& pi_rValueName,
                       WString*       po_pValue);

bool       WriteNumber    (HKEY           pi_RegKey,
                            const WString& pi_rValueName,
                            uint32_t        pi_Value);
bool       ReadNumber     (HKEY           pi_RegKey,
                            const WString& pi_rValueName,
                            uint32_t*            po_pValue);
bool       WriteNumber    (HKEY          pi_RegKey,
                            const WString& pi_rValueName,
                            int32_t        pi_Value);
bool       ReadNumber     (HKEY          pi_RegKey,
                            const WString& pi_rValueName,
                            int32_t*        po_pValue);


bool           WriteBinary   (HKEY           pi_RegKey,
                               const WString& pi_rValueName,
                               const Byte*   pi_pValue,
                               uint32_t       pi_ValueSize);
bool           ReadBinary (HKEY                   pi_RegKey,
                            const WString&         pi_rValueName,
                            HArrayAutoPtr<Byte>&  po_rpValue,
                            uint32_t&                po_rValueSize);

bool           EnumRegistryKeys(HKEY     pi_RegKey,
                                 DWORD    pi_Index,
                                 WString* po_pKeyName);
bool           EnumRegistryValues(HKEY     pi_RegKey,
                                   DWORD    pi_Index,
                                   WString* po_pValueName,
                                   DWORD*   pio_pType);

bool           DeleteRegistryKey(HKEY           pi_RegKey,
                                  const WString& pi_rValueName);
bool           DeleteRegistryValue(HKEY           pi_RegKey,
                                    const WString& pi_rValueName);

bool           CloseRegistryKey(HKEY pi_RegKey);
bool           FlushRegistryKey(HKEY pi_RegKey);
};