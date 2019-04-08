//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFDataDescriptor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once



namespace HTGFF {

/*---------------------------------------------------------------------------------**//**
* @description  Description of a dimensions stored as part of a data type in a directory.
* @bsiclass                                                  Raymond.Gauthier   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class Dimension
    {
public:
    enum Type
        {
        TYPE_VOID,
        //TDORAY: Check
        //NOTE: No 1 Bit dimension defined as it would complicate the interface...
        TYPE_UINT8,
        TYPE_SINT8,
        TYPE_UINT16,
        TYPE_SINT16,
        TYPE_UINT32,
        TYPE_SINT32,
        TYPE_FLOAT_32,
        TYPE_UINT64,
        TYPE_SINT64,
        TYPE_FLOAT_64,

        // TDORAY: Characters?
        TYPE_QTY,
        TYPE_END = TYPE_QTY,
        };

    enum TypeRepresentation
        {
        TYPE_REPRESENTATION_NONE,
        TYPE_REPRESENTATION_UNSIGNED_INTEGER,
        TYPE_REPRESENTATION_SIGNED_INTEGER,
        TYPE_REPRESENTATION_FLOATING,
        TYPE_REPRESENTATION_QTY,
        };

    enum TypeSize
        {
        TYPE_SIZE_0,
        TYPE_SIZE_8,
        TYPE_SIZE_16,
        TYPE_SIZE_32,
        TYPE_SIZE_64,
        TYPE_SIZE_QTY,
        };


    typedef uint32_t                   Role;

    static Dimension               CreateIntFrom                  (TypeSize                    pi_Size,
                                                                   bool                        pi_Signed = false,
                                                                   Role                        pi_Role = GetUndefinedRole());


    static Dimension               CreateSIntFrom                 (TypeSize                    pi_Size,
                                                                   Role                        pi_Role = GetUndefinedRole());
    static Dimension               CreateUIntFrom                 (TypeSize                    pi_Size,
                                                                   Role                        pi_Role = GetUndefinedRole());
    static Dimension               CreateFloatFrom                (TypeSize                    pi_Size,
                                                                   Role                        pi_Role = GetUndefinedRole());

    static Dimension               CreateFrom                     (Type                        pi_Type,
                                                                   Role                        pi_Role = GetUndefinedRole());

    static const Dimension&        CreateVoid                     ();

    static const Dimension&        CreateByte                     ();
    static const Dimension&        CreateChar                     ();
    static const Dimension&        CreateWideChar                 ();

    static const Role                   GetUndefinedRole               ();


    Dimension                 ();

    Dimension                 (Type                        pi_Type,
                               Role                        pi_Role = GetUndefinedRole());


    bool                               operator<                      (const Dimension&       pi_rRight) const;
    bool                               operator==                     (const Dimension&       pi_rRight) const;

    Type                                GetType                        () const;
    size_t                              GetSize                        () const;

    //TDORAY: Add Method for accessing TypeRepresentation and TypeSize

    Role                                GetRole                        () const;
    void                                SetRole                        (Role                        pi_Role);


private:
    Type                                m_Type;
    Role                                m_Role; // User defined

    static const Role                   UNDEFINED_ROLE;
    static const size_t                 s_TypeSize[TYPE_QTY];
    };


/*---------------------------------------------------------------------------------**//**
* @description  Description of the data type stored in a directory. A data type
*               can have multiple dimensions
* @bsiclass                                                  Raymond.Gauthier   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class DataType
    {
public:
    typedef Dimension*             iterator;
    typedef const Dimension*       const_iterator;

    static const DataType&         CreateVoid                     ();
    static const DataType&         CreateByte                     ();
    static const DataType&         CreateChar                     ();
    static const DataType&         CreateWideChar                 ();

    // Simple packet data
    explicit                            DataType                  (const Dimension&       pi_Dimension = Dimension::CreateVoid());
    // Structured packet data
    explicit                            DataType                  (const_iterator              pi_DimensionBegin,
                                                                   const_iterator              pi_DimensionEnd);
    // Structured packet data
    explicit                            DataType                  (const Dimension::Type* pi_DimensionTypeBegin,
                                                                   const Dimension::Type* pi_DimensionTypeEnd);

    bool                               operator<                      (const DataType&        pi_rRight) const;
    bool                               operator==                     (const DataType&        pi_rRight) const;

    const_iterator                      Begin                          () const;
    const_iterator                      End                            () const;

    iterator                            Begin                          ();
    iterator                            End                            ();

    size_t                              GetDimensionQty                () const;
    size_t                              GetSize                        () const;
private:
    /*
                                        HTGFFDataType                  (const HTGFFDataType&);
    HTGFFDataType                       operator=                      (const HTGFFDataType&);
    */

    vector<Dimension>              m_Dimensions;
    size_t                              m_Size;
    // TDORAY: Add a separate flag here??
    };


} // END namespace HTGFF