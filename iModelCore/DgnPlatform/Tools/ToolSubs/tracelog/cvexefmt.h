/***    cvexefmt.h - format of CodeView information in exe
 *
 *      Structures, constants, etc. for reading CodeView information
 *      from the executable.
 *
 */

//  The following structures and constants describe the format of the
//  CodeView Debug OMF for that will be accepted by CodeView 4.0 and
//  later.  These are executables with signatures of NB05, NB06 and NB08.
//  There is some confusion about the signatures NB03 and NB04 so none
//  of the utilities will accept executables with these signatures.  NB07 is
//  the signature for QCWIN 1.0 packed executables.

//  All of the structures described below must start on a Int32 word boundary
//  to maintain natural alignment.  Pad space can be inserted during the
//  write operation and the addresses adjusted without affecting the contents
//  of the structures.

#ifndef __cvexefmtH__
#define __cvexefmtH__
#include <Bentley/Bentley.h>
#pragma warning(push)
#pragma warning(disable:4200)
#ifndef FAR
    #if _M_IX86 >= 300
	#define    FAR
    #else
	#define FAR far
    #endif
#endif

#ifndef _CV_INFO_INCLUDED
#include "cvinfo.h"
#endif

#pragma pack(1)

#if !defined (CV_ZEROLEN)
#define CV_ZEROLEN 0
#endif

//  Type of subsection entry.

#define sstModule           0x120
#define sstTypes            0x121
#define sstPublic           0x122
#define sstPublicSym        0x123   // publics as symbol (waiting for link)
#define sstSymbols          0x124
#define sstAlignSym         0x125
#define sstSrcLnSeg         0x126   // because link doesn't emit SrcModule
#define sstSrcModule        0x127
#define sstLibraries        0x128
#define sstGlobalSym        0x129
#define sstGlobalPub        0x12a
#define sstGlobalTypes      0x12b
#define sstMPC              0x12c
#define sstSegMap           0x12d
#define sstSegName          0x12e
#define sstPreComp          0x12f   // precompiled types
#define sstPreCompMap       0x130   // map precompiled types in global types
#define sstOffsetMap16      0x131
#define sstOffsetMap32      0x132
#define sstFileIndex        0x133   // Index of file names
#define sstStaticSym        0x134

typedef enum OMFHash {
    OMFHASH_NONE,           // no hashing
    OMFHASH_SUMUC16,        // upper case sum of chars in 16 bit table
    OMFHASH_SUMUC32,        // upper case sum of chars in 32 bit table
    OMFHASH_ADDR16,         // sorted by increasing address in 16 bit table
    OMFHASH_ADDR32          // sorted by increasing address in 32 bit table
} OMFHASH;

//  CodeView Debug OMF signature.  The signature at the end of the file is
//  a negative offset from the end of the file to another signature.  At
//  the negative offset (base address) is another signature whose filepos
//  field points to the first OMFDirHeader in a chain of directories.
//  The NB05 signature is used by the link utility to indicated a completely
//  unpacked file.  The NB06 signature is used by ilink to indicate that the
//  executable has had CodeView information from an incremental link appended
//  to the executable.  The NB08 signature is used by cvpack to indicate that
//  the CodeView Debug OMF has been packed.  CodeView will only process
//  executables with the NB08 signature.


typedef struct OMFSignature {
    char        Signature[4];   // "NBxx"
    Int32        filepos;        // offset in file
} OMFSignature ;



//  directory information structure
//  This structure contains the information describing the directory.
//  It is pointed to by the signature at the base address or the directory
//  link field of a preceeding directory.  The directory entries immediately
//  follow this structure.


typedef struct OMFDirHeader {
    UInt16  cbDirHeader;    // length of this structure
    UInt16  cbDirEntry;     // number of bytes in each directory entry
    UInt32  cDir;           // number of directorie entries
    Int32            lfoNextDir;     // offset from base of next directory
    UInt32  flags;          // status flags
} OMFDirHeader ;




//  directory structure
//  The data in this structure is used to reference the data for each
//  subsection of the CodeView Debug OMF information.  Tables that are
//  not associated with a specific module will have a module index of
//  oxffff.  These tables are the global types table, the global symbol
//  table, the global public table and the library table.


typedef struct OMFDirEntry {
    UInt16  SubSection;     // subsection type (sst...)
    UInt16  iMod;           // module index
    Int32            lfo;            // large file offset of subsection
    UInt32  cb;             // number of bytes in subsection
} OMFDirEntry ;



//  information decribing each segment in a module

typedef struct OMFSegDesc {
    UInt16  Seg;            // segment index
    UInt16  pad;            // pad to maintain alignment
    UInt32  Off;            // offset of code in segment
    UInt32  cbSeg;          // number of bytes in segment
} OMFSegDesc ;




//  per module information
//  There is one of these subsection entries for each module
//  in the executable.  The entry is generated by link/ilink.
//  This table will probably require padding because of the
//  variable length module name.

