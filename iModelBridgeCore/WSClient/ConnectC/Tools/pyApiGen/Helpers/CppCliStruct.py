from CStruct import CStruct


class CppCliStruct(CStruct):
    def __init__(self, ecclass_dom, api, status_codes, excluded_ecclass):
        super(CppCliStruct, self).__init__(ecclass_dom, api, status_codes, excluded_ecclass)

    def get_property_declarations(self):
        properties_str = "        public:\n"
        properties_str += "            {0}();\n".format(self.get_name())
        properties_str += "            {0}({1}HANDLE apiHandle, {1}DATABUFHANDLE dataBuffer, int16_t index);\n".format(self.get_name(), self._api.get_upper_api_acronym())
        for ecproperty in self.get_properties():
            if self._excluded_ecclass.should_filter_property(ecproperty.attributes["propertyName"].value):
                continue
            property_type = ecproperty.attributes["typeName"].value
            if property_type == "guid":
                properties_str += "            property {0:16} {1};\n".format("Guid^",
                                                                              ecproperty.attributes["propertyName"].value)
            elif property_type == "string":
                properties_str += "            property {0:16} {1};\n".format("String^",
                                                                              ecproperty.attributes["propertyName"].value)
            elif property_type == "boolean":
                properties_str += "            property {0:16} {1};\n".format("Nullable<bool>",
                                                                              ecproperty.attributes["propertyName"].value)
            elif property_type == "long":
                properties_str += "            property {0:16} {1};\n".format('Nullable<int64_t>',
                                                                              ecproperty.attributes["propertyName"].value)

            else:
                properties_str += "            property {0:16} {1};\n".format("Nullable<" + property_type + '>',
                                                                              ecproperty.attributes["propertyName"].value)
        return properties_str

    def get_constructor_implementation(self):
        ctor_str = '    {0}::{0}() {{}}\n'.format(self.get_name())
        ctor_str += '    {0}::{0}({1}HANDLE apiHandle, {1}DATABUFHANDLE dataBuffer, int16_t index)\n'.format(self.get_name(), self._api.get_upper_api_acronym())
        ctor_str += '        {\n'
        ctor_str += '        if(dataBuffer == nullptr || apiHandle == nullptr)\n'
        ctor_str += '            throw gcnew ArgumentException("Null dataBuffer passed into constructor");\n\n'
        ctor_str += '        uint16_t status;\n'.format(self._api.get_upper_api_name())
        if self.does_contain_string():
            ctor_str += '        wchar_t stringBuf[4096];\n'
        if self.does_contain_boolean():
            ctor_str += '        bool boolean;\n'
        if self.does_contain_double():
            ctor_str += '        double pDouble;\n'
        if self.does_contain_guid():
            ctor_str += '        wchar_t guid[4096];\n'
        if self.does_contain_int():
            ctor_str += '        int32_t integer;\n'
        if self.does_contain_long():
            ctor_str += '        int64_t pLong;\n'
        for ecproperty in self.get_properties():
            if self._excluded_ecclass.should_filter_property(ecproperty.attributes["propertyName"].value):
                continue
            property_type = ecproperty.attributes["typeName"].value
            ctor_str += '        status = {0}_DataBufferGet{1}Property('.format(self._api.get_api_name(), property_type.title())
            ctor_str += 'apiHandle, dataBuffer, {0}_BUFF_{1}, index'.format(self.get_upper_name(), ecproperty.attributes["propertyName"].value.upper())
            if property_type == "string":
                ctor_str += ", 4096, stringBuf);\n"
            elif property_type == "guid":
                ctor_str += ", 4096, guid);\n"
            elif property_type == "boolean":
                ctor_str += ", &boolean);\n"
            elif property_type == "int":
                ctor_str += ", &integer);\n"
            elif property_type == "double":
                ctor_str += ", &pDouble);\n"
            elif property_type == "long":
                ctor_str += ", &pLong);\n"
            ctor_str += '        if (status == SUCCESS)\n'.format(self._api.get_upper_api_name())
            if property_type == "string":
                ctor_str += "             {0} = gcnew String(stringBuf);\n".format(ecproperty.attributes["propertyName"].value)
            elif property_type == "guid":
                ctor_str += "             {0} = gcnew Guid(gcnew String(guid));\n"\
                    .format(ecproperty.attributes["propertyName"].value)
            elif property_type == "boolean":
                ctor_str += "             {0} = boolean;\n".format(ecproperty.attributes["propertyName"].value)
            elif property_type == "int":
                ctor_str += "             {0} = integer;\n".format(ecproperty.attributes["propertyName"].value)
            elif property_type == "double":
                ctor_str += "             {0} = pDouble;\n".format(ecproperty.attributes["propertyName"].value)
            elif property_type == "long":
                ctor_str += "             {0} = pLong;\n".format(ecproperty.attributes["propertyName"].value)
        ctor_str += '        }\n'
        return ctor_str

    def get_api_gws_read_list_definition(self):
        return 'CallStatus^ Read{0}s(List<{0}^>^ {2}s);\n'.format(self.get_name(), self._api.get_api_name(), self.get_lower_name())

    def get_api_gws_read_list_implementation(self):
        read_list_str = '    CallStatus^ {1}::Read{0}s(List<{0}^>^ {2}s)\n'.format(self.get_name(), self._api.get_api_name(),
                                                                                   self.get_lower_name())
        read_list_str += '        {\n'
        read_list_str += '        {0}DATABUFHANDLE dataBuffer;\n'.format(self._api.get_upper_api_acronym())
        read_list_str += '        uint16_t status = {1}_Read{2}List(m_api, &dataBuffer);\n'.format(self._api.get_upper_api_name(),
                                                                                                     self._api.get_api_name(),
                                                                                                     self.get_name())
        read_list_str += '        if (SUCCESS != status)\n'.format(self._api.get_upper_api_name())
        read_list_str += '            return gcnew CallStatus(status, {0}_GetLastStatusMessage(m_api), {0}_GetLastStatusDescription(m_api));\n'\
            .format(self._api.get_api_name())
        read_list_str += '        uint64_t bufferCount = {0}_DataBufferGetCount(dataBuffer);\n'.format(self._api.get_api_name())
        read_list_str += '        if (bufferCount == 0)\n'
        read_list_str += '            return gcnew CallStatus(status, {0}_GetLastStatusMessage(m_api), {0}_GetLastStatusDescription(m_api));\n'\
            .format(self._api.get_api_name())
        read_list_str += '        for (int i = 0; i < bufferCount; i++)\n'
        read_list_str += '            {0}s->Add(gcnew {1}(m_api, dataBuffer, i));\n'.format(self.get_lower_name(), self.get_name())
        read_list_str += '        return gcnew CallStatus(status, {0}_GetLastStatusMessage(m_api), {0}_GetLastStatusDescription(m_api));\n'\
            .format(self._api.get_api_name())
        read_list_str += '        }\n'
        return read_list_str

    def get_api_gws_create_definition(self):
        return 'CallStatus^ Create{0}({0}^ {1});\n'.format(self.get_name(), self.get_lower_name(), self._api.get_api_name())

    def get_api_gws_create_implementation(self):
        create_str = '    CallStatus^ {2}::Create{0}({0}^ {1})\n'.format(self.get_name(), self.get_lower_name(), self._api.get_api_name())
        create_str += '        {\n'
        for ecproperty in self.get_properties():
            if self._excluded_ecclass.should_filter_property(ecproperty.attributes["propertyName"].value):
                continue
            if ecproperty.hasAttribute("readOnly") and ecproperty.attributes["readOnly"].value:
                continue
            property_type = ecproperty.attributes["typeName"].value
            if property_type == "string":
                create_str += '        pin_ptr<const wchar_t> {2} = PtrToStringChars({0}->{1});\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value,
                            ecproperty.attributes["propertyName"].value.lower())
            elif property_type == "guid":
                create_str += '        pin_ptr<const wchar_t> {2} = PtrToStringChars({0}->{1}->ToString());\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value,
                            ecproperty.attributes["propertyName"].value.lower())
            elif property_type == "boolean":
                create_str += '        bool {2} = {0}->{1}.GetValueOrDefault();\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value,
                            ecproperty.attributes["propertyName"].value.lower())
            elif property_type == "long":
                create_str += '        int64_t {2} = {0}->{1}.GetValueOrDefault();\n' \
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value,
                            ecproperty.attributes["propertyName"].value.lower())
            else:
                create_str += '        {2} {3} = {0}->{1}.GetValueOrDefault();\n'\
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value, property_type,
                            ecproperty.attributes["propertyName"].value.lower())
        create_str += '\n        uint16_t createStatus = {1}_Create{2}(m_api'.format(self._api.get_upper_api_name(),
                                                                                       self._api.get_api_name(),
                                                                                       self.get_name())
        for ecproperty in self.get_properties():
            if self._excluded_ecclass.should_filter_property(ecproperty.attributes["propertyName"].value):
                continue
            if ecproperty.hasAttribute("readOnly") and ecproperty.attributes["readOnly"].value:
                continue
            property_type = ecproperty.attributes["typeName"].value
            if property_type == "string" or property_type == "guid":
                create_str += ', {0}'.format(ecproperty.attributes["propertyName"].value.lower())
            else:
                create_str += ', &{0}'.format(ecproperty.attributes["propertyName"].value.lower())
        create_str += ');\n'
        create_str += '        return gcnew CallStatus(createStatus, {0}_GetLastStatusMessage(m_api), {0}_GetLastStatusDescription(m_api));\n'\
            .format(self._api.get_api_name())
        create_str += '        }\n'
        return create_str

    def get_api_gws_read_definition(self):
        return 'CallStatus^ Read{0}(String^ {1}Id, {0}^% {1});\n'.format(self.get_name(), self.get_lower_name(), self._api.get_api_name())

    def get_api_gws_read_implementation(self):
        read_str = '    CallStatus^ {1}::Read{0}(String^ {2}Id, {0}^% {2})\n'\
            .format(self.get_name(), self._api.get_api_name(), self.get_lower_name())
        read_str += '        {\n'
        read_str += '        {0}DATABUFHANDLE dataBuffer;\n'.format(self._api.get_upper_api_acronym())
        read_str += '        pin_ptr<const wchar_t> {0}IdPtr = PtrToStringChars({0}Id);\n'.format(self.get_lower_name())
        read_str += '        uint16_t readStatus = {1}_Read{2}(m_api, {3}IdPtr, &dataBuffer);\n'\
            .format(self._api.get_upper_api_name(), self._api.get_api_name(), self.get_name(), self.get_lower_name())
        read_str += '        if (SUCCESS != readStatus)\n'.format(self._api.get_upper_api_name())
        read_str += '            return gcnew CallStatus(readStatus, {0}_GetLastStatusMessage(m_api), {0}_GetLastStatusDescription(m_api));\n'\
            .format(self._api.get_api_name())
        read_str += '        uint64_t bufferCount = {0}_DataBufferGetCount(dataBuffer);\n'.format(self._api.get_api_name())
        read_str += '        if (bufferCount != 1)\n'
        read_str += '            return gcnew CallStatus(readStatus, {0}_GetLastStatusMessage(m_api), {0}_GetLastStatusDescription(m_api));\n'\
            .format(self._api.get_api_name())
        read_str += '        {0} = gcnew {1}(m_api, dataBuffer, 0);\n'.format(self.get_lower_name(), self.get_name())
        read_str += '        return gcnew CallStatus(readStatus, {0}_GetLastStatusMessage(m_api), {0}_GetLastStatusDescription(m_api));\n'\
            .format(self._api.get_api_name())
        read_str += '        }\n'
        return read_str

    def get_api_gws_update_definition(self):
        return 'CallStatus^ Update{0}(String^ {1}Id, {0}^ {1});\n'.format(self.get_name(), self.get_lower_name(), self._api.get_api_name())

    def get_api_gws_update_implementation(self):
        update_str = '    CallStatus^ {2}::Update{0}(String^ {1}Id, {0}^ {1})\n'\
            .format(self.get_name(), self.get_lower_name(), self._api.get_api_name())
        update_str += '        {\n'
        update_str += '        pin_ptr<const wchar_t> {0}IdPtr = PtrToStringChars({0}Id);\n'.format(self.get_lower_name())
        for ecproperty in self.get_properties():
            if self._excluded_ecclass.should_filter_property(ecproperty.attributes["propertyName"].value):
                continue
            if ecproperty.hasAttribute("readOnly") and ecproperty.attributes["readOnly"].value:
                continue
            property_type = ecproperty.attributes["typeName"].value
            if property_type == "string":
                update_str += '        pin_ptr<const wchar_t> {2} = PtrToStringChars({0}->{1});\n' \
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value,
                            ecproperty.attributes["propertyName"].value.lower())
            elif property_type == "guid":
                update_str += '        pin_ptr<const wchar_t> {2} = PtrToStringChars({0}->{1}->ToString());\n' \
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value,
                            ecproperty.attributes["propertyName"].value.lower())
            elif property_type == "boolean":
                update_str += '        bool {2} = {0}->{1}.GetValueOrDefault();\n' \
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value,
                            ecproperty.attributes["propertyName"].value.lower())
            elif property_type == "long":
                update_str += '        int64_t {2} = {0}->{1}.GetValueOrDefault();\n' \
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value,
                            ecproperty.attributes["propertyName"].value.lower())
            else:
                update_str += '        {2} {3} = {0}->{1}.GetValueOrDefault();\n' \
                    .format(self.get_lower_name(), ecproperty.attributes["propertyName"].value, property_type,
                            ecproperty.attributes["propertyName"].value.lower())
        update_str += '\n        uint16_t updateStatus = {1}_Update{2}(m_api, {3}IdPtr'\
            .format(self._api.get_upper_api_name(), self._api.get_api_name(), self.get_name(), self.get_lower_name())
        for ecproperty in self.get_properties():
            if self._excluded_ecclass.should_filter_property(ecproperty.attributes["propertyName"].value):
                continue
            if ecproperty.hasAttribute("readOnly") and ecproperty.attributes["readOnly"].value:
                continue
            property_type = ecproperty.attributes["typeName"].value
            if property_type == "string" or property_type == "guid":
                update_str += ', {0}'.format(ecproperty.attributes["propertyName"].value.lower())
            else:
                update_str += ', &{0}'.format(ecproperty.attributes["propertyName"].value.lower())
        update_str += ');\n'
        update_str += '        return gcnew CallStatus(updateStatus, {0}_GetLastStatusMessage(m_api), {0}_GetLastStatusDescription(m_api));\n' \
            .format(self._api.get_api_name())
        update_str += '        }\n'
        return update_str

    def get_api_gws_delete_definition(self):
        return 'CallStatus^ Delete{0}(String^ {1}Id);\n'.format(self.get_name(), self.get_lower_name(), self._api.get_api_name())

    def get_api_gws_delete_implementation(self):
        delete_str = '    CallStatus^ {2}::Delete{0}(String^ {1}Id)\n'\
            .format(self.get_name(), self.get_lower_name(), self._api.get_api_name())
        delete_str += '        {\n'
        delete_str += '        pin_ptr<const wchar_t> {0}IdPtr = PtrToStringChars({0}Id);\n'.format(self.get_lower_name())
        delete_str += '        uint16_t deleteStatus = {1}_Delete{2}(m_api, {3}IdPtr);\n'\
            .format(self._api.get_upper_api_name(), self._api.get_api_name(), self.get_name(), self.get_lower_name())
        delete_str += '        return gcnew CallStatus(deleteStatus, {0}_GetLastStatusMessage(m_api), {0}_GetLastStatusDescription(m_api));\n' \
            .format(self._api.get_api_name())
        delete_str += '        }\n'
        return delete_str
