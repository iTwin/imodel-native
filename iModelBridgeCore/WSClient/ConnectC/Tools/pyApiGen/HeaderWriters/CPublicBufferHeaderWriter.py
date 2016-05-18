from Helpers.CBufferStruct import CBufferStruct
from HeaderWriters.HeaderWriter import HeaderWriter
from PropertyTypeError import PropertyTypeError


class CPublicBufferHeaderWriter(HeaderWriter):
    def __init__(self, ecclasses, header_filename, api, status_codes, excluded_classes=None):
        super(CPublicBufferHeaderWriter, self).__init__(ecclasses, header_filename, api, status_codes, excluded_classes)
        self.__buffer_structs = []
        for ecclass in self._ecclasses:
            if ecclass.attributes["typeName"].value in excluded_classes and \
                    excluded_classes[ecclass.attributes["typeName"].value].should_exclude_entire_class():
                continue
            self.__buffer_structs.append(CBufferStruct(ecclass, api, self._status_codes))

    def write_header(self):
        self.__write_header_comment()
        self._write_spacing()
        self.__write_using_directives()
        self._write_spacing()
        self.__write_buffer_enums()
        self._write_spacing()
        self.__write_api_function_definitions()
        self._close_file()

    def __write_header_comment(self):
        self._write_header_comments(True, True)
        self._file.write('\n#include <WebServices/ConnectC/{0}GenPublic.h>\n'.format(self._api.get_upper_api_acronym()))

    def __write_using_directives(self):
        self._file.write('using namespace std;')

    def __write_buffer_enums(self):
        self._file.write(self._COMMENT_GroupStart.format("BufferPropertyEnums", "{0} Buffer Property Enumerations"
                                                         .format(self._api.get_api_name())))
        for buffer_struct in self.__buffer_structs:
            self._file.write(self._COMMENT_GroupBriefShort.format(buffer_struct.get_name()))
            self._file.write(buffer_struct.get_enum())
            self._write_spacing()
        self._file.write(self._COMMENT_GroupEnd)

    def __write_api_function_definitions(self):
        self._file.write(self._COMMENT_GroupStart.format("BufferFunctions", "{0} Data Buffer Functions".format(self._api.get_api_name())))
        self.__write_api_free_buffer()
        self.__write_api_count_buffer()
        self.__write_api_get_functions()
        self._file.write(self._COMMENT_GroupEnd)

    def __write_api_count_buffer(self):
        self._file.write(self._COMMENT_GroupBriefLong
                         .format("Get a count of the number of items in a data buffer",
                                 "\param[in] dataBuffer Data buffer\n* \\return Object count"))
        self._file.write("{0}_EXPORT uint64_t {1}_DataBufferGetCount\n".format(self._api.get_upper_api_acronym(), self._api.get_api_name()))
        self._file.write("(\n")
        self._file.write("{0}DATABUFHANDLE dataBuffer\n".format(self._api.get_upper_api_acronym()))
        self._file.write(");\n")

    def __write_api_free_buffer(self):
        self._file.write(self._COMMENT_GroupBriefLong
                         .format("Free an allocated data buffer",
                                 "\param[in] apiHandle Handle to api\n* \param[in] dataBuffer Data buffer"))
        self._file.write("{0}_EXPORT CallStatus {1}_DataBufferFree\n".format(self._api.get_upper_api_acronym(), self._api.get_api_name()))
        self._file.write("(\n")
        self._file.write("{0}HANDLE apiHandle,\n{0}DATABUFHANDLE dataBuffer\n".format(self._api.get_upper_api_acronym()))
        self._file.write(");\n")

    def __write_api_get_functions(self):
        self.__write_api_string_accessors()
        self._write_spacing()
        self.__write_api_guid_accessor()
        self._write_spacing()
        self.__write_api_bool_accessor()
        self._write_spacing()
        self.__write_api_integer_accessor()
        self._write_spacing()
        self.__write_api_double_accessor()
        self._write_spacing()
        self.__write_api_long_accessor()
        self._write_spacing()
        
    def __write_api_string_accessors(self):
        ecclass_contains_a_string_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_string():
                ecclass_contains_a_string_property = True
                break
        if ecclass_contains_a_string_property:
            self._file.write(self.__get_api_accessor_comment("string"))
            self._file.write(self.__get_api_accessor_definition("string"))
            self._write_spacing()
            self._file.write(self.__get_api_accessor_comment("StringLength"))
            self._file.write(self.__get_api_accessor_definition("StringLength"))

    def __write_api_guid_accessor(self):
        ecclass_contains_a_guid_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_guid():
                ecclass_contains_a_guid_property = True
                break
        if ecclass_contains_a_guid_property:
            self._file.write(self.__get_api_accessor_comment("guid"))
            self._file.write(self.__get_api_accessor_definition("guid"))

    def __write_api_bool_accessor(self):
        ecclass_contains_a_boolean_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_boolean():
                ecclass_contains_a_boolean_property = True
                break
        if ecclass_contains_a_boolean_property:
            self._file.write(self.__get_api_accessor_comment("boolean"))
            self._file.write(self.__get_api_accessor_definition("boolean"))

    def __write_api_integer_accessor(self):
        ecclass_contains_a_integer_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_int():
                ecclass_contains_a_integer_property = True
                break
        if ecclass_contains_a_integer_property:
            self._file.write(self.__get_api_accessor_comment("int"))
            self._file.write(self.__get_api_accessor_definition("int"))

    def __write_api_double_accessor(self):
        ecclass_contains_a_double_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_int():
                ecclass_contains_a_double_property = True
                break
        if ecclass_contains_a_double_property:
            self._file.write(self.__get_api_accessor_comment("double"))
            self._file.write(self.__get_api_accessor_definition("double"))

    def __write_api_long_accessor(self):
        ecclass_contains_a_long_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_int():
                ecclass_contains_a_long_property = True
                break
        if ecclass_contains_a_long_property:
            self._file.write(self.__get_api_accessor_comment("long"))
            self._file.write(self.__get_api_accessor_definition("long"))

    def __get_api_accessor_comment(self, property_type):
        comment_str = "/************************************************************************************//**\n"
        comment_str += "* \\brief Get a {0} property from a data buffer\n".format(property_type.title())
        comment_str += "* \param[in] apiHandle handle to api\n"
        comment_str += "* \param[in] dataBuffer Data buffer\n"
        comment_str += "* \param[in] bufferProperty buffer property\n"
        comment_str += "* \param[in] index buffer index\n"
        if property_type == "string":
            comment_str += "* \param[in] strLength buffer length\n"
            comment_str += "* \param[out] str Pointer to buffer to store string property\n"
        elif property_type == "StringLength":
            comment_str += "* \param[out] outStringSize Pointer to store the string length\n"
        elif property_type == "guid":
            comment_str += "* \param[in] strLength guid-buffer length\n"
            comment_str += "* \param[out] guid Pointer to buffer to store GUID property\n"
        elif property_type == "boolean":
            comment_str += "* \param[out] boolean Pointer to bool to store property\n"
        elif property_type == "int":
            comment_str += "* \param[out] integer Pointer to int to store property\n"
        elif property_type == "double":
            comment_str += "* \param[out] pDouble Pointer to double to store property\n"
        elif property_type == "long":
            comment_str += "* \param[out] pLong Pointer to long to store property\n"
        else:
            raise PropertyTypeError("Property type {0} not accepted".format(property_type))
        comment_str += "* \\return Success or error code. See \\ref {0}StatusCodes\n".format(self._api.get_api_name())
        comment_str += "****************************************************************************************/\n"
        return comment_str

    def __get_api_accessor_definition(self, property_type):
        if property_type is 'StringLength':
            accessor_str = "{0}_EXPORT CallStatus {1}_DataBufferGetStringLength\n".format(self._api.get_upper_api_acronym(), self._api.get_api_name())
        else:
            accessor_str = "{0}_EXPORT CallStatus {1}_DataBufferGet{2}Property\n".format(self._api.get_upper_api_acronym(),
                                                                                         self._api.get_api_name(),
                                                                                         property_type.title())
        accessor_str += "(\n"
        accessor_str += "{0}HANDLE apiHandle,\n{0}DATABUFHANDLE dataBuffer,\n".format(self._api.get_upper_api_acronym())
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
        accessor_str += ");\n"
        return accessor_str








