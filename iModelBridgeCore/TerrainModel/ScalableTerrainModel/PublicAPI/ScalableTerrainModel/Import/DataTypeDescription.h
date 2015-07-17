/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/DataTypeDescription.h $
|    $RCSfile: DataTypeDescription.h,v $
|   $Revision: 1.7 $
|       $Date: 2012/03/21 18:37:10 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>


BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimensionType
    {
    enum ID
        {
        ID_VOID,
        ID_UINT8,
        ID_UINT16,
        ID_UINT32,
        ID_FLOAT32,
        ID_FLOAT64,
        /* If adding new type, update s_TypeSize as well */

        ID_QTY,
        ID_UNKNOWN = ID_VOID,
        ID_CUSTOM = 0xFF,
        };

    struct Register : private Uncopyable
        {
        const WChar*                         m_id;
        IMPORT_DLLE explicit                    Register                       (const WChar*                  name,
                                                                                size_t                          size);
        IMPORT_DLLE                             ~Register                      ();
        };

    struct Info;
private:
    ID                                          m_id;
    const Info*                                 m_infoP;     

    
    explicit                                    DimensionType                  (ID                              id,
                                                                                const Info&                     info);

    IMPORT_DLLE bool                            EqualTo                        (const DimensionType&            rhs) const;
    IMPORT_DLLE bool                            LessThan                       (const DimensionType&            rhs) const;

public:

    IMPORT_DLLE static const DimensionType&     GetVoid                        ();
    IMPORT_DLLE static const DimensionType&     GetUInt8                       ();
    IMPORT_DLLE static const DimensionType&     GetUInt16                      ();
    IMPORT_DLLE static const DimensionType&     GetUInt32                      ();
    IMPORT_DLLE static const DimensionType&     GetFloat32                     ();
    IMPORT_DLLE static const DimensionType&     GetFloat64                     ();


    IMPORT_DLLE static const DimensionType&     GetUnknown                     ();

    IMPORT_DLLE static const DimensionType&     GetFor                         (ID                              id);
    IMPORT_DLLE static DimensionType            GetFor                         (const WChar*                  name);


    // Use default copy behavior

    ID                                          GetID                          () const { return m_id; } 

    bool                                        IsCustom                       () const { return ID_CUSTOM == m_id; }
    bool                                        IsNative                       () const { return ID_QTY > m_id; }

    IMPORT_DLLE const WString&               GetName                        () const;
    IMPORT_DLLE const WChar*                 GetNameCStr                    () const;

    IMPORT_DLLE size_t                          GetSize                        () const;

    friend bool                                 operator==                     (const DimensionType&            lhs,
                                                                                const DimensionType&            rhs);
    
    friend bool                                 operator<                      (const DimensionType&            lhs,
                                                                                const DimensionType&            rhs);
    };


