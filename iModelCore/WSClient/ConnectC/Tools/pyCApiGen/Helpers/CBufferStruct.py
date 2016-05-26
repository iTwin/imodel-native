from CStruct import CStruct
from PropertyTypeError import PropertyTypeError


class CBufferStruct(CStruct):
    def __init__(self, ecclass_dom, api, status_codes, excluded_ecclass):
        super(CBufferStruct, self).__init__(ecclass_dom, api, status_codes, excluded_ecclass)

    def __get_accessor_functions_definition(self, property_type):
        if not self.does_contain_property_type(property_type):
            raise PropertyTypeError("Property type {0} not accepted".format(property_type))
        if property_type is 'StringLength':
            accessor_str = "CallStatus " + self.get_lower_name() + "GetStringLength\n"
        else:
            accessor_str = "CallStatus " + self.get_lower_name() + "Get" + property_type.title() + "Property\n"
        accessor_str += "(\n"
        accessor_str += "LP{0} api,\nH{0}BUFFER buf,\n".format(self._api.get_api_acronym())
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

    def get_accessor_function_definition(self, property_type):
        return self.__get_accessor_functions_definition(property_type) + ";\n"

    def get_accessor_function_implementation(self, property_type):
        accessor_str = self.__get_accessor_functions_definition(property_type) + "\n"
        accessor_str += "    {\n"
        accessor_str += "    if (buf == nullptr || bufferProperty == 0"
        if property_type == "string":
            accessor_str += " || str == nullptr || strLength == 0"
        elif property_type == "StringLength":
            accessor_str += " || outStringSize == nullptr"
        elif property_type == "guid":
            accessor_str += " || guid == nullptr || strLength == 0"
        elif property_type == "boolean":
            accessor_str += " || boolean == nullptr"
        elif property_type == "int":
            accessor_str += " || integer == nullptr"
        elif property_type == "double":
            accessor_str += " || pDouble == nullptr"
        elif property_type == "long":
            accessor_str += " || pLong == nullptr"
        else:
            raise PropertyTypeError("Property type {0} not accepted".format(property_type))
        accessor_str += ")\n"
        accessor_str += '        {\n'
        accessor_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                        '        return {0};\n        }}\n\n'\
            .format("INVALID_PARAMETER",
                    self._status_codes["INVALID_PARAMETER"].message,
                    "An invalid buffer pointer or invalid property pointer was passed into the get property function.")
        accessor_str += '    if(index >= buf->lCount)\n'
        accessor_str += '        {\n'
        accessor_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                        '        return {0};\n        }}\n\n' \
            .format("INVALID_PARAMETER",
                    self._status_codes["INVALID_PARAMETER"].message,
                    "The index parameter passed into the get property function is out of bounds.")
        accessor_str += "    LP{0}{1}BUFFER {2}Buf = (LP{0}{1}BUFFER) buf->lItems[index];\n"\
            .format(self._api.get_api_acronym(), self.get_upper_name(), self.get_lower_name())

        is_first_property = True
        for ecproperty in self.get_properties():
            if self._excluded_ecclass.should_filter_property(ecproperty.attributes["propertyName"].value):
                continue
            if ecproperty.attributes["typeName"].value == property_type or \
               (ecproperty.attributes["typeName"].value == 'string' and property_type == 'StringLength'):
                if is_first_property:
                    accessor_str += "    if ({0}_BUFF_{1} == bufferProperty)\n".format(self.get_upper_name(),
                                                                                       ecproperty.attributes["propertyName"].value.upper())
                    is_first_property = False
                else:
                    accessor_str += "    else if ({0}_BUFF_{1} == bufferProperty)\n".format(self.get_upper_name(),
                                                                                            ecproperty.attributes["propertyName"].value.upper())
                accessor_str += "        {\n"
                property_access_str = '        if ({0}Buf->IsSet.find(WString("{1}", true)) == {0}Buf->IsSet.end() '
                property_access_str += '|| {0}Buf->IsSet[WString("{1}", true)] == false)\n'
                property_access_str += '            {{\n'
                if property_type == "string":
                    property_access_str += '            str = nullptr;\n'
                elif property_type == "StringLength":
                    property_access_str += '            outStringSize = nullptr;\n'
                elif property_type == "guid":
                    property_access_str += '            guid = nullptr;\n'
                elif property_type == "boolean":
                    property_access_str += '            boolean = nullptr;\n'
                elif property_type == "int":
                    property_access_str += '            integer = nullptr;\n'
                elif property_type == "double":
                    property_access_str += '            pDouble = nullptr;\n'
                elif property_type == "long":
                    property_access_str += '            pLong = nullptr;\n'
                else:
                    raise PropertyTypeError("Property type {0} not accepted".format(property_type))
                property_access_str += '            {2};\n'
                property_access_str += '            }}\n'

                if property_type == "string":
                    property_access_str += "        BeStringUtilities::Wcsncpy(str, strLength, {0}Buf->{1}.c_str());\n"
                elif property_type == "StringLength":
                    property_access_str += "        *outStringSize = {0}Buf->{1}.length();\n"
                elif property_type == "guid":
                    property_access_str += "        BeStringUtilities::Wcsncpy(guid, strLength, {0}Buf->{1}.c_str());\n"
                elif property_type == "boolean":
                    property_access_str += "        *boolean = {0}Buf->{1};\n"
                elif property_type == "int":
                    property_access_str += "        *integer = {0}Buf->{1};\n"
                elif property_type == "double":
                    property_access_str += "        *pDouble = {0}Buf->{1};\n"
                elif property_type == "long":
                    property_access_str += "        *pLong = {0}Buf->{1};\n"
                else:
                    raise PropertyTypeError("Property type {0} not accepted".format(property_type))
                accessor_str += property_access_str.format(self.get_lower_name(), ecproperty.attributes["propertyName"].value,
                                                           'api->SetStatusMessage("{1}");\n'
                                                           '            api->SetStatusDescription("{2}");\n'
                                                           '            return {0}'
                                                           .format("PROPERTY_HAS_NOT_BEEN_SET",
                                                                   self._status_codes['PROPERTY_HAS_NOT_BEEN_SET'].message,
                                                                   "{0} property is not set, so it can not be retrieved."
                                                                   .format(ecproperty.attributes["propertyName"].value)))
                accessor_str += "        }\n"
        if not is_first_property:
            accessor_str += "    else\n"
            accessor_str += '        {\n'
            accessor_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                            '        return {0};\n        }}\n'\
                .format("INVALID_PARAMETER",
                        self._status_codes["INVALID_PARAMETER"].message,
                        "The bufferProperty is invalid. It did not match up with any of the buffer's properties.")
        accessor_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                        '    return {0};\n'\
            .format("SUCCESS",
                    self._status_codes["SUCCESS"].message,
                    "The property retrieval function completed successfully.")
        accessor_str += "    }\n"
        return accessor_str

    def get_struct_typedef(self):
        struct_str = 'typedef struct _' + self._api.get_api_acronym() + '_'
        struct_str += self._ecclass.attributes["typeName"].value.upper() + '_BUFFER \n'
        struct_str += '    {\n'
        struct_str += '    bmap<WString, bool> IsSet;\n'
        for ecproperty in self.get_properties():
            if self._excluded_ecclass.should_filter_property(ecproperty.attributes["propertyName"].value):
                continue
            property_type = ecproperty.attributes["typeName"].value
            if property_type == "string":
                struct_str += "    WString "
            elif property_type == "guid":
                struct_str += "    WString "
            elif property_type == "boolean":
                struct_str += "    bool "
            elif property_type == "int":
                struct_str += "    int32_t "
            elif property_type == "long":
                struct_str += "    int64_t "
            else:
                struct_str += "    " + ecproperty.attributes["typeName"].value + " "
            struct_str += ecproperty.attributes["propertyName"].value + ";\n"
        struct_str += "    } " + self._api.get_api_acronym() + self._ecclass.attributes["typeName"].value.upper()
        struct_str += "BUFFER, *LP" + self._api.get_api_acronym()
        struct_str += self._ecclass.attributes["typeName"].value.upper() + "BUFFER;"
        return struct_str

    def get_enum(self):
        enum_str = "typedef enum\n"
        enum_str += "    {\n"
        enum_count = 1
        for ecproperty in self.get_properties():
            if self._excluded_ecclass.should_filter_property(ecproperty.attributes["propertyName"].value):
                continue
            enum_str += "    {0}_BUFF_{1:30} = {2}, /**< \\b {3}. */\n"\
                .format(self.get_upper_name(), ecproperty.attributes["propertyName"].value.upper(), enum_count,
                        ecproperty.attributes["propertyName"].value)
            enum_count += 1
        enum_str += "    }} {0}_BUFF_PROPERTY;\n".format(self.get_upper_name())
        return enum_str

    def get_stuffer_definition(self):
        stuffer_str = "void {0}BufferStuffer\n".format(self.get_name())
        stuffer_str += "(\n"
        stuffer_str += "LP{0}{1}BUFFER {2}Buf,\n".format(self._api.get_api_acronym(), self.get_upper_name(), self.get_lower_name())
        stuffer_str += "RapidJsonValueCR properties\n"
        stuffer_str += ");\n"
        return stuffer_str

    def get_stuffer_implementation(self):
        stuffer_str = "void {0}BufferStuffer\n".format(self.get_name())
        stuffer_str += "(\n"
        stuffer_str += "LP{0}{1}BUFFER {2}Buf,\n".format(self._api.get_api_acronym(), self.get_upper_name(), self.get_lower_name())
        stuffer_str += "RapidJsonValueCR properties\n"
        stuffer_str += ")\n"
        stuffer_str += "    {\n"
        for ecproperty in self.get_properties():
            if self._excluded_ecclass.should_filter_property(ecproperty.attributes["propertyName"].value):
                continue
            property_type = ecproperty.attributes["typeName"].value
            if property_type == "string":
                stuffer_str += '    if(properties.HasMember("{0}") && properties["{0}"].IsString())\n'\
                    .format(ecproperty.attributes["propertyName"].value)
                stuffer_str += '        {0}Buf->{1} = WString(properties["{1}"].GetString(), true);\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = (properties.HasMember("{1}") && properties["{1}"].IsString());\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value)
            elif property_type == "guid":
                stuffer_str += '    if(properties.HasMember("{0}") && properties["{0}"].IsString())\n'\
                    .format(ecproperty.attributes["propertyName"].value)
                stuffer_str += '        {0}Buf->{1} = WString(properties["{1}"].GetString(), true);\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = (properties.HasMember("{1}") && properties["{1}"].IsString());\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value)
            elif property_type == "boolean":
                stuffer_str += '    if(properties.HasMember("{0}") && properties["{0}"].IsBool())\n'\
                    .format(ecproperty.attributes["propertyName"].value)
                stuffer_str += '        {0}Buf->{1} = properties["{1}"].GetBool();\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = (properties.HasMember("{1}") && properties["{1}"].IsBool());\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value)
            elif property_type == "int" or property_type == "long":
                stuffer_str += '    if(properties.HasMember("{0}") && properties["{0}"].IsInt())\n'\
                    .format(ecproperty.attributes["propertyName"].value)
                stuffer_str += '        {0}Buf->{1} = properties["{1}"].GetInt();\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = (properties.HasMember("{1}") && properties["{1}"].IsInt());\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value)
            elif property_type == "double":
                stuffer_str += '    if(properties.HasMember("{0}") && properties["{0}"].IsDouble())\n'\
                    .format(ecproperty.attributes["propertyName"].value)
                stuffer_str += '        {0}Buf->{1} = properties["{1}"].GetDouble();\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = (properties.HasMember("{1}") && properties["{1}"].IsDouble());\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value)
            else:
                raise PropertyTypeError("Property type {0} not accepted".format(property_type))
        stuffer_str += "    }\n"
        return stuffer_str

