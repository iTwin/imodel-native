/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/nonport/winnt/gdileak.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <map>
#include <vector>
#include <RmgrTools/Tools/pagstruc.h>

int32_t   pagallocI_recordCallingStack (PageMallocEntry *headerP, void* returnAddress);

std::map<uintptr_t, PageMallocEntry const*> g_HandleInfo;

void AddEntry (uintptr_t handleValue, void* returnAddress);
void RemoveEntry (uintptr_t handleValue, bool bReturn);

#define TRACE_CREATE_CALL(handleValue, returnAddress)\
        AddEntry ((uintptr_t)handleValue, returnAddress); \
        return handleValue;

#define DELETE_TRACE(handleValue)\
        RemoveEntry ((uintptr_t)handleValue, (bReturn?true:false)); \
        return bReturn;

#define DECLARE_CREATE_API1(Name, ResultType, vType)\
        typedef ResultType (WINAPI *Name##Proc)(vType);\
        static Name##Proc __##Name;\
        static ResultType WINAPI _##Name(vType x)\
            {\
            ResultType hVar##ResultType = __##Name(x);\
            TRACE_CREATE_CALL(hVar##ResultType, CGdiMonitor::_##Name);\
            }


#define DECLARE_CREATE_API2(Name, ResultType, vType1, vType2)\
        typedef ResultType (WINAPI *Name##Proc)(vType1, vType2);\
        static Name##Proc __##Name;\
        static ResultType WINAPI _##Name(vType1 x1, vType2 x2)\
            {\
            ResultType hVar##ResultType = __##Name(x1, x2);\
            TRACE_CREATE_CALL(hVar##ResultType, CGdiMonitor::_##Name);\
            }

#define DECLARE_CREATE_API3(Name, ResultType, vType1, vType2, vType3) \
        typedef ResultType (WINAPI *Name##Proc)(vType1, vType2, vType3);\
        static Name##Proc __##Name;\
        static ResultType WINAPI _##Name(vType1 x1, vType2 x2, vType3 x3)\
            {\
            ResultType hVar##ResultType = __##Name(x1, x2, x3);\
            TRACE_CREATE_CALL(hVar##ResultType, CGdiMonitor::_##Name);\
            }

#define DECLARE_CREATE_API4(Name, ResultType, vType1, vType2, vType3, vType4)\
        typedef ResultType (WINAPI *Name##Proc)(vType1, vType2, vType3, vType4);\
        static Name##Proc __##Name;\
        static ResultType WINAPI _##Name(vType1 x1, vType2 x2, vType3 x3, vType4 x4)\
            {\
            ResultType hVar##ResultType = __##Name(x1, x2, x3, x4);\
            TRACE_CREATE_CALL(hVar##ResultType, CGdiMonitor::_##Name);\
            }

#define DECLARE_CREATE_API5(Name, ResultType, vType1, vType2, vType3, vType4, vType5)\
        typedef ResultType (WINAPI *Name##Proc)(vType1, vType2, vType3, vType4, vType5);\
        static Name##Proc __##Name;\
        static ResultType WINAPI _##Name(vType1 x1, vType2 x2, vType3 x3, vType4 x4, vType5 x5)\
            {\
            ResultType hVar##ResultType = __##Name(x1, x2, x3, x4, x5);\
            TRACE_CREATE_CALL(hVar##ResultType, CGdiMonitor::_##Name);\
            }

#define DECLARE_CREATE_API6(Name, ResultType, vType1, vType2, vType3, vType4, vType5, vType6)\
        typedef ResultType (WINAPI *Name##Proc)(vType1, vType2, vType3, vType4, vType5, vType6);\
        static Name##Proc __##Name;\
        static ResultType WINAPI _##Name(vType1 x1, vType2 x2, vType3 x3, vType4 x4, vType5 x5, vType6 x6)\
            {\
            ResultType hVar##ResultType = __##Name(x1, x2, x3, x4, x5, x6);\
            TRACE_CREATE_CALL(hVar##ResultType, CGdiMonitor::_##Name);\
            }

#define DECLARE_CREATE_API7(Name, ResultType, vType1, vType2, vType3, vType4, vType5, vType6, vType7)\
        typedef ResultType (WINAPI *Name##Proc)(vType1, vType2, vType3, vType4, vType5, vType6, vType7);\
        static Name##Proc __##Name;\
        static ResultType WINAPI _##Name(vType1 x1, vType2 x2, vType3 x3, vType4 x4, vType5 x5, vType6 x6, vType7 x7)\
            {\
            ResultType hVar##ResultType = __##Name(x1, x2, x3, x4, x5, x6, x7);\
            TRACE_CREATE_CALL(hVar##ResultType, CGdiMonitor::_##Name);\
            }

#define DECLARE_CREATE_API14(Name, ResultType, vType1, vType2, vType3, vType4, vType5, vType6,\
        vType7, vType8, vType9, vType10, vType11, vType12, vType13, vType14)\
        typedef ResultType (WINAPI *Name##Proc)(\
                vType1, vType2, vType3, vType4, vType5, vType6,\
                vType7, vType8, vType9, vType10, vType11, vType12, vType13, vType14);\
        static Name##Proc __##Name;\
        static ResultType WINAPI _##Name(\
                vType1 x1, vType2 x2, vType3 x3, vType4 x4, vType5 x5, vType6 x6,\
                vType7 x7, vType8 x8, vType9 x9, vType10 x10, vType11 x11, vType12 x12,\
                vType13 x13, vType14 x14)\
            {\
            ResultType hVar##ResultType = __##Name(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14);\
            TRACE_CREATE_CALL(hVar##ResultType, CGdiMonitor::_##Name);\
            }

#define DECLARE_DELETE_API(Name, vType)\
        typedef BOOL (WINAPI *Name##Proc)(vType);\
        static Name##Proc __##Name;\
        static BOOL WINAPI _##Name(vType x)\
            {\
            BOOL bReturn = __##Name(x);\
            DELETE_TRACE(x);\
            }

#define DECLARE_RELEASE_API(Name, vType1, vType2)\
        typedef int (WINAPI *Name##Proc)(vType1, vType2);\
        static Name##Proc __##Name;\
        static int WINAPI _##Name(vType1 x1, vType2 x2)\
            {\
            int bReturn = __##Name(x1, x2);\
            DELETE_TRACE(x2);\
            }

#define DECLARE_CLOSE_API(Name, ResultType)\
        typedef ResultType (WINAPI *Name##Proc)(HDC);\
        static Name##Proc __##Name;\
        static ResultType WINAPI _##Name(HDC x)\
            {\
            ResultType handleValue = __##Name(x);\
            if(handleValue != NULL)\
                {\
                delete g_HandleInfo[(uintptr_t)x]; \
                g_HandleInfo.erase((uintptr_t)x);\
                PageMallocEntry *entry = new PageMallocEntry; \
                pagallocI_recordCallingStack (entry, _ReturnAddress()); \
                g_HandleInfo[(uintptr_t)handleValue] = entry; \
                }\
            return handleValue;\
            }

class CGdiMonitor
    {
public:
    CGdiMonitor(void);
    ~CGdiMonitor(void);
    // device context
    DECLARE_CREATE_API4(CreateDCA, HDC, LPCSTR, LPCSTR, LPCSTR, CONST DEVMODE*)
    DECLARE_CREATE_API4(CreateDCW, HDC, LPCWSTR, LPCWSTR, LPCWSTR, CONST DEVMODE*)
    DECLARE_CREATE_API1(CreateCompatibleDC, HDC, HDC)
    DECLARE_CREATE_API4(CreateICA, HDC, LPCSTR, LPCSTR, LPCSTR, CONST DEVMODE*)
    DECLARE_CREATE_API4(CreateICW, HDC, LPCWSTR, LPCWSTR, LPCWSTR, CONST DEVMODE*)
    DECLARE_CREATE_API1(GetDC, HDC, HWND)//(USER32.dll)
    DECLARE_CREATE_API3(GetDCEx, HDC, HWND, HRGN, DWORD)//(USER32.dll)
    DECLARE_CREATE_API1(GetWindowDC, HDC, HWND)//(USER32.dll)
    // pen
    DECLARE_CREATE_API3(CreatePen, HPEN, int, int, COLORREF)
    DECLARE_CREATE_API1(CreatePenIndirect, HPEN, CONST LOGPEN*)
    DECLARE_CREATE_API5(ExtCreatePen, HPEN, DWORD, DWORD, CONST LOGBRUSH*, DWORD, CONST DWORD*)
    // brush API
    DECLARE_CREATE_API1(CreateSolidBrush, HBRUSH, COLORREF)
    DECLARE_CREATE_API2(CreateHatchBrush, HBRUSH, int, COLORREF)
    DECLARE_CREATE_API1(CreateBrushIndirect, HBRUSH, CONST LOGBRUSH*)
    DECLARE_CREATE_API1(CreatePatternBrush, HBRUSH, HBITMAP)
    DECLARE_CREATE_API2(CreateDIBPatternBrush, HBRUSH, HGLOBAL, UINT)
    DECLARE_CREATE_API2(CreateDIBPatternBrushPt, HBRUSH, CONST VOID*, UINT)
    // bitmap API
    DECLARE_CREATE_API2(LoadBitmapA, HBITMAP, HINSTANCE, LPSTR)//(USER32.dll)
    DECLARE_CREATE_API2(LoadBitmapW, HBITMAP, HINSTANCE, LPWSTR)//(USER32.dll)
    DECLARE_CREATE_API5(CreateBitmap, HBITMAP, int, int, UINT, UINT, CONST VOID*)
    DECLARE_CREATE_API1(CreateBitmapIndirect, HBITMAP, CONST BITMAP*)
    DECLARE_CREATE_API3(CreateCompatibleBitmap, HBITMAP, HDC, int, int)
    DECLARE_CREATE_API6(CreateDIBitmap, HBITMAP, HDC, CONST BITMAPINFOHEADER *, DWORD, CONST VOID *, CONST BITMAPINFO *, UINT)
    DECLARE_CREATE_API6(CreateDIBSection, HBITMAP, HDC, CONST BITMAPINFO*, UINT, VOID **, HANDLE, DWORD)
    DECLARE_CREATE_API3(CreateDiscardableBitmap, HBITMAP, HDC, INT, INT)
    // font
    DECLARE_CREATE_API14(CreateFontA, HFONT, int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCSTR)
    DECLARE_CREATE_API14(CreateFontW, HFONT, int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCWSTR)
    DECLARE_CREATE_API1(CreateFontIndirectA, HFONT, CONST LOGFONTA*)
    DECLARE_CREATE_API1(CreateFontIndirectW, HFONT, CONST LOGFONTW*)
    DECLARE_CREATE_API1(CreateFontIndirectExA, HFONT, CONST ENUMLOGFONTEXDVA*)
    DECLARE_CREATE_API1(CreateFontIndirectExW, HFONT, CONST ENUMLOGFONTEXDVW*)
    // region
    DECLARE_CREATE_API4(CreateRectRgn, HRGN, int, int, int, int)
    DECLARE_CREATE_API1(CreateRectRgnIndirect, HRGN, CONST RECT*)
    DECLARE_CREATE_API4(CreateEllipticRgn, HRGN, int, int, int, int)
    DECLARE_CREATE_API1(CreateEllipticRgnIndirect, HRGN, CONST RECT*)
    DECLARE_CREATE_API3(CreatePolygonRgn, HRGN, CONST POINT*, int, int)
    DECLARE_CREATE_API4(CreatePolyPolygonRgn, HRGN, CONST POINT*, CONST INT*, int, int)
    DECLARE_CREATE_API6(CreateRoundRectRgn, HRGN, int, int, int, int, int, int)
    DECLARE_CREATE_API1(PathToRegion, HRGN, HDC)
    DECLARE_CREATE_API3(ExtCreateRegion, HRGN, CONST XFORM*, DWORD, CONST RGNDATA*)
    DECLARE_CREATE_API4(CombineRgn, INT, HRGN, HRGN, HRGN, INT)
    // metafile dc(released by CloseMetaFile/CloseEnhMetaFile)
    DECLARE_CREATE_API1(CreateMetaFileA, HDC, LPCSTR)
    DECLARE_CREATE_API1(CreateMetaFileW, HDC, LPCWSTR)
    DECLARE_CREATE_API4(CreateEnhMetaFileA, HDC, HDC, LPCSTR, CONST RECT*, LPCSTR)
    DECLARE_CREATE_API4(CreateEnhMetaFileW, HDC, HDC, LPCWSTR, CONST RECT*, LPCWSTR)
    // metafile
    DECLARE_CREATE_API1(GetEnhMetaFileA, HENHMETAFILE, LPCSTR)
    DECLARE_CREATE_API1(GetEnhMetaFileW, HENHMETAFILE, LPCWSTR)
    DECLARE_CREATE_API1(GetMetaFileA, HMETAFILE, LPCSTR)
    DECLARE_CREATE_API1(GetMetaFileW, HMETAFILE, LPCWSTR)
    // palette
    DECLARE_CREATE_API1(CreateHalftonePalette, HPALETTE, HDC)
    DECLARE_CREATE_API1(CreatePalette, HPALETTE, CONST LOGPALETTE*)
    // icons
    DECLARE_CREATE_API1(CopyIcon, HICON, HICON)
    DECLARE_CREATE_API7(CreateIcon, HICON, HINSTANCE, int, int, BYTE, BYTE, CONST BYTE*, CONST BYTE*)
    DECLARE_CREATE_API4(CreateIconFromResource, HICON, PBYTE, DWORD, BOOL, DWORD)
    DECLARE_CREATE_API7(CreateIconFromResourceEx, HICON, PBYTE, DWORD, BOOL, DWORD, int, int, UINT)
    DECLARE_CREATE_API1(CreateIconIndirect, HICON, PICONINFO)
    DECLARE_CREATE_API2(DuplicateIcon, HICON, HINSTANCE, HICON)
    DECLARE_CREATE_API3(ExtractAssociatedIconA, HICON, HINSTANCE, LPCSTR, WORD*)
    DECLARE_CREATE_API3(ExtractAssociatedIconW, HICON, HINSTANCE, LPWSTR, WORD*)
    DECLARE_CREATE_API4(ExtractAssociatedIconExA, HICON, HINSTANCE, LPCSTR, LPWORD, LPWORD)
    DECLARE_CREATE_API4(ExtractAssociatedIconExW, HICON, HINSTANCE, LPWSTR, LPWORD, LPWORD)
    DECLARE_CREATE_API3(ExtractIconA, HICON, HINSTANCE, LPCSTR, UINT)
    DECLARE_CREATE_API3(ExtractIconW, HICON, HINSTANCE, LPWSTR, UINT)
//    DECLARE_CREATE_API1(ExtractIconExA, HICON) // Not sure how to track these; return 2 icons in arguments.
//    DECLARE_CREATE_API1(ExtractIconExW, HICON)
    DECLARE_CREATE_API2(LoadIconA, HICON, HINSTANCE, LPCSTR)
    DECLARE_CREATE_API2(LoadIconW, HICON, HINSTANCE, LPWSTR)
//    DECLARE_CREATE_API1(PrivateExtractIconsA, HICON, HDC) // Optional return in 5th argument
//    DECLARE_CREATE_API1(PrivateExtractIconsW, HICON, HDC)
    // object deletion
    DECLARE_DELETE_API(DeleteObject, HGDIOBJ)
    DECLARE_DELETE_API(DeleteDC, HDC)
    DECLARE_DELETE_API(DeleteMetaFile, HMETAFILE)
    DECLARE_DELETE_API(DeleteEnhMetaFile, HENHMETAFILE)
    DECLARE_DELETE_API(DestroyIcon, HICON)
    // object release
    DECLARE_RELEASE_API(ReleaseDC, HWND, HDC)//(USER32.dll)
    //delete metafile dc and generate metafile
    DECLARE_CLOSE_API(CloseMetaFile, HMETAFILE)
    DECLARE_CLOSE_API(CloseEnhMetaFile, HENHMETAFILE)
    };