// TDORAY: We should consider creating a class for dimension role so that custom roles can be registered 
//         by the user and that these roles be comparable via a String. See DimensionType for
//         an example of how it can be achieved.
typedef UInt                                    DimensionRole;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimensionDef
    {
public:
    IMPORT_DLLE explicit                DimensionDef                   ();
    
    IMPORT_DLLE explicit                DimensionDef                   (const DimensionType&            type);

    IMPORT_DLLE explicit                DimensionDef                   (const DimensionType&            type, 
                                                                        DimensionRole                   role);

    IMPORT_DLLE                         ~DimensionDef                  ();

    IMPORT_DLLE const DimensionType&    GetType                        () const;
    IMPORT_DLLE DimensionRole           GetRole                        () const;

    IMPORT_DLLE size_t                  GetTypeSize                    () const;


    IMPORT_DLLE void                    SetType                        (const DimensionType&            type);
    IMPORT_DLLE void                    SetRole                        (DimensionRole                   role);

    IMPORT_DLLE bool                    IsComplete                     () const;
    IMPORT_DLLE bool                    IsNative                       () const;

private:
    DimensionType                       m_type;
    DimensionRole                       m_role;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*    
* TDORAY: May benefits from the same global ID treatment then point type. See DimensionsConverter.
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimensionOrg
    {
private:
    struct Impl;
    std::auto_ptr<Impl>                 m_implP;

public:
    typedef const DimensionDef*         const_iterator;
    typedef DimensionDef*               iterator;

    IMPORT_DLLE explicit                DimensionOrg                   (size_t                          capacity = 0);


    IMPORT_DLLE explicit                DimensionOrg                   (const DimensionDef&             d0);

    IMPORT_DLLE explicit                DimensionOrg                   (const DimensionDef&             d0,
                                                                        const DimensionDef&             d1);

    IMPORT_DLLE explicit                DimensionOrg                   (const DimensionDef&             d0,
                                                                        const DimensionDef&             d1,
                                                                        const DimensionDef&             d2);

    IMPORT_DLLE explicit                DimensionOrg                   (const DimensionDef&             d0,
                                                                        const DimensionDef&             d1,
                                                                        const DimensionDef&             d2,
                                                                        const DimensionDef&             d3);

    IMPORT_DLLE explicit                DimensionOrg                   (const DimensionDef&             d0,
                                                                        const DimensionDef&             d1,
                                                                        const DimensionDef&             d2,
                                                                        const DimensionDef&             d3,
                                                                        const DimensionDef&             d4);
    // TDORAY: Would profits using C++0x initializer list cstor



    IMPORT_DLLE explicit                DimensionOrg                   (const_iterator                  begin, 
                                                                        const_iterator                  end);

    IMPORT_DLLE                         ~DimensionOrg                  ();


    IMPORT_DLLE                         DimensionOrg                   (const DimensionOrg&             rhs);
    IMPORT_DLLE DimensionOrg&           operator=                      (const DimensionOrg&             rhs);

    IMPORT_DLLE DimensionDef&           operator[]                     (size_t                          idx);
    IMPORT_DLLE const DimensionDef&     operator[]                     (size_t                          idx) const;

    IMPORT_DLLE const_iterator          cbegin                         () const;
    IMPORT_DLLE const_iterator          cend                           () const;

    IMPORT_DLLE const_iterator          begin                          () const;
    IMPORT_DLLE const_iterator          end                            () const;

    IMPORT_DLLE iterator                begin                          ();
    IMPORT_DLLE iterator                end                            ();

    IMPORT_DLLE void                    push_back                      (const DimensionDef&             dimension);

    IMPORT_DLLE size_t                  GetSize                        () const;
    IMPORT_DLLE size_t                  GetTypeSize                    () const;

    IMPORT_DLLE bool                    IsPOD                          () const;
    IMPORT_DLLE bool                    IsComplete                     () const;

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimensionOrgGroup
    {
private:
    struct Impl;
    std::auto_ptr<Impl>                 m_implP;

public:
    typedef const DimensionOrg*         const_iterator;
    typedef DimensionOrg*               iterator;

    IMPORT_DLLE explicit                DimensionOrgGroup              (size_t                          capacity = 0);

    IMPORT_DLLE explicit                DimensionOrgGroup              (const_iterator                  begin, 
                                                                        const_iterator                  end);


    IMPORT_DLLE                         ~DimensionOrgGroup             ();


    IMPORT_DLLE                         DimensionOrgGroup              (const DimensionOrgGroup&        rhs);
    IMPORT_DLLE DimensionOrgGroup&      operator=                      (const DimensionOrgGroup&        rhs);


    IMPORT_DLLE DimensionOrg&           operator[]                     (size_t                          idx);
    IMPORT_DLLE const DimensionOrg&     operator[]                     (size_t                          idx) const;


    IMPORT_DLLE const_iterator          cbegin                         () const;
    IMPORT_DLLE const_iterator          cend                           () const;

    IMPORT_DLLE const_iterator          begin                          () const;
    IMPORT_DLLE const_iterator          end                            () const;

    IMPORT_DLLE iterator                begin                          ();
    IMPORT_DLLE iterator                end                            ();

    IMPORT_DLLE void                    push_back                      (const DimensionOrg&             org);

    IMPORT_DLLE size_t                  GetSize                        () const;
    IMPORT_DLLE size_t                  GetTypeSize                    () const;

    IMPORT_DLLE bool                    IsPOD                          () const;
    IMPORT_DLLE bool                    IsComplete                     () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* TDORAY: Is really inlined?
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool                                     operator==                     (const DimensionType&            lhs,
                                                                                const DimensionType&            rhs)
    {
    // TDORAY: Couldn't it be replaced by logic such as the following? Would be way more
    //         efficient and clear.
    // return lhs.GetID() == rhs.GetID() || 
    //        (lhs.IsCustom() && lhs.EqualTo(rhs));

    return (lhs.GetID() != rhs.GetID()) ? 
                false : 
                (!lhs.IsCustom() || lhs.EqualTo(rhs));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* TDORAY: Is really inlined?
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool                                     operator<                      (const DimensionType&            lhs,
                                                                                const DimensionType&            rhs)
    {
    // NTERAY: Maybe sorting using name strings only would be fine too.. Check performances.
    // TDORAY: Couldn't it be replaced by logic such as the following? Would be way more
    //         efficient and clear.
    // return lhs.GetID() < rhs.GetID() || 
    //        (!(rhs.GetID() < lhs.GetID()) && lhs.IsCustom() && lhs.LessThan(rhs))


    return (lhs.GetID() < rhs.GetID()) ? 
                true : 
                ((rhs.GetID() < lhs.GetID()) ? 
                    false : 
                    (!lhs.IsCustom()) ? // TDORAY: Consider running directly LessThan at this point
                        false : 
                        lhs.LessThan(rhs));
    }


END_BENTLEY_MRDTM_IMPORT_NAMESPACE