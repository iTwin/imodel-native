//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

#define TEST_DYNAMIC 1

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/TestParser.h>
#include <Imagepp/all/h/HPADynamicParser.h>
#include <sstream>

#if TEST_DYNAMIC
char TestInputGrammar[] =
    "GRAMMAR_BEGIN \n"             \
    "\n"                           \
    "TOKEN IF_tk \"IF\"; \n"        \
    "TOKEN THEN_tk \"THEN\"; \n"    \
    "TOKEN ELSE_tk \"ELSE\"; \n"    \
    "TOKEN WHILE_tk \"WHILE\"; \n"  \
    "TOKEN DO_tk \"DO\"; \n"        \
    "TOKEN BEGIN_tk \"BEGIN\"; \n"  \
    "TOKEN END_tk \"END\"; \n"      \
    "TOKEN PRINT_tk \"PRINT\"; \n"  \
    "TOKEN EQ_tk \"=\"; \n"         \
    "TOKEN NOT_tk \"!\"; \n"     \
    "TOKEN GT_tk \">\"; \n"         \
    "TOKEN LT_tk \"<\"; \n"         \
    "TOKEN PLUS_tk \"+\"; \n"       \
    "TOKEN MINUS_tk \"-\"; \n"      \
    "TOKEN MUL_tk \"*\"; \n"        \
    "TOKEN DIV_tk \"/\"; \n"        \
    "TOKEN LP_tk \"(\"; \n"         \
    "TOKEN RP_tk \")\"; \n"         \
    "\n"                           \
    "Program =         StatementList; \n"                            \
    "\n"                                                              \
    "StatementList =   StatementList Statement \n"                   \
    "                | Statement; \n"                                \
    "\n"                                                              \
    "Statement =       IF_tk  BooleanExpr THEN_tk Statement \n"      \
    "                | IF_tk  BooleanExpr THEN_tk Statement ELSE_tk Statement \n" \
    "                | WHILE_tk BooleanExpr DO_tk Statement \n"      \
    "                | BEGIN_tk StatementList END_tk \n"             \
    "                | PRINT_tk Expression \n"                       \
    "                | IDENTIFIER EQ_tk Expression; \n"       \
    "\n"                                                              \
    "BooleanExpr =     Expression EQ_tk EQ_tk Expression \n"             \
    "                | Expression NOT_tk EQ_tk Expression \n"            \
    "                | Expression GT_tk Expression \n"               \
    "                | Expression LT_tk Expression; \n"              \
    "\n"                                                              \
    "Expression =      Expression PLUS_tk Expression \n"             \
    "                | Expression MINUS_tk Expression \n"            \
    "                | Expression MUL_tk Expression \n"              \
    "                | Expression DIV_tk Expression \n"              \
    "                | LP_tk Expression RP_tk \n"                    \
    "                | IDENTIFIER \n"                         \
    "                | NUMBER; \n"                            \
    "\n"                                                              \
    "GRAMMAR_END Program \n";
#endif

char TestInput[] =
    "IF A == 1 THEN \n"      \
    "  BEGIN \n"             \
    "    PRINT C \n"         \
    "  END \n"               \
    "ELSE \n"                \
    "  WHILE B != C DO \n"   \
    "    B = A * (B + 1) \n" \
    "PRINT PRINT \n";


int main(int argc, char** argv)
    {
    string InputBuffer = TestInput;
    wistringstream InputStream(InputBuffer);
#if TEST_DYNAMIC
    string InputBufferGrammar = TestInputGrammar;
    wistringstream InputStreamGrammar(InputBufferGrammar);
    HPADynamicParser Parser;
    Parser.ParseGrammar(InputStreamGrammar);
#else
    TestParser Parser;
#endif
    HPANode* pNode = Parser.Parse(InputStream);
#ifdef __HMR_DEBUG
    pNode->PrintState();
#endif
    cin.get();
    return 0;
    }