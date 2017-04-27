//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HTGFFCompression.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFCompression.h>


namespace HTGFF {

Compression::Compression (Base* pi_pCompression)
    :   m_pCompression(pi_pCompression)
    {

    }

Compression::~Compression ()
    {
    };


Compression::Compression (const Compression& pi_rRight)
    :   m_pCompression(pi_rRight.m_pCompression->Clone())
    {

    }

Compression& Compression::operator= (const Compression& pi_rRight)
    {
    m_pCompression = auto_ptr<Base>(pi_rRight.m_pCompression->Clone());
    return *this;
    }

Compression::Type Compression::GetType () const
    {
    return m_pCompression->GetType();
    }



bool Compression::Accept (Visitor& pi_rVisitor) const
    {
    return m_pCompression->Accept(pi_rVisitor);
    }


Compression::Base::Base (Type pi_Type)
    :   m_Type(pi_Type)
    {
    HPRECONDITION(m_Type < TYPE_QTY);
    }

Compression::Base::~Base ()
    {

    }

Compression::Base::Base (const Base& pi_rRight)
    :   m_Type(pi_rRight.m_Type)
    {

    }

bool Compression::Base::Accept (Compression::Visitor& pi_rVisitor) const
    {
    return _Accept(pi_rVisitor);
    }

Compression::Base* Compression::Base::Clone () const
    {
    return _Clone();
    }


Compression::Type Compression::Base::GetType () const
    {
    return m_Type;
    }



Compression Compression::None::Create ()
    {
    return Compression(new None());
    }

Compression::None::None ()
    :   Compression::Base(Compression::TYPE_NONE)
    {

    }

Compression::None* Compression::None::_Clone () const
    {
    return new None(*this);
    }

bool Compression::None::_Accept (Visitor& pi_rVisitor) const
    {
    pi_rVisitor.Visit(*this);
    return true;
    }


Compression Compression::Deflate::Create ()
    {
    return Compression(new Deflate());
    }

Compression::Deflate::Deflate ()
    :   Compression::Base(Compression::TYPE_DEFLATE)
    {


    }

Compression::Deflate* Compression::Deflate::_Clone () const
    {
    return new Deflate(*this);
    }

bool Compression::Deflate::_Accept (Visitor& pi_rVisitor) const
    {
    pi_rVisitor.Visit(*this);
    return true;
    }



void Compression::Visitor::Visit (const None& pi_rCompression)
    {
    return _Visit(pi_rCompression);
    }

void Compression::Visitor::Visit (const Deflate& pi_rCompression)
    {
    return _Visit(pi_rCompression);
    }

} //END namespace HTGFF