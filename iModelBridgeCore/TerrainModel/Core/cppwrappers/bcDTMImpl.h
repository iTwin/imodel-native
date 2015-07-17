/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/cppwrappers/bcDTMImpl.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifndef __bcDTMImplH__
#define __bcDTMImplH__

/*----------------------------------------------------------------------+
| Include standard library header files                                 |
+----------------------------------------------------------------------*/
#include <string>

#ifdef _WIN32_WCE
typedef unsigned int __w64;
#endif

#if (_MSC_VER < 1300)
typedef unsigned int __w64;
#endif

/*----------------------------------------------------------------------+
| Include BCivil general header files                                   |
+----------------------------------------------------------------------*/
#include <TerrainModel/TerrainModel.h>
#include	<bcDTMClass.h>
#include	<bcMem.h>

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

#define  PRFL_NO_FEATURE                  (0L)

inline void Copy(DPoint3d** destPP, DPoint3d const *srcP, long nbPt)
    {
    *destPP = (DPoint3d*)bcMem_malloc (nbPt*sizeof(DPoint3d));
    memcpy (*destPP, srcP, nbPt*sizeof(DPoint3d));
    }
END_BENTLEY_TERRAINMODEL_NAMESPACE
#endif	/* #ifndef __bcDTMImplH__ */