typedef struct OMFModule {
    UInt16  ovlNumber;      // overlay number
    UInt16  iLib;           // library that the module was linked from
    UInt16  cSeg;           // count of number of segments in module
    char            Style[2];       // debugging style "CV"
    OMFSegDesc      SegInfo[1];     // describes segments in module
    char            Name[CV_ZEROLEN]; // length prefixed module name padded to
                                    // Int32 word boundary
} OMFModule ;



//  Symbol hash table format
//  This structure immediately preceeds the global publics table
//  and global symbol tables.

typedef struct OMFSymHash {
    UInt16  symhash;        // symbol hash function index
    UInt16  addrhash;       // address hash function index
    UInt32  cbSymbol;       // length of symbol information
    UInt32  cbHSym;         // length of symbol hash data
    UInt32  cbHAddr;        // length of address hashdata
} OMFSymHash ;



//  Global types subsection format
//  This structure immediately preceeds the global types table.
//  The offsets in the typeOffset array are relative to the address
//  of ctypes.  Each type entry following the typeOffset array must
//  begin on a Int32 word boundary.

typedef struct OMFTypeFlags {
    UInt32  sig     :8;
    UInt32  unused  :24;
} OMFTypeFlags ;


typedef struct OMFGlobalTypes {
    OMFTypeFlags    flags;
    UInt32  cTypes;         // number of types
    UInt32  typeOffset[CV_ZEROLEN];   // array of offsets to types
} OMFGlobalTypes ;




//  Precompiled types mapping table
//  This table should be ignored by all consumers except the incremental
//  packer.


typedef struct OMFPreCompMap_16t {
    UInt16  FirstType;      // first precompiled type index
    UInt16  cTypes;         // number of precompiled types
    UInt32  signature;      // precompiled types signature
    UInt16  pad;
    CV_typ16_t      map[CV_ZEROLEN];          // mapping of precompiled types
} OMFPreCompMap_16t ;

typedef struct OMFPreCompMap_32t {
    UInt16  FirstType;      // first precompiled type index
    UInt16  cTypes;         // number of precompiled types
    UInt32  signature;      // precompiled types signature
    UInt16  pad;
    CV_typ32_t      map[CV_ZEROLEN];          // mapping of precompiled types
} OMFPreCompMap_32t ;




//  Source line to address mapping table.
//  This table is generated by the link/ilink utility from line number
//  information contained in the object file OMF data.  This table contains
//  only the code contribution for one segment from one source file.


typedef struct OMFSourceLine {
    UInt16  Seg;            // linker segment index
    UInt16  cLnOff;         // count of line/offset pairs
    UInt32  offset[1];      // array of offsets in segment
    UInt16  lineNbr[1];     // array of line lumber in source
} OMFSourceLine ;

typedef OMFSourceLine FAR * LPSL ;


//  Source file description
//  This table is generated by the linker


typedef struct OMFSourceFile {
    UInt16  cSeg;           // number of segments from source file
    UInt16  reserved;       // reserved
    UInt32  baseSrcLn[1];   // base of OMFSourceLine tables
                                    // this array is followed by array
                                    // of segment start/end pairs followed by
                                    // an array of linker indices
                                    // for each segment in the file
    UInt16  cFName;         // length of source file name
    char            Name;           // name of file padded to Int32 boundary
} OMFSourceFile ;

typedef OMFSourceFile FAR * LPSF;


//  Source line to address mapping header structure
//  This structure describes the number and location of the
//  OMFAddrLine tables for a module.  The offSrcLine entries are
//  relative to the beginning of this structure.


typedef struct OMFSourceModule {
    UInt16  cFile;          // number of OMFSourceTables
    UInt16  cSeg;           // number of segments in module
    UInt32  baseSrcFile[1]; // base of OMFSourceFile table
                                    // this array is followed by array
                                    // of segment start/end pairs followed
                                    // by an array of linker indices
                                    // for each segment in the module
} OMFSourceModule ;

typedef OMFSourceModule FAR * LPSM ;

//  sstLibraries

typedef struct OMFLibrary {
    UChar   cbLibs;     // count of library names
    char            Libs[1];    // array of length prefixed lib names (first entry zero length)
} OMFLibrary;


// sstFileIndex - An index of all of the files contributing to an
//  executable.

typedef struct OMFFileIndex {
    UInt16  cmodules;       // Number of modules
    UInt16  cfilerefs;      // Number of file references
    UInt16  modulelist[1];  // Index to beginning of list of files
                                    // for module i. (0 for module w/o files)
    UInt16  cfiles[1];      // Number of file names associated
                                    // with module i.
    UInt32  ulNames[1];     // Offsets from the beginning of this
                                    // table to the file names
    char            Names[CV_ZEROLEN];        // The length prefixed names of files
} OMFFileIndex ;


//  Offset mapping table
//  This table provides a mapping from logical to physical offsets.
//  This mapping is applied between the logical to physical mapping
//  described by the seg map table.

