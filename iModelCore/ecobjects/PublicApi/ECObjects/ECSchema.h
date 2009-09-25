/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECSchema.h $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects\ECObjects.h>
#include <string>

BEGIN_BENTLEY_EC_NAMESPACE
  
   
/*=================================================================================**//**
* @bsistruct                                                     
+===============+===============+===============+===============+===============+======*/
struct Class
    {
private:


public:        
    StatusInt GetECProperty (ECPropertyP & ecProperty, const wchar_t * propertyName) const;

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
struct Schema //: public CECCommon
{
    //friend class CECSchemaReference;
    //friend class CECReferencedSchemaIterator;
    //friend class CECClassIterator;

private:
    //    static ECValuePtrList s_recycledValues;    
    std::wstring m_schemaName;
    std::wstring m_schemaPrefix;
    std::wstring m_displayLabel;
    UInt32   m_majorVersion;
    UInt32   m_minorVersion;

    //    
    //    std::wstring m_referencedSchemaSearchPath;
    //    
    //    ECRelationshipConstraintUpgradeHandlerP m_ecRelationshipUpgradeHandler;
    //    ECClassResolutionHelperP                m_ecClassResolutionHelper;
    //    ECSchemaReferenceVector                 m_referencedSchemas; //needwork: make this a map based on the prefix
    //    CECClassVector                          m_CECClassVector;  //needswork: make this a map for fast lookup by name
    //    CECRelationshipClassVector              m_CECRelationshipClassVector;

    void init (std::wstring & name, std::wstring & prefix);

public:    
    Schema (const wchar_t * prefix, const wchar_t * name, UInt32 majorVersion, UInt32 minorVersion);
    ~Schema();

    const wchar_t * GetSchemaName() const;
    void SetSchemaName (const wchar_t * schemaName);    

    const wchar_t * GetSchemaPrefix() const;    
    void SetSchemaPrefix (const wchar_t * schemaPrefix);

    const wchar_t * GetDisplayLabel() const;
    void SetDisplayLabel (const wchar_t * displayLabel);

    static bool SchemaNameIsValid (const wchar_t * name);

    //    ECRelationshipConstraintUpgradeHandlerP GetECRelationshipUpgradeHandler ();

    //    void SetECRelationshipUpgradeHandler(ECRelationshipConstraintUpgradeHandlerP handler);

    //    ECClassResolutionHelperP GetECClassResolutionHelper ();
    //    void SetECClassResolutionHelper(ECClassResolutionHelperP helper);

    //    static BOOL CECSchema::IsPrimitiveType (LPCWSTR typeName);

    //    static CECValue * GetRecycledECValue (CECProperty * pECProperty);
    //    static void RecycleECValue (CECValue * pECValue);
    //    static void FreeRecycledValues ();

    //    virtual void AddReference(CECSchemaReference & ecSchemaReference);// this is only virtual so that this becomes a polymorphic type (to avoid error C2683)
  

    //    CECSchema * GetReferencedSchema (ULONG i);

    //    CECSchema * FindReferencedSchema (LPCWSTR schemaName, LPCWSTR versionString = NULL);

    //    CECSchema * FindReferencedSchemaByPrefix (LPCWSTR prefix);

    //    void AddECRelationshipClass(CECRelationshipClass *pCECRelationshipClass);

    //    CECRelationshipClass * GetECRelationshipClass(int i);
    //    int GetECRelationshipClassCount();

    //    CECClass * CreateECClass (LPCWSTR typeName, LPCWSTR description = L"", DWORD classType = ECXML_CLASSTYPE_DOMAIN);

    //    CECRelationshipClass * CreateECRelationshipClass (LPCWSTR typeName, LPCWSTR description = L"", DWORD classType = ECXML_CLASSTYPE_DOMAIN);

    //    void AddECClass (CECClass *pCECClass);
    //    void RemoveAllClasses ();

    //    /* needswork: I'd like to have a GetECClassCount, but does that just count locals or all the referenced schemas?
    //       Maybe everything just works locally, except IterateOverAllClasses

    //       Change GetECClassVectorSize to GetECClassCount, and MAYBE pass an optional parameter (includeReferencedSchemas)?

    //       Right now, I've exposed multiple ways to iterate... by index, by IterateOverAllClasses, and CECClassIterator.
    //       We should get rid of either the CECClassIterator or the ByIndex... ByIndex would still work if we have a sorted
    //       std::vector, but it would stop working if we changed to a map. Sorted std::vector is probably adequate.
    //       But the CECClassIterator ultimately gives us the greatest flexibility.
    //       */

    //    CECClass * GetECClassByIndex(int i);
    //    int GetECClassCount();

    //    // used to iterate over all classes in a schema
    //    // keeps iterating until a non-NULL value is returned.
    //    typedef void * (* ClassIteratorCallbackP) (CECClass * pECClass, void * context);

    //    void * IterateOverAllClasses
    //    (
    //    ClassIteratorCallbackP callback,                // i  - processing continues until the callback returns a non-NULL value
    //    void *                 context,                 // i  - pass whatever contextual information you want into this
    //    BOOL                   includeReferencedSchemas
    //    );
    //        

    //    // used to iterate over all classes in a schema
    //    typedef void * (* RelationshipClassIteratorCallbackP) (CECRelationshipClass * pECRelationshipClass, void * context);

    //    void * IterateOverAllRelationshipClasses
    //    (
    //    RelationshipClassIteratorCallbackP callback,                // i  - processing continues until the callback returns a non-NULL value
    //    void *                 context,                 // i  - pass whatever contextual information you want into this
    //    BOOL                   includeReferencedSchemas
    //    );
    //        

    //    /*---------------------------------------------------------------------------------**//**
    //      Used by FindECClass
    //     @bsimethod                                                     Casey.Mullen    06/04
    //    +---------------+---------------+---------------+---------------+---------------+------*/
    //private:
    //    static void * findECClassCallback (CECClass * pECClass, void * context);
    //public:
    //    /*---------------------------------------------------------------------------------**//**
    //      The classType will be logically AND-ed with the ECClasses' ClassType flags.
    //      The result of the AND must be non-zero, and the className must match, in order
    //      for a given class to be considered a match.

    //     @bsimethod                                                     Casey.Mullen    06/04
    //    +---------------+---------------+---------------+---------------+---------------+------*/
    //    CECClass * FindECClass
    //    (
    //    LPCWSTR className,                   // i  - name of the class to be found. //needswork: what about the namespace?
    //    DWORD classType = ECXML_CLASSTYPE_ALL // i  - This will be logically AND-ed with the ClassType flags. The AND must be non-zero, and the className must match, in order for a given class to be a match
    //    );

    //    /*---------------------------------------------------------------------------------**//**
    //      The classType will be logically AND-ed with the ECClasses' ClassType flags.
    //      The result of the AND must be non-zero, and the className must match, in order
    //      for a given class to be considered a match.

    //     @bsimethod                                                     Casey.Mullen    06/04
    //    +---------------+---------------+---------------+---------------+---------------+------*/
    //    CECClass * FindECClassUsingSchemaName
    //    (
    //    LPCWSTR className,                   // i  - name of the class to be found.
    //    LPCWSTR schemaName,                  // i  - full schemaName
    //    DWORD classType = ECXML_CLASSTYPE_ALL // i  - This will be logically AND-ed with the ClassType flags. The AND must be non-zero, and the className must match, in order for a given class to be a match
    //    );
    //    
    //    CECClass * FindECClassUsingPrefix
    //    (
    //    LPCWSTR className,                   // i  - name of the class to be found.
    //    LPCWSTR prefix    ,                  // i  - schemaPrefix
    //    DWORD classType = ECXML_CLASSTYPE_ALL // i  - This will be logically AND-ed with the ClassType flags. The AND must be non-zero, and the className must match, in order for a given class to be a match
    //    );

    //    /*---------------------------------------------------------------------------------**//**
    //      The classType will be logically AND-ed with the ECClasses' ClassType flags.
    //      The result of the AND must be non-zero, and the className must match, in order
    //      for a given class to be considered a match.

    //     @bsimethod                                                     Casey.Mullen    06/04
    //    +---------------+---------------+---------------+---------------+---------------+------*/
    //    CECClass * FindECClassUsingFullName
    //    (
    //    LPCWSTR fullname,                    // i  - full name (including prefix) of the class to be found.
    //    DWORD classType = ECXML_CLASSTYPE_ALL // i  - This will be logically AND-ed with the ClassType flags. The AND must be non-zero, and the className must match, in order for a given class to be a match
    //    );

    //    /*---------------------------------------------------------------------------------**//**
    //      Used by FindECClass
    //     @bsimethod                                                     Casey.Mullen    06/04
    //    +---------------+---------------+---------------+---------------+---------------+------*/
    //private:
    //    static void * findECRelationshipClassCallback (CECRelationshipClass * pECRelationshipClass, void * context);
    //public:
    //    /*---------------------------------------------------------------------------------**//**
    //      The classType will be logically AND-ed with the ECRelationshipClasses' ClassType flags.
    //      The result of the AND must be non-zero, and the className must match, in order
    //      for a given class to be considered a match.

    //     @bsimethod                                                     Casey.Mullen    06/04
    //    +---------------+---------------+---------------+---------------+---------------+------*/
    //    CECRelationshipClass * FindECRelationshipClass
    //    (
    //    LPCWSTR className,                   // i  - name of the class to be found. //needswork: what about the namespace?
    //    DWORD classType = ECXML_CLASSTYPE_ALL // i  -
    //    );

    //    CECRelationshipClass * FindECRelationshipClassUsingSchemaName
    //    (
    //    LPCWSTR className,                   // i  - name of the class to be found.
    //    LPCWSTR schemaName,                  // i  - full schemaName
    //    DWORD classType = ECXML_CLASSTYPE_ALL // i  - This will be logically AND-ed with the ClassType flags. The AND must be non-zero, and the className must match, in order for a given class to be a match
    //    );

    //    /*---------------------------------------------------------------------------------**//**
    //      The classType will be logically AND-ed with the ECRelationshipClasses' ClassType flags.
    //      The result of the AND must be non-zero, and the className must match, in order
    //      for a given class to be considered a match.

    //     @bsimethod                                                     Casey.Mullen    06/04
    //    +---------------+---------------+---------------+---------------+---------------+------*/
    //    CECRelationshipClass * FindECRelationshipClassUsingPrefix
    //    (
    //    LPCWSTR className,                   // i  - name of the class to be found.
    //    LPCWSTR prefix    ,                  // i  - schemaPrefix
    //    DWORD classType = ECXML_CLASSTYPE_ALL // i  - This will be logically AND-ed with the ClassType flags. The AND must be non-zero, and the className must match, in order for a given class to be a match
    //    );

    //    CECRelationshipClass * FindECRelationshipClassUsingFullName
    //    (
    //    LPCWSTR fullname,                    // i  - full name (including prefix) of the class to be found.
    //    DWORD classType = ECXML_CLASSTYPE_ALL // i  - This will be logically AND-ed with the ClassType flags. The AND must be non-zero, and the className must match, in order for a given class to be a match
    //    );



    //    static void ParseVersionString (LPCWSTR version, ULONG * pMajor, ULONG * pMinor);
    //    static void FormatVersionString (std::wstring & version, ULONG major, ULONG minor);

    //    static void ParseECClassName
    //    (
    //    LPCWSTR          fullClassName,  // i  -
    //    std::wstring &   prefix,         // o  -
    //    std::wstring &   className       // o  -
    //    );        

    //    std::wstring GetRelativeClassName (CECClass * pECClass);

    //    std::wstring GetRelativeClassName (LPCWSTR className);

    //    std::wstring OfficialFilename ();

    //    inline std::wstring XmlNamespace ();


    //    void SetVersionString (LPCWSTR version);
    //    std::wstring VersionString ();

    //    int WriteXML (LPCWSTR ecSchemaXmlFile);//, LPCWSTR ecprefix, LPCWSTR ecschemauri); //needswork... the schema should know this
    //    int ReadXML (LPCWSTR ecSchemaXmlFile);

}; // Schema


END_BENTLEY_EC_NAMESPACE