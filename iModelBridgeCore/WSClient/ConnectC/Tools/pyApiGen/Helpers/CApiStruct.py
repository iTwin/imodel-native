from CStruct import CStruct


class CApiStruct(CStruct):
    def __init__(self, ecschema_name, ecclass_dom, api, status_codes):
        super(CApiStruct, self).__init__(ecclass_dom, api, status_codes)
        self.__ecschema_name = ecschema_name

    def __get_api_gws_read_list_funtion_def(self):
        get_request_str = "CallStatus {0}_Read{1}List\n".format(self._api.get_api_name(), self.get_name())
        get_request_str += "(\n"
        get_request_str += "{0}HANDLE apiHandle,\n".format(self._api.get_upper_api_acronym())
        get_request_str += "{0}DATABUFHANDLE* {1}Buffer\n".format(self._api.get_upper_api_acronym(), self.get_lower_name())
        get_request_str += ")"
        return get_request_str

    def get_api_gws_read_list_definition(self):
        return "{0}_EXPORT ".format(self._api.get_upper_api_acronym()) + self.__get_api_gws_read_list_funtion_def() + ";\n"

    def get_api_gws_read_list_implementation(self):
        get_request_str = self.__get_api_gws_read_list_funtion_def() + "\n"
        get_request_str += "    {\n"
        get_request_str += "    VERIFY_API\n\n"
        get_request_str += "    if ({0}Buffer == nullptr)\n".format(self.get_lower_name())
        get_request_str += "        {\n"
        get_request_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                           '        return {0};\n        }}\n\n'\
            .format("INVALID_PARAMETER",
                    self._status_codes["INVALID_PARAMETER"].message,
                    "{0}Buffer is a nullptr.".format(self.get_lower_name()))
        get_request_str += '    auto result = api->m_wsRepositoryClientPtr->SendQueryRequest(WSQuery("{0}", "{1}"))->GetResult();\n'\
            .format(self.__ecschema_name, self.get_name())
        get_request_str += "    if (!result.IsSuccess())\n"
        get_request_str += "        return wsresultTo{0}Status(api, result.GetError().GetId(), result.GetError().GetDisplayMessage()," \
                           " result.GetError().GetDisplayDescription());\n\n"\
            .format(self._api.get_api_name())
        get_request_str += "    {0}BUFFER* buf = ({0}BUFFER*) calloc(1, sizeof({0}BUFFER));\n".format(self._api.get_api_acronym())
        get_request_str += "    if (buf == nullptr)\n"
        get_request_str += "        {\n"
        get_request_str += "        free(buf);\n"
        get_request_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                           '        return {0};\n' \
            .format("INTERNAL_MEMORY_ERROR",
                    self._status_codes["INTERNAL_MEMORY_ERROR"].message,
                    "Failed to calloc memory for {0}BUFFER.".format(self._api.get_api_acronym()))
        get_request_str += "        }\n\n"
        get_request_str += "    for (WSObjectsReader::Instance instance : result.GetValue().GetInstances())\n"
        get_request_str += "        {\n"
        get_request_str += "        LP{0}{1}BUFFER bufToFill = new {0}{1}BUFFER;\n"\
            .format(self._api.get_api_acronym(), self.get_upper_name(), self.get_lower_name())
        get_request_str += "        {0}BufferStuffer(bufToFill, instance.GetProperties());\n".format(self.get_name())
        get_request_str += "        buf->lItems.push_back(bufToFill);\n"
        get_request_str += "        }\n\n"
        get_request_str += "    buf->lCount = buf->lItems.size();\n"
        get_request_str += "    buf->lType = BUFF_TYPE_{0};\n".format(self.get_upper_name())
        get_request_str += "    *{0}Buffer = ({1}DATABUFHANDLE) buf;\n\n".format(self.get_lower_name(), self._api.get_upper_api_acronym())
        get_request_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                           '    return {0};\n'.format("SUCCESS", self._status_codes["SUCCESS"].message,
                                                      "{0}_Read{1}List completed successfully."
                                                      .format(self._api.get_api_name(), self.get_name()))
        get_request_str += "    }\n"
        return get_request_str

    def __get_api_gws_create_function_def(self):
        create_request_str = "CallStatus {0}_Create{1}\n".format(self._api.get_api_name(), self.get_name())
        create_request_str += "(\n"
        create_request_str += "{0}HANDLE apiHandle".format(self._api.get_upper_api_acronym())
        create_request_str += self.__get_gws_properties_for_function_def()
        create_request_str += "\n)"
        return create_request_str

    def get_api_gws_create_definition(self):
        return "{0}_EXPORT ".format(self._api.get_upper_api_acronym()) + self.__get_api_gws_create_function_def() + ";\n"

    def get_api_gws_create_implementation(self):
        create_request_str = self.__get_api_gws_create_function_def() + "\n"
        create_request_str += "    {\n"
        create_request_str += "    VERIFY_API\n\n"
        create_request_str += "    Json::Value instance;\n"
        create_request_str += '    instance["schemaName"] = "{0}";\n'.format(self.__ecschema_name)
        create_request_str += '    instance["className"] = "{0}";\n\n'.format(self.get_name())
        create_request_str += self.__get_gws_properties_for_function_impl()
        create_request_str += '    instance["properties"] = propertiesJson;\n\n'
        create_request_str += '    Json::Value objectCreationJson;\n'
        create_request_str += '    objectCreationJson["instance"] = instance;\n\n'
        create_request_str += '    auto result = api->m_wsRepositoryClientPtr->SendCreateObjectRequest(objectCreationJson)->GetResult();\n'
        create_request_str += "    if (!result.IsSuccess())\n"
        create_request_str += "        return wsresultTo{0}Status(api, result.GetError().GetId(), result.GetError().GetDisplayMessage()," \
                              " result.GetError().GetDisplayDescription());\n\n"\
            .format(self._api.get_api_name())
        create_request_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                              '    return {0};\n'\
            .format("SUCCESS", self._status_codes["SUCCESS"].message,
                    "{0}_Create{1} completed successfully.".format(self._api.get_api_name(), self.get_name()))
        create_request_str += "    }\n"
        return create_request_str

    def __get_api_gws_read_function_def(self):
        read_request_str = "CallStatus {0}_Read{1}\n".format(self._api.get_api_name(), self.get_name())
        read_request_str += "(\n"
        read_request_str += "{0}HANDLE apiHandle,\n".format(self._api.get_upper_api_acronym())
        read_request_str += "WCharCP {0}Id,\n".format(self.get_lower_name())
        read_request_str += "{0}DATABUFHANDLE* {1}Buffer\n".format(self._api.get_upper_api_acronym(), self.get_lower_name())
        read_request_str += ")"
        return read_request_str

    def get_api_gws_read_definition(self):
        return "{0}_EXPORT ".format(self._api.get_upper_api_acronym()) + self.__get_api_gws_read_function_def() + ";\n"

    def get_api_gws_read_implementation(self):
        get_request_str = self.__get_api_gws_read_function_def() + "\n"
        get_request_str += "    {\n"
        get_request_str += "    VERIFY_API\n\n"
        get_request_str += "    if ({0}Buffer == nullptr || {0}Id == nullptr || wcslen({0}Id) == 0)\n".format(self.get_lower_name())
        get_request_str += "        {\n"
        get_request_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                           '        return {0};\n        }}\n\n'\
            .format("INVALID_PARAMETER", self._status_codes["INVALID_PARAMETER"].message,
                    "{0}Buffer is a nullptr or {0}Id is nullptr or empty.".format(self.get_lower_name()))
        get_request_str += '    auto result = api->m_wsRepositoryClientPtr->'
        get_request_str += 'SendGetObjectRequest({{"{0}", "{1}", Utf8String({2}Id)}})->GetResult();\n'\
            .format(self.__ecschema_name, self.get_name(), self.get_lower_name())
        get_request_str += "    if (!result.IsSuccess())\n"
        get_request_str += "        return wsresultTo{0}Status(api, result.GetError().GetId(), result.GetError().GetDisplayMessage()," \
                           " result.GetError().GetDisplayDescription());\n\n"\
            .format(self._api.get_api_name())
        get_request_str += "    LP{0}{1}BUFFER {2}Buf = new {0}{1}BUFFER;\n" \
            .format(self._api.get_api_acronym(), self.get_upper_name(), self.get_lower_name())
        get_request_str += "    {0}BufferStuffer({1}Buf, (*result.GetValue().GetInstances().begin()).GetProperties());\n\n"\
            .format(self.get_name(), self.get_lower_name())
        get_request_str += "    {0}BUFFER* buf = ({0}BUFFER*) calloc(1, sizeof({0}BUFFER));\n".format(self._api.get_api_acronym())
        get_request_str += "    if (buf == nullptr)\n"
        get_request_str += "        {\n"
        get_request_str += "        free({0}Buf);\n".format(self.get_lower_name())
        get_request_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                           '        return {0};\n' \
            .format("INTERNAL_MEMORY_ERROR",
                    self._status_codes["INTERNAL_MEMORY_ERROR"].message,
                    "Failed to calloc memory for {0}BUFFER.".format(self._api.get_api_acronym()))
        get_request_str += "        }\n\n"
        get_request_str += "    buf->lCount = 1;\n"
        get_request_str += "    buf->lType = BUFF_TYPE_{0};\n".format(self.get_upper_name())
        get_request_str += "    buf->lItems = {{{0}Buf}};\n".format(self.get_lower_name())
        get_request_str += "    *{0}Buffer = ({1}DATABUFHANDLE) buf;\n\n".format(self.get_lower_name(), self._api.get_upper_api_acronym())
        get_request_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                           '    return {0};\n'.format("SUCCESS", self._status_codes["SUCCESS"].message,
                                                      "{0}_Read{1} completed successfully."
                                                      .format(self._api.get_api_name(), self.get_name()))
        get_request_str += "    }\n"
        return get_request_str

    def __get_api_gws_update_function_def(self):
        update_request_str = "CallStatus {0}_Update{1}\n".format(self._api.get_api_name(), self.get_name())
        update_request_str += "(\n"
        update_request_str += "{0}HANDLE apiHandle,\n".format(self._api.get_upper_api_acronym())
        update_request_str += "WCharCP {0}Id".format(self.get_lower_name())
        update_request_str += self.__get_gws_properties_for_function_def()
        update_request_str += "\n)"
        return update_request_str

    def get_api_gws_update_definition(self):
        return "{0}_EXPORT ".format(self._api.get_upper_api_acronym()) + self.__get_api_gws_update_function_def() + ";\n"

    def get_api_gws_update_implementation(self):
        create_request_str = self.__get_api_gws_update_function_def() + "\n"
        create_request_str += "    {\n"
        create_request_str += "    VERIFY_API\n\n"
        create_request_str += self.__get_gws_properties_for_function_impl()
        create_request_str += '\n    auto result = api->m_wsRepositoryClientPtr->SendUpdateObjectRequest({{"{0}", "{1}", Utf8String({2}Id)}}, '\
            .format(self.__ecschema_name, self.get_name(), self.get_lower_name())
        create_request_str += 'propertiesJson)->GetResult();\n'
        create_request_str += "    if (!result.IsSuccess())\n"
        create_request_str += "        return wsresultTo{0}Status(api, result.GetError().GetId(), result.GetError().GetDisplayMessage()," \
                              " result.GetError().GetDisplayDescription());\n\n"\
            .format(self._api.get_api_name())
        create_request_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                              '    return {0};\n'\
            .format("SUCCESS", self._status_codes["SUCCESS"].message,
                    "{0}_Update{1} completed successfully.".format(self._api.get_api_name(), self.get_name()))
        create_request_str += "    }\n"
        return create_request_str

    def __get_api_gws_delete_function_def(self):
        delete_request_str = "CallStatus {0}_Delete{1}\n".format(self._api.get_api_name(), self.get_name())
        delete_request_str += "(\n"
        delete_request_str += "{0}HANDLE apiHandle,\n".format(self._api.get_upper_api_acronym())
        delete_request_str += "WCharCP {0}Id\n".format(self.get_lower_name())
        delete_request_str += ")"
        return delete_request_str

    def get_api_gws_delete_definition(self):
        return "{0}_EXPORT ".format(self._api.get_upper_api_acronym()) + self.__get_api_gws_delete_function_def() + ";\n"

    def get_api_gws_delete_implementation(self):
        delete_request_str = self.__get_api_gws_delete_function_def() + "\n"
        delete_request_str += "    {\n"
        delete_request_str += "    VERIFY_API\n\n"
        delete_request_str += "    if ({0}Id == nullptr || wcslen({0}Id) == 0)\n".format(self.get_lower_name())
        delete_request_str += "        {\n"
        delete_request_str += '        api->SetStatusMessage("{1}");\n         api->SetStatusDescription("{2}");\n' \
                              '        return {0};\n        }}\n\n'\
            .format("INVALID_PARAMETER", self._status_codes["INVALID_PARAMETER"].message,
                    "{0}Id is a nullptr or empty.".format(self.get_lower_name()))
        delete_request_str += '    auto result = api->m_wsRepositoryClientPtr->'
        delete_request_str += 'SendDeleteObjectRequest({{"{0}", "{1}", Utf8String({2}Id)}})->GetResult();\n' \
            .format(self.__ecschema_name, self.get_name(), self.get_lower_name())
        delete_request_str += "    if (!result.IsSuccess())\n"
        delete_request_str += "        return wsresultTo{0}Status(api, result.GetError().GetId(), result.GetError().GetDisplayMessage()," \
                              " result.GetError().GetDisplayDescription());\n\n"\
            .format(self._api.get_api_name())
        delete_request_str += '    api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                              '    return {0};\n'\
            .format("SUCCESS",
                    self._status_codes["SUCCESS"].message,
                    "{0}_Delete{1} completed successfully.".format(self._api.get_api_name(), self.get_name()))
        delete_request_str += "    }\n"
        return delete_request_str

    def __get_gws_properties_for_function_def(self):
        property_str = ""
        for ecproperty in self.get_properties():
            if ecproperty.hasAttribute("readOnly") and ecproperty.attributes["readOnly"].value:
                continue
            property_str += ',\n'
            property_type = ecproperty.attributes["typeName"].value
            if property_type == "string":
                property_str += "WCharCP "
            elif property_type == "guid":
                property_str += "WCharCP "
            elif property_type == "boolean":
                property_str += "bool* "
            elif property_type == "int":
                property_str += "int32_t* "
            elif property_type == "long":
                property_str += "int64_t* "
            else:
                property_str += ecproperty.attributes["typeName"].value + "* "
            property_str += ecproperty.attributes["propertyName"].value
        return property_str

    def __get_gws_properties_for_function_impl(self):
        properties_str = '    Json::Value propertiesJson;\n'
        for ecproperty in self.get_properties():
            if ecproperty.hasAttribute("readOnly") and ecproperty.attributes["readOnly"].value:
                continue
            properties_str += "    if ({0} != nullptr) ".format(ecproperty.attributes["propertyName"].value)
            property_type = ecproperty.attributes["typeName"].value
            if property_type == "guid":
                properties_str += 'propertiesJson["{0}"] = Utf8String({0});\n' \
                    .format(ecproperty.attributes["propertyName"].value)
            elif property_type == "string":
                properties_str += 'propertiesJson["{0}"] = Utf8String({0});\n' \
                    .format(ecproperty.attributes["propertyName"].value)
            else:
                properties_str += 'propertiesJson["{0}"] = *{0};\n'.format(ecproperty.attributes["propertyName"].value)
        properties_str += '    if (propertiesJson.size() == 0)\n'
        properties_str += '        {\n'
        properties_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                          '        return {0};\n        }}\n'\
            .format("INVALID_PARAMETER",
                    self._status_codes["INVALID_PARAMETER"].message,
                    "There were not any valid {0} properties passed in.".format(self.get_name()))
        return properties_str
