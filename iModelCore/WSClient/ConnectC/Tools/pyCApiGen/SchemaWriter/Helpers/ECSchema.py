from SchemaWriter.Helpers.ECClass import ECClass
from SchemaWriter.Helpers.PropertyTypeError import PropertyTypeError


class ECSchema(object):
    def __init__(self, url_descriptor, repository_id, filename, name, api, status_codes):
        self.__url_descriptor = url_descriptor
        self.__repository_id = repository_id
        self.__filename = filename
        self.__name = name
        self.__api = api
        self.__status_codes = status_codes
        self.__ecclasses = []
        self.__ecschema_xmldoc = None

    def get_filename(self):
        return self.__filename

    def get_name(self):
        return self.__name

    def get_upper_name(self):
        return self.get_name().upper()

    def get_lower_name(self):
        return self.get_name().lower()

    def get_url_descriptor(self):
        return self.__url_descriptor

    def get_repository_id(self):
        return self.__repository_id

    def add_ecclass(self, autogenclass_row):
        self.__ecclasses.append(ECClass(autogenclass_row, self.__api, self.__status_codes))

    def get_classes(self):
        return self.__ecclasses

    def has_ecclass_with_property_type(self, property_type):
        for ecclass in self.get_classes():
            if ecclass.does_contain_property_type(property_type):
                return True
        return False

    def init_xml(self, xmldoc):
        self.__ecschema_xmldoc = xmldoc
        ecclasses = self.__ecschema_xmldoc.getElementsByTagName('ECClass')
        for ecclass in ecclasses:
            self.__get_ecclass_from_name(ecclass.attributes['typeName'].value).init_xml(ecclass)

    def __get_ecclass_from_name(self, name):
        for ecclass in self.__ecclasses:
            if ecclass.get_name() == name:
                return ecclass
        raise NameError("The name passed into __get_ecclass_from_name does not exists in the ecclass array")

    def __get_buffer_accessor_function_def(self, property_type):
        if property_type is 'StringLength':
            accessor_str = "CallStatus " + self.get_name() + "_GetStringLength\n"
        else:
            accessor_str = "CallStatus " + self.get_name() + "_Get" + property_type.title() + "Property\n"
        accessor_str += "(\n"
        accessor_str += "LP{0} api,\nH{0}BUFFER buf,\n".format(self.__api.get_api_acronym())
        accessor_str += "int16_t bufferProperty,\n"
        accessor_str += "uint32_t index,\n"
        if property_type == "string":
            accessor_str += "uint32_t strLength,\n"
            accessor_str += "WCharP str\n"
        elif property_type == "StringLength":
            accessor_str += "size_t* outStringSize\n"
        elif property_type == "guid":
            accessor_str += "uint32_t strLength,\n"
            accessor_str += "WCharP guid\n"
        elif property_type == "boolean":
            accessor_str += "bool* boolean\n"
        elif property_type == "int":
            accessor_str += "int32_t* integer\n"
        elif property_type == "double":
            accessor_str += "double* pDouble\n"
        elif property_type == "long":
            accessor_str += "int64_t* pLong\n"
        else:
            raise PropertyTypeError("Property type {0} not accepted".format(property_type))
        accessor_str += ")"
        return accessor_str

    def get_buffer_accessor_function_definition(self, property_type):
        return self.__get_buffer_accessor_function_def(property_type) + ';\n'

    def get_buffer_accessor_function_implementation(self, property_type):
        accessor_str = self.__get_buffer_accessor_function_def(property_type) + "\n"
        accessor_str += "    {\n"
        accessor_str += "    switch (buf->lClassType)\n"
        accessor_str += "        {\n"
        for ecclass in self.get_classes():
            if ecclass.should_exclude_entire_class() or not ecclass.does_contain_property_type(property_type):
                continue
            accessor_str += '        case BUFF_TYPE_{0}:\n'.format(ecclass.get_upper_name())
            accessor_str += '            {\n'
            if property_type is 'StringLength':
                    accessor_str += "            return {0}_GetStringLength".format(ecclass.get_name())
            else:
                accessor_str += "            return {0}_Get{1}Property".format(ecclass.get_name(), property_type.title())
            if property_type == "string":
                accessor_str += "(api, buf, bufferProperty, index, strLength, str);\n"
            elif property_type == "StringLength":
                accessor_str += "(api, buf, bufferProperty, index, outStringSize);\n"
            elif property_type == "guid":
                accessor_str += "(api, buf, bufferProperty, index, strLength, guid);\n"
            elif property_type == "boolean":
                accessor_str += "(api, buf, bufferProperty, index, boolean);\n"
            elif property_type == "int":
                accessor_str += "(api, buf, bufferProperty, index, integer);\n"
            elif property_type == "double":
                accessor_str += "(api, buf, bufferProperty, index, pDouble);\n"
            elif property_type == "long":
                accessor_str += "(api, buf, bufferProperty, index, pLong);\n"
            else:
                raise PropertyTypeError("Property type {0} not accepted".format(property_type))
            accessor_str += '            }\n'
        accessor_str += "        default:\n"
        accessor_str += '            api->SetStatusMessage("{1}");\n            api->SetStatusDescription("{2}");\n' \
                        '            return {0};\n'.format("INVALID_PARAMETER", self.__status_codes["INVALID_PARAMETER"].message,
                                                           "The buffer type passed in is invalid.")
        accessor_str += "        }\n"
        accessor_str += "    }\n"
        return accessor_str

    def __get_buffer_free_function_def(self):
        free_str = "CallStatus {0}_DataBufferFree\n".format(self.get_name())
        free_str += "(\n"
        free_str += "LP{0} api,\nH{0}BUFFER buf\n".format(self.__api.get_upper_api_acronym())
        free_str += ")"
        return free_str

    def get_buffer_free_function_implementation(self):
        free_str = self.__get_buffer_free_function_def() + '\n'
        free_str += "    {\n"
        free_str += "    for (int index = 0; index < buf->lItems.size(); index++)\n"
        free_str += "        {\n"
        free_str += "        if (buf->lItems[index] != nullptr)\n"
        free_str += "            {\n"
        free_str += "            switch(buf->lClassType)\n"
        free_str += "                {\n"
        for ecclass in self.get_classes():
            if ecclass.should_exclude_entire_class():
                continue
            free_str += "                case BUFF_TYPE_{0}:\n".format(ecclass.get_upper_name())
            free_str += "                    {\n"
            free_str += "                    LP{0}{1}BUFFER {2}Buf = (LP{0}{1}BUFFER) buf->lItems[index];\n"\
                .format(self.__api.get_upper_api_acronym(), ecclass.get_upper_name(), ecclass.get_lower_name())
            free_str += "                    delete {0}Buf;\n".format(ecclass.get_lower_name())
            free_str += "                    }\n"
            free_str += "                    break;\n"
        free_str += "                default:\n"
        free_str += "                    continue;\n"
        free_str += "                }\n"
        free_str += "            }\n"
        free_str += "        }\n"
        free_str += "    free(buf);\n"
        free_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                    '    return {0};\n    }}\n'.format("SUCCESS", self.__status_codes["SUCCESS"].message,
                                                       "The {0}_DataBufferFree function successfully completed."
                                                       .format(self.__api.get_api_name()))
        return free_str
