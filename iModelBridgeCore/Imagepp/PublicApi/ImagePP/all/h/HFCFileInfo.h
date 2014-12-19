//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCFileInfo.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCFileInfo
//-----------------------------------------------------------------------------
#pragma once

#ifdef _WIN32
#define HFC_FILEINFO_NORMAL   _A_NORMAL
#define HFC_FILEINFO_RDONLY   _A_RDONLY
#define HFC_FILEINFO_HIDDEN   _A_HIDDEN
#define HFC_FILEINFO_SYSTEM   _A_SYSTEM
#define HFC_FILEINFO_SUBDIR   _A_SUBDIR
#define HFC_FILEINFO_ARCH     _A_ARCH
#else
#define HFC_FILEINFO_NORMAL   0x00
#define HFC_FILEINFO_RDONLY   0x01
#define HFC_FILEINFO_HIDDEN   0x02
#define HFC_FILEINFO_SYSTEM   0x04
#define HFC_FILEINFO_SUBDIR   0x10
#define HFC_FILEINFO_ARCH     0x20
#endif
// FLAG VALUE FOR PHYSICAL FILES
// Valid Flag are mapped to define in io.h for windows
// and to equivalent value for UNIX
// HFC_FILEINFO_NORMAL   Normal. File can be read or written to without restriction. Value: 0x00
// HFC_FILEINFO_RDONLY   Read-only. File cannot be opened for writing, and a file with the same name cannot be created. Value: 0x01
// HFC_FILEINFO_HIDDEN   Hidden file. Not normally seen with the DIR command, unless the /AH option is used. Returns information about normal files as well as files with this attribute. Value: 0x02
// HFC_FILEINFO_SYSTEM   System file. Not normally seen with the DIR command, unless the /AS option is used. Value: 0x04
// HFC_FILEINFO_SUBDIR   Subdirectory. Value: 0x10
// HFC_FILEINFO_ARCH     Archive. Set whenever the file is changed, and cleared by the BACKUP command. Value: 0x20
//Multiple constants can be combined with the OR operator (|).

class HFCFileInfo
    {
public:

    typedef list< HFCFileInfo*, allocator <HFCFileInfo*> > HFCFileInfoList;

    // Constructor
    _HDLLu HFCFileInfo(const WString& pi_Name, uint32_t pi_Flag);
    _HDLLu HFCFileInfo(const HFCFileInfo&);

    // Assignment operator
    HFCFileInfo& operator=(const HFCFileInfo&);

    // Destructor
    _HDLLu virtual ~HFCFileInfo();

    // Physical file flag check
    bool IsReadOnly();
    bool IsDirectory();
    bool IsHidden();

    void SetReadOnly(bool pi_State);
    void SetDirectory(bool pi_State);
    void SetHidden(bool pi_State);

    bool FlagSet(uint32_t pi_Flag);

    const WString& GetName();
    uint32_t        GetFlag();
    WString        AsChar();
    void          SetName(const WString& pi_Name);

    bool operator==(const HFCFileInfo& pi_Src);

protected:

private:

    // No default constructor permitted
    HFCFileInfo();

    // Private methods
    void CommonCopy(const HFCFileInfo& pi_Src);

    // Attributes
    WString m_FileName;
    uint32_t m_Flag;
    };

#include "HFCFileInfo.hpp"
