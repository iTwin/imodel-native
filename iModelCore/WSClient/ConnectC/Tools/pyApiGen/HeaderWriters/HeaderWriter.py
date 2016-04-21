from Writer import Writer


class HeaderWriter(Writer):
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

    def __init__(self, ecclasses, filename, api, status_codes, excluded_classes):
        super(HeaderWriter, self).__init__(ecclasses, filename, api, status_codes, excluded_classes)
