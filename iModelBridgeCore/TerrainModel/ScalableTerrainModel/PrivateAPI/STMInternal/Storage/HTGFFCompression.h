//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFCompression.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

namespace HTGFF {

/*---------------------------------------------------------------------------------**//**
* @description  Descriptor for a compression type. As some compression type also
*               include specific parameters, this class has underlying polymorphic
*               types for specifying each compress type. Each compress type can
*               be factored via their Create function.
*               e.g: HTGFFCompression::Deflate::Create([maybe some parameters here])
*
*               In order to access specific compression parameters, one has to create
*               his own visitor type (Visitor pattern).
*               e.g.:
*               class MyVisitor : public HTGFFCompression::Visitor
*               {
*               ... implement Visit methods here ...
*               };
*
*               MyVisitor myVisitor (...);
*               HTGFFCompression myCompression = ...;
*               myCompression.Accept(myVisitor);
*
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class Compression
    {
public:
    enum Type
        {
        TYPE_NONE,
        TYPE_DEFLATE,
        TYPE_QTY,
        };

    // Supported compress types.
    class None;
    class Deflate;


    // Compress type visitor
    class Visitor;
                                  ~Compression                   ();


    Compression                    (const Compression&                      pi_rRight);
    Compression&                        operator=                      (const Compression&                      pi_rRight);

     Type                         GetType                        () const;

     bool                         Accept                         (Visitor&                                pi_rVisitor) const;

private:
    class Base;

    explicit                            Compression                    (Base*                                   pi_pCompression);

    auto_ptr<Base>                      m_pCompression;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Base class for specific compression flavors.
*
* @see HTGFFCompression
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class Compression::Base
    {
public:
    virtual                             ~Base                          () = 0;

     Type                         GetType                        () const;

     bool                         Accept                         (Compression::Visitor&                   pi_rVisitor) const;
     Base*                        Clone                          () const;

protected:
    explicit                            Base                           (Type                                    pi_Type);

    // Give specialization the possibility to clone themselves using their copy constructors
    Base                           (const Base&                             pi_rRight);
private:
    Base&                               operator=                      (const Base&                             pi_rRight);

    virtual bool                        _Accept                        (Compression::Visitor&                   pi_rVisitor) const = 0;

    virtual Base*                       _Clone                         () const = 0;

    Type                                m_Type;
    };

/*---------------------------------------------------------------------------------**//**
* @description  No compression flavor.
*
* @see HTGFFCompression
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class Compression::None : public Compression::Base
    {
    explicit                            None                           ();

    virtual bool                        _Accept                        (Compression::Visitor&              pi_rVisitor) const override;
    virtual None*                       _Clone                         () const override;

public:
     static Compression           Create                         ();
    };

/*---------------------------------------------------------------------------------**//**
* @description  Deflate compression flavor.
*
* @see HTGFFCompression
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class Compression::Deflate : public Compression::Base
    {
    explicit                            Deflate                        ();

    virtual bool                        _Accept                        (Compression::Visitor&              pi_rVisitor) const override;
    virtual Deflate*                    _Clone                         () const override;

public:
     static Compression           Create                         ();

    // TDORAY: Add parameters accessors here
    };


/*---------------------------------------------------------------------------------**//**
* @description  Visitor base. Implement specialization in order to access specific
*               compression parameters.
*
* @see HTGFFCompression
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class Compression::Visitor
    {
    virtual void  _Visit                         (const None&                             pi_rCompression) = 0;
    virtual void  _Visit                         (const Deflate&                          pi_rCompression) = 0;

public:
    virtual       ~Visitor                       () {};

     void   Visit                          (const None&                             pi_rCompression);
     void   Visit                          (const Deflate&                          pi_rCompression);

    };


} // END namespace HTGFF