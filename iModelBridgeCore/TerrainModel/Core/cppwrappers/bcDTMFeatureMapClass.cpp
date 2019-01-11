/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/cppwrappers/bcDTMFeatureMapClass.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifdef notdef

#pragma warning(disable: 4018) 
#pragma warning(disable: 4786) 

/*----------------------------------------------------------------------+
| Include standard library header files                                 |
+----------------------------------------------------------------------*/

#include <math.h>

#include <TerrainModel/TerrainModel.h>

#include <bcDTMBaseDef.h>

/*----------------------------------------------------------------------+
| Include BCivil general header files                                   |
+----------------------------------------------------------------------*/
#include <bcMacros.h>
#include <bcGmcNorm.h>   

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "bcMem.h"
#include "bcDTMImpl.h"

/*------------------------------------------------------------------+
| Local defintions                                                  |
+------------------------------------------------------------------*/

//NOTE: [InRoads] #define DTM_C_NAMSIZ 64
#define FEATUREMAP_NAMESIZE 64

/*==================================================================*/
/*                                                                  */
/*          INTERNAL FUNCTIONS                                      */
/*                                                                  */
/*==================================================================*/

/*----------------------------------------------------------------------+
|                                                                       |
|   twl.04feb2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
void BcDTMFeatureMap::_initialize
(
)
{
    // Empty the _features array;
    _features.clear();
    _nameMap.clear();
}

/*----------------------------------------------------------------------+
|                                                                       |
|   twl.04feb2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
void BcDTMFeatureMap::_generateId
(
    DTMFeatureId *elementIdP
)
{
// ToDo
//bcDTMGuid_generate ( elementIdP );
}

/*----------------------------------------------------------------------+
|                                                                       |
|   twl.04feb2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMFeatureMap::BcDTMFeatureMap
(
)
{
    // Intialize data
    _initialize();
}

/*----------------------------------------------------------------------+
|                                                                       |
|   twl.04feb2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMFeatureMap::~BcDTMFeatureMap
(
)
{
    int bid = 0;
}

/*----------------------------------------------------------------------+
|                                                                       |
|   twl.04feb2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureMap::addFeature
(
    WCharCP nameP,
    WCharCP descriptionP,
    DTMFeatureId   *elementIdP
)
{
    BcDTMFeatureMapItem mapItem;
    NameMapIterator nameMapIt;
    DTMFeatureId elementId;
    wstring name;
    wstring description;
#ifdef _WIN32_WCE
    __w64 index;
#else
    int __w64 index;
#endif

    int sts = DTM_SUCCESS;

    // Generate a new unique identifier if needed.
    if (*elementIdP == DTM_NULL_FEATURE_ID)
        _generateId ( &elementId );
    else
        elementId = *elementIdP;

    if ( nameP )
        name = nameP;

    if ( descriptionP )
        description = descriptionP;

    // Add the new feature to the features array.
    mapItem._name = name;
    mapItem._description = description;
    mapItem._elementId = elementId;
    _features.push_back ( mapItem );

    // Get the index of the new feature.
    index = (int)(_features.end() - _features.begin() - 1);

    // If a name is passed in see if can be found in the name map.
    if ( !name.empty() )
        nameMapIt = _nameMap.find ( name );

    // If it can't be found in the name map, add it.
    if ( !name.empty() && nameMapIt == _nameMap.end() )
    {
        NameMapIdxs idxs;
        _nameMap.insert ( NameMapPair ( name, idxs ) );
        nameMapIt = _nameMap.find ( name );
    }

    // Add the feature index to the list of indexes in the
    // the name map entry.  This accommodates multiple 
    // features with the same name.
    if ( !name.empty() && nameMapIt != _nameMap.end() )
#ifdef _WIN32_WCE
	{
		NameMapPair nmp = *nameMapIt;
		NameMapIdxs nmIdxs = nmp.second;
		nmIdxs.insert(NameMapIdxsPair ( index, index));
	}
#else
        nameMapIt->second.insert ( NameMapIdxsPair ( index, index) );
#endif

    // Add the feature index to the identifier map.
    _identifierMap.insert ( IdentifierMapPair ( elementId, index ) );

    // Return the generated element id.
    if ( elementIdP )
        *elementIdP = elementId;
    
    return sts;
}
/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.31mar2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureMap::setFeatureById
(
    const WCharCP nameP,
    const WCharCP descriptionP,
    DTMFeatureId   *elementIdP
)
{
    IdentifierMapIterator idIt;
    FeaturesIterator ftrIt;
    int sts = DTM_ERROR;

    idIt = _identifierMap.find ( *elementIdP );

    if ( idIt != _identifierMap.end() )
    {
#ifdef _WIN32_WCE
		IdentifierMapPair imp = *idIt;
		__w64 index = imp.second;
#else
        int __w64 index = idIt->second;
#endif
        ftrIt = _features.begin() + index;
        if (ftrIt < _features.end() && nameP != NULL)
        {
            wstring name;
            NameMapIterator nameMapIt;
            // Fisrt delete the previous name from the table
            sts = _deleteNameById (ftrIt->_name.data(), elementIdP);

            // add the name to the name map
            name = nameP;
            if ( !name.empty() )
               nameMapIt = _nameMap.find ( name );

            // If it can't be found in the name map, add it.
            if ( !name.empty() && nameMapIt == _nameMap.end() )
            {
                NameMapIdxs idxs;
                _nameMap.insert ( NameMapPair ( name, idxs ) );
                nameMapIt = _nameMap.find ( name );
            }
            // Add the feature index to the list of indexes in the
            // the name map entry.  This accommodates multiple 
            // features with the same name.
            if ( !name.empty() && nameMapIt != _nameMap.end() )
#ifdef _WIN32_WCE
			{
				NameMapPair nmp = *nameMapIt;
				NameMapIdxs nmIdxs = nmp.second;
				nmIdxs.insert(NameMapIdxsPair ( index, index));
			}
#else
                nameMapIt->second.insert ( NameMapIdxsPair ( index, index) );
#endif
            // update the name
            ftrIt->_name = name;
        }
        if (ftrIt < _features.end() && descriptionP != NULL )
            ftrIt->_description = descriptionP;

        sts = DTM_SUCCESS;
    }

    return sts;
}

/*----------------------------------------------------------------------+
|                                                                       |
|   twl.04feb2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureMap::getFeatureById
(
    wstring         *nameP,
    wstring         *descriptionP,
    DTMFeatureId       *elementIdP
)
{
    IdentifierMapIterator idIt;
    FeaturesIterator ftrIt;
    wstring name;
    int sts = DTM_ERROR;

    if ( elementIdP )
    {
        idIt = _identifierMap.find ( *elementIdP );

        if ( idIt != _identifierMap.end() )
        {
#ifdef _WIN32_WCE
			IdentifierMapPair imp = *idIt;
			__w64 index = imp.second;
#else
            int __w64 index = idIt->second;
#endif
            ftrIt = _features.begin() + index;
            
            if ( ftrIt < _features.end() )
            {                                
                if ( nameP )
                    *nameP = ftrIt->_name;

                if ( descriptionP )
                    *descriptionP = ftrIt->_description;

                sts = DTM_SUCCESS;
            }
        }
    }

    return sts;
}


/*----------------------------------------------------------------------+
|                                                                       |
|   twl.04feb2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureMap::deleteFeatureById
(
    DTMFeatureId   *elementIdP
)
{
    int sts = DTM_SUCCESS;

    if ( elementIdP )
    {
        IdentifierMapIterator idIt = _identifierMap.find ( *elementIdP );

        if ( idIt != _identifierMap.end() )
        {
#ifdef _WIN32_WCE
			IdentifierMapPair imp = *idIt;
		    __w64 index = imp.second;
#else
            int __w64 index = idIt->second;
#endif
            FeaturesIterator ftrIt = _features.begin() + index;
            sts = _deleteNameById (ftrIt->_name.data(), elementIdP);
            _identifierMap.erase ( idIt );
        }
    }

    return sts;
}

/*----------------------------------------------------------------------+
|                                                                       |
|   twl.04feb2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureMap::deleteFeaturesByName
(
    WCharCP nameP
)
{
    int sts = DTM_SUCCESS;

    if ( nameP )
    {
        wstring name = nameP;
        NameMapIterator nameIt = _nameMap.find ( name );

        if ( nameIt != _nameMap.end() )
        {
            NameMapIdxsIterator idxsIt;

#ifdef _WIN32_WCE
			NameMapPair nmp = *nameIt;
			NameMapIdxs nmIdxs = nmp.second;
            for ( idxsIt = nmIdxs.begin(); idxsIt != nmIdxs.end(); idxsIt++ )
#else
            for ( idxsIt = nameIt->second.begin(); idxsIt != nameIt->second.end(); idxsIt++ )
#endif
            {
#ifdef _WIN32_WCE
				NameMapIdxsPair nmip = *idxsIt;
				FeaturesIterator ftrIt = _features.begin() + nmip.second;
#else
                FeaturesIterator ftrIt = _features.begin() + idxsIt->second;                
#endif
                IdentifierMapIterator idIt = _identifierMap.find ( ftrIt->_elementId );

                if ( idIt != _identifierMap.end() )
                    _identifierMap.erase ( idIt );

                _features.erase ( ftrIt );
            }

            _nameMap.erase ( nameIt );
        }
    }

    return sts;
}

/*----------------------------------------------------------------------+
|                                                                       |
|   mah.18oct2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
void BcDTMFeatureMap::UnPackFeatureItems
(
    int*  nBytes,
    void* byteP
)
{
    //NOTE: [InRoads] #define DTM_C_NAMSIZ 64

    int nSizeOfFlag = sizeof( byte );
    int nSizeOfGuid = sizeof( DTMFeatureId );
    int nSizeOfName = FEATUREMAP_NAMESIZE * sizeof( wchar_t );
    int nSizeOfDesc = FEATUREMAP_NAMESIZE * sizeof( wchar_t );
    int nSizeOfItem = nSizeOfFlag + nSizeOfGuid + nSizeOfName + nSizeOfDesc;

    if( ! nBytes || *nBytes == 0 || ( *nBytes % nSizeOfItem != 0 ) )
        return;

    int nByte = 0;
    byte* byteArrayP = (byte *)byteP;

    wchar_t name[FEATUREMAP_NAMESIZE] = L"";
    wchar_t desc[FEATUREMAP_NAMESIZE] = L"";
    DTMFeatureId guid;
    byte flag;

    while( nByte < *nBytes )
    {
        memset( name, 0, sizeof( name ) );
        memset( desc, 0, sizeof( desc ) );

        if( nByte + nSizeOfName < *nBytes )
        {
            memcpy( name, &byteArrayP[nByte], nSizeOfName );
            nByte += nSizeOfName;
        }
        else
            break;

        if( nByte + nSizeOfDesc < *nBytes )
        {
            memcpy( desc, &byteArrayP[nByte], nSizeOfDesc );
            nByte += nSizeOfDesc;
        }
        else
            break;

        if( nByte + nSizeOfGuid < *nBytes )
        {
            memcpy( &guid, &byteArrayP[nByte], nSizeOfGuid );
            nByte += nSizeOfGuid;
        }
        else
            break;

        if( nByte + nSizeOfFlag < *nBytes )
        {
            memcpy( &flag, &byteArrayP[nByte], nSizeOfFlag );
            nByte += nSizeOfFlag;
        }
        else
            break;

        addFeature( name, desc, &guid );
    }
}

/*----------------------------------------------------------------------+
|                                                                       |
|   mah.18oct2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
void BcDTMFeatureMap::PackFeatureItems
(
    int*   nBytes,
    void** bytePP
)
{
    //NOTE: [InRoads] #define DTM_C_NAMSIZ 64

    int nFeatureItems = (int)_features.size();

    if( nFeatureItems )
    {
        int nNameSize = FEATUREMAP_NAMESIZE * sizeof( wchar_t );
        int nDescSize = FEATUREMAP_NAMESIZE * sizeof( wchar_t );
        int nGuidSize = sizeof( DTMFeatureId );
        int nByteSize = sizeof( byte );
        int nItemSize = nNameSize + nDescSize + nGuidSize + nByteSize;

        byte* byteP = (byte *)malloc( nFeatureItems * nItemSize );

        if( byteP )
        {
            for( int i = 0; i < nFeatureItems; i++ )
            {
                const BcDTMFeatureMapItem &ftrItem = _features[ i ];

                int nByteOffset = 0;

                wchar_t name[FEATUREMAP_NAMESIZE] = L"";
                wcscpy( name, ftrItem._name.data() );
                memcpy( &byteP[i*nItemSize+nByteOffset], name, nNameSize );
                nByteOffset += nNameSize;

                wchar_t desc[FEATUREMAP_NAMESIZE] = L"";
                wcscpy( desc, ftrItem._description.data() );
                memcpy( &byteP[i*nItemSize+nByteOffset], desc, nDescSize );
                nByteOffset += nDescSize;

                memcpy( &byteP[i*nItemSize+nByteOffset], &ftrItem._elementId, nGuidSize );
                nByteOffset += nGuidSize;

                memcpy( &byteP[i*nItemSize+nByteOffset], &ftrItem._flg, nByteSize );
                nByteOffset += nByteSize;
            }

            *bytePP = byteP;
            *nBytes = nFeatureItems * nItemSize;
        }
    }
}

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.31mar2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureMap::_deleteNameById 
(
     const WCharCP nameP,
     DTMFeatureId  *elementIdP
)
{
    int sts = DTM_SUCCESS;
    IdentifierMapIterator idIt = _identifierMap.find ( *elementIdP );

    if (idIt == _identifierMap.end()) return DTM_ERROR;

#ifdef _WIN32_WCE
	IdentifierMapPair imp = *idIt;
	__w64 index = imp.second;
#else
    int __w64 index = idIt->second;
#endif
    FeaturesIterator ftrIt = _features.begin() + index;
    NameMapIterator nameIt = _nameMap.find (nameP );
    if ( nameIt != _nameMap.end() )
    {
#ifdef _WIN32_WCE
		NameMapPair nmp = *nameIt;
		NameMapIdxs nmIdxs = nmp.second;
		NameMapIdxsIterator idxsIt = nmIdxs.find(index);
#else
        NameMapIdxsIterator idxsIt = nameIt->second.find ( index );
#endif
#ifdef _WIN32_WCE
        if ( idxsIt != nmIdxs.end())
            nmIdxs.erase ( idxsIt );
#else
        if ( idxsIt != nameIt->second.end())
            nameIt->second.erase ( idxsIt );
#endif

#ifdef _WIN32_WCE
        if (nmIdxs.size() == 0)
            _nameMap.erase ( nameIt );
#else
        if (nameIt->second.size() == 0)
            _nameMap.erase ( nameIt );
#endif
    }
    return sts;
}

/*----------------------------------------------------------------------+
|                                                                       |
|   twl.04feb2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMFeatureMapItem::BcDTMFeatureMapItem 
(
)
{
    // Intialize data
#if (_MSC_VER < 1300)
#else
    _name.clear();
    _description.clear();
#endif
    memset ( &_elementId, 0, sizeof ( _elementId ) );
    _flg = 0;
}
#endif