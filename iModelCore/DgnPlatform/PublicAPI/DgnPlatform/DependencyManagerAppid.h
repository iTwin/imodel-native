/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DependencyManagerAppid.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

/*----------------------------------------------------------------------+
                                                                       |
   Registry of DependencyLinkage appIDs

        A DependencyLinkage appID is a short integer value that
        uniquely identifies a type of DependencyLinkage and serves
        as the key for registering a post-processing callback for
        that type of linkage.

        All appIDs must be registered in this file, in order to prevent
        developers from independently reserving the same appID value
        to designate different dependency linkage types.
                                                                       |
+----------------------------------------------------------------------*/

// NOTE: Zero is an unused APPID
// NOTE: Values between 1 and 100011 are reserved for uStn. Values above 10011 are for 3rd party use
//       Bentley internal ids count DOWN from 10011
#define DEPENDENCYAPPID_TestDepend                      1   // testdepend shows usage of mdlDependency API
                                                            // }
                                                            // } REMAINING INTERNAL BENTLEY IDS TO BE ALLOCATED
                                                            // }
#define DEPENDENCYAPPID_NextBentleyInternal___       9992   // <===== Next internal Dependency APPID ===========
#define DEPENCENCYAPPID_TextField                    9993   // text fields
#define DEPENDENCYAPPID_DictionaryEntry              9994   // for dictionary entries
#define DEPENDENCYAPPID_Relationship                 9995   // a relationship between elements - associated with an assembly manager
#define DEPENDENCYAPPID_Note                         9996   // for note annotations
#define DEPENDENCYAPPID_DgnLinks                     9997   // design links
#define DEPENDENCYAPPID_NamedGroup                   9998   // for named groups
#define DEPENDENCYAPPID_DigitalSignature             9999   // for digital signature element
#define DEPENDENCYAPPID_MicroStation                10000   // MicroStation dependency linkage
#define DEPENDENCYAPPID_Modeler                     10001   // Modeler references to other elements
#define DEPENDENCYAPPID_AssocRegion                 10002   // Associative Regions dependency application ID
#define DEPENDENCYAPPID_DimensionCell               10003   // shared cell def used for terminator/prefix/suffix
#define DEPENDENCYAPPID_PatternCell                 10004   // reference to pattern shared cell def
#define DEPENDENCYAPPID_ModelerTabDim               10005   // Modeler Table dimension references between table cells and origin.
#define DEPENDENCYAPPID_SharedCellDef               10006   // reference to shared cell definition
#define DEPENDENCYAPPID_TagSetDef                   10007   // reference to tag set definiion
#define DEPENDENCYAPPID_ModelerAssociatedElement    10008   // Elements associated to modeler tree
#define DEPENDENCYAPPID_Text                        10009   // Text elements associated with linear elements
#define DEPENDENCYAPPID_ATFSpaces                   10010   // reference to ATF spaces
#define DEPENDENCYAPPID_Raster                      10011   // For raster graphic element extension
//      END OF BENTLEY INTERNAL IDS                 XXXXX
#define DEPENDENCYAPPID_First3rdParty___            10012   // <====== First third party Dependency APPID ============
                                                            // }
                                                            // } 3rd PARTY IDS ALLOCATED BY BENTLEY SELECT
                                                            // }
#define DEPENDENCYAPPID_ATFDGAnnotation             48770   // For reference to ATFDGAnnotation

/*----------------------------------------------------------------------+
|   App Values - DEPENDENCYAPPID_MicroStation                           |
+----------------------------------------------------------------------*/
#define DEPENDENCYAPPVALUE_AssocPoint               1       // appValue type for a associative point
#define DEPENDENCYAPPVALUE_ClipElement              2       // clip element
#define DEPENDENCYAPPVALUE_BrepData                 3       // linkage references external solids data
#define DEPENDENCYAPPVALUE_FarReference             4       // far association element
#define DEPENDENCYAPPVALUE_ExternalTextFile         5       // A dependency for a text element or text node on an external text file
#define DEPENDENCYAPPVALUE_ReferenceFrozenLevel     6       // Frozen layers table entry unique IDs (from AutoCAD Viewports).
#define DEPENDENCYAPPVALUE_NestedTagInstance        7       // nested shared cell instance tag associated to.
#define DEPENDENCYAPPVALUE_BrepPartition            8       // external solids partition data
#define DEPENDENCYAPPVALUE_ADTMultiViewBlockCell    9       // Architectural desktop multi-view block definition.
#define DEPENDENCYAPPVALUE_CrossReference           10      // link 2 references
#define DEPENDENCYAPPVALUE_ClipMaskElement          11      // clip mask element
#define DEPENDENCYAPPVALUE_DictionaryEntry          12      // dictionary entry
#define DEPENDENCYAPPVALUE_OriginalClipElement      13
#define DEPENDENCYAPPVALUE_OriginalClipMaskElement  14
#define DEPENDENCYAPPVALUE_BorderAttachment         15      // Associate an attachment to a sheet definition - self dependency
#define DEPENDENCYAPPVALUE_ReferenceSavedView       16      // Saved view element from which reference was derived.
#define DEPENDENCYAPPVALUE_XGraphicsSymbol          17

/*----------------------------------------------------------------------+
|   App Values - DEPENDENCYAPPID_Modeler                                |
+----------------------------------------------------------------------*/
#define DEPENDENCYAPPVALUE_ModelerProfile           1       // appValue type for a Modeler profile
#define DEPENDENCYAPPVALUE_ModelerACS               2       // ACS element referred to by Modeler elements

/*----------------------------------------------------------------------+
|   App Values - DEPENDENCYAPPID_DimensionCell                          |
+----------------------------------------------------------------------*/
//The dimension code needs to updated if this value is modified
#define DEPENDENCYAPPVALUE_ArrowHeadTerminator      1
#define DEPENDENCYAPPVALUE_StrokeTerminator         2
#define DEPENDENCYAPPVALUE_OriginTerminator         3
#define DEPENDENCYAPPVALUE_DotTerminator            4
#define DEPENDENCYAPPVALUE_PrefixCell               5
#define DEPENDENCYAPPVALUE_SuffixCell               6
#define DEPENDENCYAPPVALUE_ProxyCell                7
#define DEPENDENCYAPPVALUE_NoteTerminator           8

/*----------------------------------------------------------------------+
|   App Values - DEPENDENCYAPPID_ModelerTabDim                          |
+----------------------------------------------------------------------*/
#define DEPENDENCYAPPVALUE_Origin           1
#define DEPENDENCYAPPVALUE_Entry            2
#define DEPENDENCYAPPVALUE_Table            3

/*----------------------------------------------------------------------+
|   App Values - DEPENDENCYAPPID_Note                                   |
+----------------------------------------------------------------------*/
#define DEPENDENCYAPPVALUE_Dimension                1

/** @endcond */
