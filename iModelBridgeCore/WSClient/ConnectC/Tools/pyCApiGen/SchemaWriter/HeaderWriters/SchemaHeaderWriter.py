from SchemaWriter.SchemaWriter import SchemaWriter


class SchemaHeaderWriter(SchemaWriter):
    _COMMENT_GroupStart = "/************************************************************************************//**\n" \
                           "* \defgroup {0} {1}\n"                                                                     \
                           "* \{{\n"                                                                                    \
                           "****************************************************************************************/\n"

    _COMMENT_GroupDef = "/*!\n"      \
                        "\def {0}\n" \
                        "{1}\n"      \
                        "*/\n"

    _COMMENT_GroupBriefShort = "/**\n"            \
                               "* \\brief {0} \n" \
                               "*/\n"

    _COMMENT_GroupBriefLong = "/************************************************************************************//**\n" \
                              "* \\brief {0}\n"                                                                             \
                              "* {1}\n"                                                                                     \
                              "****************************************************************************************/\n"

    _COMMENT_GroupEnd = "/** \} */\n"

    def __init__(self, ecschema, filename, api, status_codes):
        super(SchemaHeaderWriter, self).__init__(ecschema, filename, api, status_codes)
