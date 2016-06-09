from SchemaWriter.HeaderWriters.SchemaHeaderWriter import SchemaHeaderWriter


class CallStatus:
    def __init__(self, code, message):
        self.code = code
        self.message = message


class SchemaApiPublicHeaderWriter(SchemaHeaderWriter):
    def __init__(self, ecschema, header_filename, api, status_codes):
        super(SchemaApiPublicHeaderWriter, self).__init__(ecschema, header_filename, api, status_codes)

    def write_header(self):
        self.__write_header_comment()
        self._write_spacing()
        self._write_includes()
        self._write_spacing()
        self.__write_class_function_definitions()
        self._close_file()

    def __write_header_comment(self):
        self._write_header_comments(True, True)

    def _write_includes(self):
        self._file.write('#include <WebServices/ConnectC/{0}.h>\n'.format(self._api.get_upper_api_acronym()))

    def __write_class_function_definitions(self):
        self._file.write(self._COMMENT_GroupStart.format("{0}APIFunctions".format(self._api.get_api_name()),
                                                         "{0} API Function Declarations".format(self._api.get_api_name())))
        for ecclass in self._ecschema.get_classes():
            if ecclass.should_exclude_entire_class():
                continue
            if ecclass.should_have_read_list():
                self.__write_read_class_list_definition(ecclass)
            if ecclass.should_have_create():
                self.__write_create_class_definition(ecclass)
            if ecclass.should_have_read():
                self.__write_read_class_definition(ecclass)
            if ecclass.should_have_update():
                self.__write_update_class_definition(ecclass)
            if ecclass.should_have_delete():
                self.__write_delete_class_definition(ecclass)
        self._file.write(self._COMMENT_GroupEnd)

    def __write_read_class_list_definition(self, ecclass):
        self._file.write(self._COMMENT_GroupBriefLong.format("Query WSG to get list of {0}s".format(ecclass.get_lower_name()),
                                                             "\param[in] apiHandle API object\n"
                                                             "* \param[out] {0}Buffer Buffer of {1} data\n"
                                                             "* \\return Success or error code. See \\ref {2}StatusCodes"
                                                             .format(ecclass.get_lower_name(), ecclass.get_name(),
                                                                     self._api.get_api_name())))
        self._file.write(ecclass.get_read_class_list_definition())
        self._write_spacing()

    def __write_create_class_definition(self, ecclass):
        param_str = "\param[in] apiHandle API object\n"
        for ecproperty in ecclass.get_properties():
            if ecproperty.should_be_excluded:
                continue
            if ecproperty.is_read_only:
                continue
            param_str += "* \param[in] {0}\n".format(ecproperty.name)
        param_str += "* \\return Success or error code. See \\ref {0}StatusCodes".format(self._api.get_api_name())
        self._file.write(self._COMMENT_GroupBriefLong.format("Create a new {0}".format(ecclass.get_lower_name()), param_str))
        self._file.write(ecclass.get_create_class_definition())
        self._write_spacing()

    def __write_read_class_definition(self, ecclass):
        self._file.write(self._COMMENT_GroupBriefLong.format("Get {0} information buffer".format(ecclass.get_lower_name()),
                                                             "\param[in] apiHandle API object\n"
                                                             "* \param[in] {0}Id {1} ID to select\n"
                                                             "* \param[out] {0}Buffer {1} data buffer\n"
                                                             "* \\return Success or error code. See \\ref {2}StatusCodes"
                                                             .format(ecclass.get_lower_name(), ecclass.get_name(),
                                                                     self._api.get_api_name())))
        self._file.write(ecclass.get_read_class_definition())
        self._write_spacing()

    def __write_update_class_definition(self, ecclass):
        param_str = "\param[in] apiHandle API object\n"
        param_str += "* \param[in] {0}Id {1} ID to update\n".format(ecclass.get_lower_name(), ecclass.get_name())
        for ecproperty in ecclass.get_properties():
            if ecproperty.should_be_excluded:
                continue
            if ecproperty.is_read_only:
                continue
            param_str += "* \param[in] {0}\n".format(ecproperty.name)
        param_str += "* \\return Success or error code. See \\ref {0}StatusCodes".format(self._api.get_api_name())
        self._file.write(self._COMMENT_GroupBriefLong.format("Update an existing {0}".format(ecclass.get_lower_name()), param_str))
        self._file.write(ecclass.get_update_class_definition())
        self._write_spacing()

    def __write_delete_class_definition(self, ecclass):
        self._file.write(self._COMMENT_GroupBriefLong.format("Delete a {0}".format(ecclass.get_lower_name()),
                                                             "\param[in] apiHandle API object\n"
                                                             "* \param[in] {0}Id {1} ID to remove\n"
                                                             "* \\return Success or error code. See \\ref {2}StatusCodes"
                                                             .format(ecclass.get_lower_name(), ecclass.get_name(),
                                                                     self._api.get_api_name())))
        self._file.write(ecclass.get_delete_class_definition())
        self._write_spacing()
