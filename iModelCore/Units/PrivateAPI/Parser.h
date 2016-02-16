
#pragma once


static const Utf8Char Multiply = '*';
static const Utf8Char OpenParen = '(';
static const Utf8Char CloseParen = ')';
// Not used yet
static const Utf8Char OpenBracket = '[';
static const Utf8Char CloseBracket = ']';
 
 
struct UnitToken
    {
private:
    Utf8String  m_tolken;
    int         m_exponent = 1;

public:
    bool IsValid() { return !Utf8String::IsNullOrEmpty(m_tolken.c_str()) && abs(m_exponent) >= 1; }
    void AddChar(Utf8Char character) { m_tolken.append(1, character); }
    void SetExponent(int exponent) { m_exponent = exponent; }
    bool IsNumerator() { return m_exponent > 0; }
    void AddToNumeratorOrDenominator(bvector<Utf8String>& numerator, bvector<Utf8String>& denominator)
        {
        if (IsNumerator())
            AddToVector(numerator);
        else
            AddToVector(denominator);
        }

    void AddToVector(bvector<Utf8String>& ator)
        {
        for (int i = 0; i < abs(m_exponent); ++i)
            ator.push_back(Utf8String(m_tolken.c_str()));

        m_tolken.clear();
        m_exponent = 1;
        }
    };
 
struct Exponent
    {
private:
    Utf8String m_exponentChars;

public:
    bool IsValid() { return !Utf8String::IsNullOrEmpty(m_exponentChars.c_str()); }
    void AddChar(Utf8Char character) { m_exponentChars.append(1, character); }
    int GetExponent()
        {
        int value = 0;
        BE_STRING_UTILITIES_UTF8_SSCANF(m_exponentChars.c_str(), "%d", &value);
        m_exponentChars.clear();
        return value;
        }
    };

struct Parser
    {
public:
    static BentleyStatus ParseDefinition(Utf8CP definition, bvector<Utf8String>& numerator, bvector<Utf8String>& denominator);
    };