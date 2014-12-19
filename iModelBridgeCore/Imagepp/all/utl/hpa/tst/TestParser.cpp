//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/tst/TestParser.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/TestParser.h>

TestParser::TestParser()
    : HPAParser(new HPADefaultTokenizer)
    {
    // Tokenizer setup

    HPADefaultTokenizer* pTok = (HPADefaultTokenizer*)GetTokenizer();
    pTok->AddSymbol("IF",    IF_tk);
    pTok->AddSymbol("THEN",  THEN_tk);
    pTok->AddSymbol("ELSE",  ELSE_tk);
    pTok->AddSymbol("WHILE", WHILE_tk);
    pTok->AddSymbol("DO",    DO_tk);
    pTok->AddSymbol("BEGIN", BEGIN_tk);
    pTok->AddSymbol("END",   END_tk);
    pTok->AddSymbol("PRINT", PRINT_tk);
    pTok->AddSymbol("=",     EQ_tk);
    pTok->AddSymbol("!",     NOT_tk);
    pTok->AddSymbol(">",     GT_tk);
    pTok->AddSymbol("<",     LT_tk);
    pTok->AddSymbol("+",     PLUS_tk);
    pTok->AddSymbol("-",     MINUS_tk);
    pTok->AddSymbol("*",     MUL_tk);
    pTok->AddSymbol("/",     DIV_tk);
    pTok->AddSymbol("(",     LP_tk);
    pTok->AddSymbol(")",     RP_tk);

    pTok->SetStringToken(String_tk);
    pTok->SetNumberToken(Number_tk);
    pTok->SetIdentifierToken(Identifier_tk);

    // Grammar definition

    Program =          HPAProduction(StatementList);

    StatementList =    StatementList + Statement
                       || Statement;

    Statement =        IF_tk + BooleanExpr + THEN_tk + Statement
                       || IF_tk + BooleanExpr + THEN_tk + Statement + ELSE_tk + Statement
                       || WHILE_tk + BooleanExpr + DO_tk + Statement
                       || BEGIN_tk + StatementList + END_tk
                       || PRINT_tk + Expression
                       || Identifier_tk + EQ_tk + Expression;

    BooleanExpr =      Expression + EQ_tk + EQ_tk + Expression
                       || Expression + NOT_tk + EQ_tk + Expression
                       || Expression + GT_tk + Expression
                       || Expression + LT_tk + Expression;

    Expression =       Expression + PLUS_tk + Expression
                       || Expression + MINUS_tk + Expression
                       || Expression + MUL_tk + Expression
                       || Expression + DIV_tk + Expression
                       || LP_tk + Expression + RP_tk
                       || Identifier_tk
                       || Number_tk;

    // for debugging

    Program.SetName("Rule : Program");
    StatementList.SetName("Rule : StatementList");
    Statement.SetName("Rule : Statement");
    BooleanExpr.SetName("Rule : BooleanExpr");
    Expression.SetName("Rule : Expression");

    // Connection to parser

    SetStartRule(&Program);
    }

//------------------------------------------------------------------------

TestParser::~TestParser()
    {
    delete GetTokenizer();
    }