typedef struct OMFOffsetMap16 {
    UInt32  csegment;       // Count of physical segments

    // The next six items are repeated for each segment

    UInt32  crangeLog;      // Count of logical offset ranges
    UInt16  rgoffLog[1];    // Array of logical offsets
    short           rgbiasLog[1];   // Array of logical->physical bias
    UInt32  crangePhys;     // Count of physical offset ranges
    UInt16  rgoffPhys[1];   // Array of physical offsets
    short           rgbiasPhys[1];  // Array of physical->logical bias
} OMFOffsetMap16 ;

typedef struct OMFOffsetMap32 {
    UInt32  csection;       // Count of physical sections

    // The next six items are repeated for each section

    UInt32  crangeLog;      // Count of logical offset ranges
    UInt32  rgoffLog[1];    // Array of logical offsets
    Int32            rgbiasLog[1];   // Array of logical->physical bias
    UInt32  crangePhys;     // Count of physical offset ranges
    UInt32  rgoffPhys[1];   // Array of physical offsets
    Int32            rgbiasPhys[1];  // Array of physical->logical bias
} OMFOffsetMap32 ;

//  Pcode support.  This subsection contains debug information generated
//  by the MPC utility used to process Pcode executables.  Currently
//  it contains a mapping table from segment index (zero based) to
//  frame paragraph.  MPC converts segmented exe's to non-segmented
//  exe's for DOS support.  To avoid backpatching all CV info, this
//  table is provided for the mapping.  Additional info may be provided
//  in the future for profiler support.

typedef struct OMFMpcDebugInfo {
    UInt16  cSeg;           // number of segments in module
    UInt16  mpSegFrame[1];  // map seg (zero based) to frame
} OMFMpcDebugInfo ;

//  The following structures and constants describe the format of the
//  CodeView Debug OMF for linkers that emit executables with the NB02
//  signature.  Current utilities with the exception of cvpack and cvdump
//  will not accept or emit executables with the NB02 signature.  Cvdump
//  will dump an unpacked executable with the NB02 signature.  Cvpack will
//  read an executable with the NB02 signature but the packed executable
//  will be written with the table format, contents and signature of NB08.


//  subsection type constants

#define SSTMODULE       0x101    // Basic info. about object module
#define SSTPUBLIC       0x102    // Public symbols
#define SSTTYPES        0x103    // Type information
#define SSTSYMBOLS      0x104    // Symbol Data
#define SSTSRCLINES     0x105    // Source line information
#define SSTLIBRARIES    0x106    // Names of all library files used
#define SSTIMPORTS      0x107    // Symbols for DLL fixups
#define SSTCOMPACTED    0x108    // Compacted types section
#define SSTSRCLNSEG     0x109    // Same as source lines, contains segment


typedef struct DirEntry{
    UInt16  SubSectionType;
    UInt16  ModuleIndex;
    Int32            lfoStart;
    UInt16  Size;
} DirEntry ;


//  information decribing each segment in a module

typedef struct oldnsg {
    UInt16  Seg;         // segment index
    UInt16  Off;         // offset of code in segment
    UInt16  cbSeg;       // number of bytes in segment
} oldnsg ;


//  old subsection module information

typedef struct oldsmd {
    oldnsg          SegInfo;     // describes first segment in module
    UInt16  ovlNbr;      // overlay number
    UInt16  iLib;
    UChar   cSeg;        // Number of segments in module
    char            reserved;
    UChar   cbName[1];   // length prefixed name of module
    oldnsg          arnsg[CV_ZEROLEN];     // cSeg-1 structures exist for alloc text or comdat code
} oldsmd ;

typedef struct{
    UInt16  Seg;
    UInt32  Off;
    UInt32  cbSeg;
} oldnsg32 ;

typedef struct {
    oldnsg32        SegInfo;     // describes first segment in module
    UInt16  ovlNbr;      // overlay number
    UInt16  iLib;
    UChar   cSeg;        // Number of segments in module
    char            reserved;
    UChar   cbName[1];   // length prefixed name of module
    oldnsg32        arnsg[CV_ZEROLEN];     // cSeg-1 structures exist for alloc text or comdat code
} oldsmd32 ;

// OMFSegMap - This table contains the mapping between the logical segment indices
// used in the symbol table and the physical segments where the program is loaded

typedef struct OMFSegMapDesc {
    UInt16  flags;       // descriptor flags bit field.
    UInt16  ovl;         // the logical overlay number
    UInt16  group;       // group index into the descriptor array
    UInt16  frame;       // logical segment index - interpreted via flags
    UInt16  iSegName;    // segment or group name - index into sstSegName
    UInt16  iClassName;  // class name - index into sstSegName
    UInt32  offset;      // byte offset of the logical within the physical segment
    UInt32  cbSeg;       // byte count of the logical segment or group
} OMFSegMapDesc ;

typedef struct OMFSegMap {
    UInt16  cSeg;        // total number of segment descriptors
    UInt16  cSegLog;     // number of logical segment descriptors
    OMFSegMapDesc   rgDesc[CV_ZEROLEN];   // array of segment descriptors
} OMFSegMap ;

#pragma pack()
#pragma warning(pop)
#endif /* __cvexefmtH__ */
