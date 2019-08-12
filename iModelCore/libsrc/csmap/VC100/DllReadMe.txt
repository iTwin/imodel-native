A new solution file has been added, CsMapDll.prj, which will build a
Windows DLL (32 or 64 bit per VC++ configuration) including much of CS_MAP
coordinate conversion functionality.

Please note:

1> The C Run Time Library is statically linked into the DLL.  This is an attempt
to make the DLL somewhat generic.  Obviously, if this is inappropriate, it is
easy enough to change in the Property Sheets.  The DLL does not reference any
other DLL (like MFC, OLE, ATL) other than the standard Windows stuff such as
kernel.dll.

2> Use an environmental variable named CS_MAP_DIR to provide the location of
the Dictionary directory which is to be referenced.  If the DLL can't
find a suitable Dictionary directory, the DLL will still load, but its
use will probably crash.

3> For 'C' and/or 'C++' applications, a header named cs_mapDll.h is provided
in the Include directory.  For VBA applications, a file named Header.vba in the
Include directory contains all the declarations which you might need.

4> There is a 'C' console test application (CS_DllTest.prj) included in the
solution.

5> Testing has been done only in the 'C', and VBA environments.

6> Some minor code changes to library code was required to get this to compile
and work.  These changes involve the use of EXP_LVL? defines which are defined
as a NULL string unless the DLL_32 or DLL_64 preprocessor variables are
defined.  These were not defined in the traditional OpenSource.sln file and
the associated project files.  Thus, these code changes should not have
any affect on any previous builds and/or applications.
