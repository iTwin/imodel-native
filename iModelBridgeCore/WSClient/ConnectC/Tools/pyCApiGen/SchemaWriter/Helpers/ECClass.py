#!Python
#--------------------------------------------------------------------------------------
#
#     $Source: ConnectC/Tools/pyCApiGen/SchemaWriter/Helpers/ECClass.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
from SchemaWriter.Helpers.ECProperty import ECProperty
from SchemaWriter.Helpers.PropertyTypeError import PropertyTypeError

#-------------------------------------------------------------------------------------------
# bsiclass                                                            06/2016
#-------------------------------------------------------------------------------------------
class ECClass(object):
    def __init__(self, row, api, status_codes):
        self.__url_descriptor = row[0].value
        self.__repository_id = row[1].value
        self.__ecschema_name = row[3].value
        self.__name = row[4].value
        self.__include_create = row[5].value
        self.__include_read = row[6].value
        self.__include_update = row[7].value
        self.__include_delete = row[8].value
        self.__include_read_list = row[9].value
        self.__excluded_properties = [] if row[10].value is None else row[10].value.split(',')
        self.__excluded_properties = [ecproperty.strip() for ecproperty in self.__excluded_properties]
        self.__api = api
        self.__status_codes = status_codes
        self.__ecproperties = []
        self.__is_domain_class = None
        self.__ecRelationshipclasses = []

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def init_xml(self, ecclass_xmldoc):
        self.__is_domain_class = ecclass_xmldoc.attributes['isDomainClass'].value
        ecproperties = ecclass_xmldoc.getElementsByTagName('ECProperty')
        for ecproperty in ecproperties:
            self.__ecproperties.append(ECProperty(ecproperty.attributes["propertyName"].value,
                                                  ecproperty.attributes["typeName"].value,
                                                  ecproperty.hasAttribute("readOnly") and
                                                  ecproperty.attributes["readOnly"].value,
                                                  ecproperty.attributes["propertyName"].value in self.__excluded_properties))

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_name(self):
        return self.__name

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_lower_name(self):
        return self.get_name().lower()

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_upper_name(self):
        return self.get_name().upper()

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_schema_name(self):
        return self.__ecschema_name

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_have_create(self):
        return self.__include_create

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_have_read(self):
        return self.__include_read

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_have_update(self):
        return self.__include_update

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_have_delete(self):
        return self.__include_delete

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_have_read_list(self):
        return self.__include_read_list

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_exclude_entire_class(self):
        return (not self.should_have_create()) and (not self.should_have_read()) and \
               (not self.should_have_update()) and (not self.should_have_delete() and
                                                    (not self.should_have_read_list()))

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_filter_property(self, property_to_filter):
        return property_to_filter in self.__excluded_properties

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_properties(self):
        return self.__ecproperties

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def _get_unique_property_types(self):
        unique_properties = []
        for ecproperty in self.get_properties():
            if ecproperty.should_be_excluded:
                continue
            if ecproperty.type not in unique_properties:
                unique_properties.append(ecproperty.type)
        return unique_properties

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def does_contain_property_type(self, property_type):
        if property_type == "StringLength":
            property_type = 'string'
        return property_type in self._get_unique_property_types()

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def does_contain_string(self):
        return self.does_contain_property_type('string')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def does_contain_guid(self):
        return self.does_contain_property_type('guid')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def does_contain_boolean(self):
        return self.does_contain_property_type('boolean')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def does_contain_int(self):
        return self.does_contain_property_type('int')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def does_contain_double(self):
        return self.does_contain_property_type('double')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def does_contain_long(self):
        return self.does_contain_property_type('long')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def does_contain_datetime(self):
        return self.does_contain_property_type('dateTime')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_read_class_list_funtion_def(self):
        get_request_str = "CallStatus {0}_Read{1}List\n".format(self.__api.get_api_name(), self.get_name())
        get_request_str += "(\n"
        get_request_str += "{0}HANDLE apiHandle,\n".format(self.__api.get_upper_api_acronym())
        get_request_str += "{0}DATABUFHANDLE* {1}Buffer\n".format(self.__api.get_upper_api_acronym(), self.get_lower_name())
        get_request_str += ")"
        return get_request_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_read_class_list_definition(self):
        return "{0}_EXPORT ".format(self.__api.get_upper_api_acronym()) + self.__get_read_class_list_funtion_def() + ";\n"

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def set_relationship_class_list(self, ec_relationship_class_list):
        self.__ecRelationshipclasses = ec_relationship_class_list
    
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_relationship_class_list(self):
        return self.__ecRelationshipclasses

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_read_class_list_implementation(self):
        get_request_str = self.__get_read_class_list_funtion_def() + "\n"
        get_request_str += "    {\n"
        get_request_str += "    VERIFY_API\n\n"
        get_request_str += "    if ({0}Buffer == nullptr)\n".format(self.get_lower_name())
        get_request_str += "        {\n"
        get_request_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                           '        return {0};\n        }}\n\n'\
            .format("INVALID_PARAMETER",
                    self.__status_codes["INVALID_PARAMETER"].message,
                    "{0}Buffer is a nullptr.".format(self.get_lower_name()))
        get_request_str += '    Utf8String {0}Url = UrlProvider::Urls::{1}.Get();\n'\
            .format(self.__url_descriptor.lower(), self.__url_descriptor)
        get_request_str += '    if (api->m_repositoryClients.find({0}Url ' \
                           '+ "{1}") ' \
                           '== api->m_repositoryClients.end())\n'.format(self.__url_descriptor.lower(), self.__repository_id)
        get_request_str += '        {\n'
        get_request_str += '        api->CreateWSRepositoryClient\n'
        get_request_str += '            (\n'
        get_request_str += '            {0}Url,\n'.format(self.__url_descriptor.lower())
        get_request_str += '            "{0}"\n'.format(self.__repository_id)
        get_request_str += '            );\n'
        get_request_str += '        }\n\n'
        get_request_str += '    auto client = api->m_repositoryClients.find({0}Url ' \
                           '+ "{1}")->second;\n'.format(self.__url_descriptor.lower(), self.__repository_id)
        get_request_str += '    auto result = client->SendQueryRequest(WSQuery("{0}", "{1}"))->GetResult();\n'\
            .format(self.__ecschema_name, self.get_name())
        get_request_str += "    if (!result.IsSuccess())\n"
        get_request_str += "        return wsresultTo{0}Status(api, result.GetError().GetId(), result.GetError().GetDisplayMessage()," \
                           " result.GetError().GetDisplayDescription());\n\n"\
            .format(self.__api.get_api_name())
        get_request_str += "    {0}BUFFER* buf = ({0}BUFFER*) calloc(1, sizeof({0}BUFFER));\n".format(self.__api.get_api_acronym())
        get_request_str += "    if (buf == nullptr)\n"
        get_request_str += "        {\n"
        get_request_str += "        free(buf);\n"
        get_request_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                           '        return {0};\n' \
            .format("INTERNAL_MEMORY_ERROR",
                    self.__status_codes["INTERNAL_MEMORY_ERROR"].message,
                    "Failed to calloc memory for {0}BUFFER.".format(self.__api.get_api_acronym()))
        get_request_str += "        }\n\n"
        get_request_str += "    for (WSObjectsReader::Instance instance : result.GetValue().GetInstances())\n"
        get_request_str += "        {\n"
        get_request_str += "        LP{0}{1}BUFFER bufToFill = new {0}{1}BUFFER;\n"\
            .format(self.__api.get_api_acronym(), self.get_upper_name(), self.get_lower_name())
        get_request_str += "        {0}_BufferStuffer(bufToFill, instance.GetObjectId(), instance.GetProperties());\n".format(self.get_name())
        get_request_str += "        buf->lItems.push_back(bufToFill);\n"
        get_request_str += "        }\n\n"
        get_request_str += "    buf->lCount = buf->lItems.size();\n"
        get_request_str += "    buf->lClassType = BUFF_TYPE_{0};\n".format(self.get_upper_name())
        get_request_str += "    buf->lSchemaType = SCHEMA_TYPE_{0};\n".format(self.get_schema_name().upper())
        get_request_str += "    buf->isWSGBuffer = true;\n"
        get_request_str += "    *{0}Buffer = ({1}DATABUFHANDLE) buf;\n\n".format(self.get_lower_name(), self.__api.get_upper_api_acronym())
        get_request_str += '    api->SetObjectsResponse(result.GetValue());\n'
        get_request_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                           '    return {0};\n'.format("SUCCESS", self.__status_codes["SUCCESS"].message,
                                                      "{0}_Read{1}List completed successfully."
                                                      .format(self.__api.get_api_name(), self.get_name()))
        get_request_str += "    }\n"
        return get_request_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_create_class_function_def(self):
        create_request_str = "CallStatus {0}_Create{1}\n".format(self.__api.get_api_name(), self.get_name())
        create_request_str += "(\n"
        create_request_str += "{0}HANDLE apiHandle".format(self.__api.get_upper_api_acronym())
        create_request_str += self.__get_class_properties_for_function_def()
        create_request_str += self.__get_relationship_source_ids_for_function_def('create')
        create_request_str += "\n)"
        return create_request_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def __get_relationship_source_ids_for_function_def(self, function_type):
        str = ""               
        #if we have any relationships, we need to allow for id for the source object to be passed in (for example:
        # a parent object id under which this item will be created) 
        # TODO: Re-examine whether we can make such an assumption (to add an id) for any relationship encountered.
        #       It is not completely clear that is a safe make that assumption at this point.        
        if (len (self.__ecRelationshipclasses) > 0):                
                for ecr in self.__ecRelationshipclasses:   
                    if function_type == 'create':
                        if ecr.should_have_create():
                            str += self.__get_relationship_source_var_for_function_def(ecr)
                    elif function_type == 'read':
                        if ecr.should_have_read():
                            str += self.__get_relationship_source_var_for_function_def(ecr)
                    elif function_type == 'update':
                        if ecr.should_have_update():
                            str += self.__get_relationship_source_var_for_function_def(ecr)
                    elif function_type == 'delete':
                        if ecr.should_have_delete():
                            str += self.__get_relationship_source_var_for_function_def(ecr)
                    elif function_type == 'readall':
                        if ecr.should_have_readall():
                            str += self.__get_relationship_source_var_for_function_def(ecr)
        return str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_relationship_properties_implementation(self, function_type):
        str = ""               
        if (len (self.__ecRelationshipclasses) > 0):                
                for ecr in self.__ecRelationshipclasses:   
                    if function_type == 'create':
                        if ecr.should_have_create():
                            str += self.__get_relationship_properties_for_function_impl(ecr)
                    elif function_type == 'read':
                        if ecr.should_have_read():
                            str += self.__get_relationship_properties_for_function_impl(ecr)
                    elif function_type == 'update':
                        if ecr.should_have_update():
                            str += self.__get_relationship_properties_for_function_impl(ecr)
                    elif function_type == 'delete':
                        if ecr.should_have_delete():
                            str += self.__get_relationship_properties_for_function_impl(ecr)
                    elif function_type == 'readall':
                        if ecr.should_have_readall():
                            str += self.__get_relationship_properties_for_function_impl(ecr)
        return str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def __get_relationship_source_var_for_function_def(self, ecr):
        str = ",\n{0}DATABUFHANDLE {1}Buffer".format(self.__api.get_upper_api_acronym(), ecr.get_var_name())
        return str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_relationship_properties_for_function_impl(self, ecr):
        properties_str = "    if ({0}Buffer != nullptr)\n ".format(ecr.get_var_name())
        properties_str += "      {\n"
        properties_str += '       Json::Value relationshipInstancesJson;\n'
        
        # HCWSCCBUFFER buf = (HCWSCCBUFFER) dataBuffer; see ConnectWebServicesClientC_DataBufferGetStringLength
        # LPCWSCCFOLDERBUFFER folderBuf = (LPCWSCCFOLDERBUFFER) buf->lItems[index]; see Folder_GetStringLength
        properties_str += "       LP{0}{1}BUFFER buf = (LP{0}{1}BUFFER) ((HCWSCCBUFFER) {2}Buffer)->lItems[0];\n" \
                                    .format(self.__api.get_api_acronym(), ecr.get_upper_name(), ecr.get_var_name())
        properties_str += '       relationshipInstancesJson["schemaName"] = "{0}";\n'.format(ecr.get_schema_name())
        properties_str += '       relationshipInstancesJson["className"] = "{0}";\n'.format(ecr.get_name())
        properties_str += '       relationshipInstancesJson["direction"] = "backward" /*TODO: figure out what direction is used for*/;\n\n'.format(ecr.get_name())
        if (len(ecr.get_properties()) > 1):
            properties_str += '       Json::Value relationshipInstancesPropertiesJson;\n'        
            for ecproperty in ecr.get_properties():
                if ecproperty.should_be_excluded:
                    continue
                if ecproperty.is_read_only:
                    continue
                #the "{SourceObject}Id"" will be set as the instanceid. Just needed a way for them to tell it to me.
                if (ecproperty.name == "{0}Id", ecr.get_source().get_name()):  
                    continue
                properties_str += "       if ({0}Buffer.{1} != nullptr) ".format(ecr.get_var_name(), ecproperty.name)
                property_type = ecproperty.type
                if property_type == "guid":
                    properties_str += 'relationshipInstancesPropertiesJson["{0}"] = Utf8String({1}Buffer.{0});\n' \
                        .format(ecproperty.name, ecr.get_var_name())
                elif property_type == "dateTime":
                    properties_str += 'relationshipInstancesPropertiesJson["{0}"] = Utf8String({1}Buffer.{0});\n' \
                        .format(ecproperty.name, ecr.get_var_name())
                elif property_type == "string":
                    properties_str += 'relationshipInstancesPropertiesJson["{0}"] = Utf8String({1}Buffer.{0});\n' \
                        .format(ecproperty.name, ecr.get_var_name())
                else:
                    properties_str += 'relationshipInstancesPropertiesJson["{0}"] = *{1}Buffer.{0};\n'.format(ecproperty.name, ecr.get_var_name())
            properties_str += '       if (relationshipInstancesPropertiesJson.size() == 0)\n'
            properties_str += '          {\n'
            properties_str += '          relationshipInstancesJson["properties"] = relationshipInstancesPropertiesJson;\n' \
                            '          }}\n\n'
        properties_str += '       Json::Value relatedInstanceJson;\n'                     
        properties_str += '       relatedInstanceJson["schemaName"] = "{0}";\n'.format(ecr.get_schema_name())
        properties_str += '       relatedInstanceJson["className"] = "{0}";\n'.format(ecr.get_source().get_name())  
        properties_str += '       relatedInstanceJson["instanceId"] = Utf8String(buf->{0}Id);\n'.format(ecr.get_source().get_name())     
        properties_str += '       relationshipInstancesJson["relatedInstance"] = relatedInstanceJson;\n' 
        properties_str += '       if (relationshipInstancesJson.size() == 0)\n'
        properties_str += '          {\n'
        properties_str += '          api->SetStatusMessage("{1}");\n' \
                          '          api->SetStatusDescription("{2}");\n' \
                          '          return {0};\n' \
                          '          }}\n'\
            .format("INVALID_PARAMETER",
                    self.__status_codes["INVALID_PARAMETER"].message,
                    "There were not any valid {0} properties passed in.".format(ecr.get_name()))
        properties_str += '      instance["relationshipInstances"] = Json::Value(Json::arrayValue);\n'
        properties_str += '      instance["relationshipInstances"].append(relationshipInstancesJson);\n'                      
        properties_str += "      }\n\n"
        return properties_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_create_class_definition(self):
        return "{0}_EXPORT ".format(self.__api.get_upper_api_acronym()) + self.__get_create_class_function_def() + ";\n"

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_create_class_implementation(self):
        create_request_str = self.__get_create_class_function_def() + "\n"
        create_request_str += "    {\n"
        create_request_str += "    VERIFY_API\n\n"
        create_request_str += "    Json::Value instance;\n"
        create_request_str += '    instance["schemaName"] = "{0}";\n'.format(self.__ecschema_name)
        create_request_str += '    instance["className"] = "{0}";\n\n'.format(self.get_name())
        create_request_str += self.__get_class_properties_for_function_impl()
        create_request_str += '    instance["properties"] = propertiesJson;\n\n'            
        create_request_str += self.get_relationship_properties_implementation('create')
        create_request_str += '    Json::Value objectCreationJson;\n'
        create_request_str += '    objectCreationJson["instance"] = instance;\n\n'
        create_request_str += '    Utf8String {0}Url = UrlProvider::Urls::{1}.Get();\n'\
            .format(self.__url_descriptor.lower(), self.__url_descriptor)
        create_request_str += '    if (api->m_repositoryClients.find({0}Url ' \
                              '+ "{1}") ' \
                              '== api->m_repositoryClients.end())\n'\
            .format(self.__url_descriptor.lower(), self.__repository_id)
        create_request_str += '        {\n'
        create_request_str += '        api->CreateWSRepositoryClient\n'
        create_request_str += '            (\n'
        create_request_str += '            {0}Url,\n'.format(self.__url_descriptor.lower())
        create_request_str += '            "{0}"\n'.format(self.__repository_id)
        create_request_str += '            );\n'
        create_request_str += '        }\n\n'
        create_request_str += '    auto client = api->m_repositoryClients.find({0}Url ' \
                              '+ "{1}")->second;\n'.format(self.__url_descriptor.lower(), self.__repository_id)
        create_request_str += '    auto result = client->SendCreateObjectRequest(objectCreationJson)->GetResult();\n'
        create_request_str += "    if (!result.IsSuccess())\n"
        create_request_str += "        return wsresultTo{0}Status(api, result.GetError().GetId(), result.GetError().GetDisplayMessage()," \
                              " result.GetError().GetDisplayDescription());\n\n"\
            .format(self.__api.get_api_name())
        create_request_str += '    api->SetCreatedObjectResponse(result.GetValue());\n'
        create_request_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                              '    return {0};\n'\
            .format("SUCCESS", self.__status_codes["SUCCESS"].message,
                    "{0}_Create{1} completed successfully.".format(self.__api.get_api_name(), self.get_name()))
        create_request_str += "    }\n"
        return create_request_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_read_class_function_def(self):
        read_request_str = "CallStatus {0}_Read{1}\n".format(self.__api.get_api_name(), self.get_name())
        read_request_str += "(\n"
        read_request_str += "{0}HANDLE apiHandle,\n".format(self.__api.get_upper_api_acronym())
        read_request_str += "WCharCP {0}Id,\n".format(self.get_lower_name())
        read_request_str += "{0}DATABUFHANDLE* {1}Buffer\n".format(self.__api.get_upper_api_acronym(), self.get_lower_name())
        read_request_str += ")"
        return read_request_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_read_class_definition(self):
        return "{0}_EXPORT ".format(self.__api.get_upper_api_acronym()) + self.__get_read_class_function_def() + ";\n"

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_read_class_implementation(self):
        get_request_str = self.__get_read_class_function_def() + "\n"
        get_request_str += "    {\n"
        get_request_str += "    VERIFY_API\n\n"
        get_request_str += "    if ({0}Buffer == nullptr || {0}Id == nullptr || wcslen({0}Id) == 0)\n".format(self.get_lower_name())
        get_request_str += "        {\n"
        get_request_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                           '        return {0};\n        }}\n\n'\
            .format("INVALID_PARAMETER", self.__status_codes["INVALID_PARAMETER"].message,
                    "{0}Buffer is a nullptr or {0}Id is nullptr or empty.".format(self.get_lower_name()))
        get_request_str += '    Utf8String {0}Url = UrlProvider::Urls::{1}.Get();\n'\
            .format(self.__url_descriptor.lower(), self.__url_descriptor)
        get_request_str += '    if (api->m_repositoryClients.find({0}Url ' \
                           '+ "{1}") ' \
                           '== api->m_repositoryClients.end())\n'.format(self.__url_descriptor.lower(), self.__repository_id)
        get_request_str += '        {\n'
        get_request_str += '        api->CreateWSRepositoryClient\n'
        get_request_str += '            (\n'
        get_request_str += '            {0}Url,\n'.format(self.__url_descriptor.lower())
        get_request_str += '            "{0}"\n'.format(self.__repository_id)
        get_request_str += '            );\n'
        get_request_str += '        }\n\n'
        get_request_str += '    auto client = api->m_repositoryClients.find({0}Url ' \
                           '+ "{1}")->second;\n'.format(self.__url_descriptor.lower(), self.__repository_id)
        get_request_str += '    auto result = client->'
        get_request_str += 'SendGetObjectRequest({{"{0}", "{1}", Utf8String({2}Id)}})->GetResult();\n'\
            .format(self.__ecschema_name, self.get_name(), self.get_lower_name())
        get_request_str += "    if (!result.IsSuccess())\n"
        get_request_str += "        return wsresultTo{0}Status(api, result.GetError().GetId(), result.GetError().GetDisplayMessage()," \
                           " result.GetError().GetDisplayDescription());\n\n"\
            .format(self.__api.get_api_name())
        get_request_str += "    LP{0}{1}BUFFER {2}Buf = new {0}{1}BUFFER;\n" \
            .format(self.__api.get_api_acronym(), self.get_upper_name(), self.get_lower_name())
        get_request_str += "    WSObjectsReader::Instance instance = (*result.GetValue().GetInstances().begin());\n "            
        get_request_str += "   {0}_BufferStuffer({1}Buf, instance.GetObjectId(), instance.GetProperties());\n\n"\
            .format(self.get_name(), self.get_lower_name())
        get_request_str += "    {0}BUFFER* buf = ({0}BUFFER*) calloc(1, sizeof({0}BUFFER));\n".format(self.__api.get_api_acronym())
        get_request_str += "    if (buf == nullptr)\n"
        get_request_str += "        {\n"
        get_request_str += "        free({0}Buf);\n".format(self.get_lower_name())
        get_request_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                           '        return {0};\n' \
            .format("INTERNAL_MEMORY_ERROR",
                    self.__status_codes["INTERNAL_MEMORY_ERROR"].message,
                    "Failed to calloc memory for {0}BUFFER.".format(self.__api.get_api_acronym()))
        get_request_str += "        }\n\n"
        get_request_str += "    buf->lCount = 1;\n"
        get_request_str += "    buf->lClassType = BUFF_TYPE_{0};\n".format(self.get_upper_name())
        get_request_str += "    buf->lSchemaType = SCHEMA_TYPE_{0};\n".format(self.get_schema_name().upper())
        get_request_str += "    buf->isWSGBuffer = true;\n"
        get_request_str += "    buf->lItems = {{{0}Buf}};\n".format(self.get_lower_name())
        get_request_str += "    *{0}Buffer = ({1}DATABUFHANDLE) buf;\n\n".format(self.get_lower_name(), self.__api.get_upper_api_acronym())
        get_request_str += '    api->SetObjectsResponse(result.GetValue());\n'
        get_request_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                           '    return {0};\n'.format("SUCCESS", self.__status_codes["SUCCESS"].message,
                                                      "{0}_Read{1} completed successfully."
                                                      .format(self.__api.get_api_name(), self.get_name()))
        get_request_str += "    }\n"
        return get_request_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_update_class_function_def(self):
        update_request_str = "CallStatus {0}_Update{1}\n".format(self.__api.get_api_name(), self.get_name())
        update_request_str += "(\n"
        update_request_str += "{0}HANDLE apiHandle,\n".format(self.__api.get_upper_api_acronym())
        update_request_str += "WCharCP {0}Id".format(self.get_lower_name())
        update_request_str += self.__get_class_properties_for_function_def()
        update_request_str += "\n)"
        return update_request_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_update_class_definition(self):
        return "{0}_EXPORT ".format(self.__api.get_upper_api_acronym()) + self.__get_update_class_function_def() + ";\n"

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_update_class_implementation(self):
        update_request_str = self.__get_update_class_function_def() + "\n"
        update_request_str += "    {\n"
        update_request_str += "    VERIFY_API\n\n"
        update_request_str += self.__get_class_properties_for_function_impl()
        update_request_str += '    Utf8String {0}Url = UrlProvider::Urls::{1}.Get();\n'\
            .format(self.__url_descriptor.lower(), self.__url_descriptor)
        update_request_str += '    if (api->m_repositoryClients.find({0}Url ' \
                              '+ "{1}") ' \
                              '== api->m_repositoryClients.end())\n'.format(self.__url_descriptor.lower(), self.__repository_id)
        update_request_str += '        {\n'
        update_request_str += '        api->CreateWSRepositoryClient\n'
        update_request_str += '            (\n'
        update_request_str += '            {0}Url,\n'.format(self.__url_descriptor.lower())
        update_request_str += '            "{0}"\n'.format(self.__repository_id)
        update_request_str += '            );\n'
        update_request_str += '        }\n\n'
        update_request_str += '    auto client = api->m_repositoryClients.find({0}Url ' \
                              '+ "{1}")->second;\n'.format(self.__url_descriptor.lower(), self.__repository_id)
        update_request_str += '    auto result = client->SendUpdateObjectRequest({{"{0}", "{1}", Utf8String({2}Id)}}, '\
            .format(self.__ecschema_name, self.get_name(), self.get_lower_name())
        update_request_str += 'propertiesJson)->GetResult();\n'
        update_request_str += "    if (!result.IsSuccess())\n"
        update_request_str += "        return wsresultTo{0}Status(api, result.GetError().GetId(), result.GetError().GetDisplayMessage()," \
                              " result.GetError().GetDisplayDescription());\n\n"\
            .format(self.__api.get_api_name())
        update_request_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                              '    return {0};\n'\
            .format("SUCCESS", self.__status_codes["SUCCESS"].message,
                    "{0}_Update{1} completed successfully.".format(self.__api.get_api_name(), self.get_name()))
        update_request_str += "    }\n"
        return update_request_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_delete_class_function_def(self):
        delete_request_str = "CallStatus {0}_Delete{1}\n".format(self.__api.get_api_name(), self.get_name())
        delete_request_str += "(\n"
        delete_request_str += "{0}HANDLE apiHandle,\n".format(self.__api.get_upper_api_acronym())
        delete_request_str += "WCharCP {0}Id\n".format(self.get_lower_name())
        delete_request_str += ")"
        return delete_request_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_delete_class_definition(self):
        return "{0}_EXPORT ".format(self.__api.get_upper_api_acronym()) + self.__get_delete_class_function_def() + ";\n"

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_delete_class_implementation(self):
        delete_request_str = self.__get_delete_class_function_def() + "\n"
        delete_request_str += "    {\n"
        delete_request_str += "    VERIFY_API\n\n"
        delete_request_str += "    if ({0}Id == nullptr || wcslen({0}Id) == 0)\n".format(self.get_lower_name())
        delete_request_str += "        {\n"
        delete_request_str += '        api->SetStatusMessage("{1}");\n         api->SetStatusDescription("{2}");\n' \
                              '        return {0};\n        }}\n\n'\
            .format("INVALID_PARAMETER", self.__status_codes["INVALID_PARAMETER"].message,
                    "{0}Id is a nullptr or empty.".format(self.get_lower_name()))
        delete_request_str += '    Utf8String {0}Url = UrlProvider::Urls::{1}.Get();\n'\
            .format(self.__url_descriptor.lower(), self.__url_descriptor)
        delete_request_str += '    if (api->m_repositoryClients.find({0}Url ' \
                              '+ "{1}") ' \
                              '== api->m_repositoryClients.end())\n'.format(self.__url_descriptor.lower(), self.__repository_id)
        delete_request_str += '        {\n'
        delete_request_str += '        api->CreateWSRepositoryClient\n'
        delete_request_str += '            (\n'
        delete_request_str += '            {0}Url,\n'.format(self.__url_descriptor.lower())
        delete_request_str += '            "{0}"\n'.format(self.__repository_id)
        delete_request_str += '            );\n'
        delete_request_str += '        }\n\n'
        delete_request_str += '    auto client = api->m_repositoryClients.find({0}Url ' \
                              '+ "{1}")->second;\n'.format(self.__url_descriptor.lower(), self.__repository_id)
        delete_request_str += '    auto result = client->'
        delete_request_str += 'SendDeleteObjectRequest({{"{0}", "{1}", Utf8String({2}Id)}})->GetResult();\n' \
            .format(self.__ecschema_name, self.get_name(), self.get_lower_name())
        delete_request_str += "    if (!result.IsSuccess())\n"
        delete_request_str += "        return wsresultTo{0}Status(api, result.GetError().GetId(), result.GetError().GetDisplayMessage()," \
                              " result.GetError().GetDisplayDescription());\n\n"\
            .format(self.__api.get_api_name())
        delete_request_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                              '    return {0};\n'\
            .format("SUCCESS",
                    self.__status_codes["SUCCESS"].message,
                    "{0}_Delete{1} completed successfully.".format(self.__api.get_api_name(), self.get_name()))
        delete_request_str += "    }\n"
        return delete_request_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_class_properties_for_function_def(self):
        property_str = ""
        for ecproperty in self.get_properties():
            if ecproperty.should_be_excluded:
                continue
            if ecproperty.is_read_only:
                continue
            property_str += ',\n'
            property_type = ecproperty.type
            if property_type == "string":
                property_str += "WCharCP "
            elif property_type == "dateTime":
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
                property_str += ecproperty.type + "* "
            property_str += ecproperty.name
        return property_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_class_properties_for_function_impl(self):
        properties_str = '    Json::Value propertiesJson;\n'
        for ecproperty in self.get_properties():
            if ecproperty.should_be_excluded:
                continue
            if ecproperty.is_read_only:
                continue
            properties_str += "    if ({0} != nullptr) ".format(ecproperty.name)
            property_type = ecproperty.type
            if property_type == "guid":
                properties_str += 'propertiesJson["{0}"] = Utf8String({0});\n' \
                    .format(ecproperty.name)
            elif property_type == "dateTime":
                properties_str += 'propertiesJson["{0}"] = Utf8String({0});\n' \
                    .format(ecproperty.name)
            elif property_type == "string":
                properties_str += 'propertiesJson["{0}"] = Utf8String({0});\n' \
                    .format(ecproperty.name)
            else:
                properties_str += 'propertiesJson["{0}"] = *{0};\n'.format(ecproperty.name)
        properties_str += '    if (propertiesJson.size() == 0)\n'
        properties_str += '        {\n'
        properties_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                          '        return {0};\n        }}\n'\
            .format("INVALID_PARAMETER",
                    self.__status_codes["INVALID_PARAMETER"].message,
                    "There were not any valid {0} properties passed in.".format(self.get_name()))
        return properties_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_buffer_accessor_functions_def(self, property_type):
        if not self.does_contain_property_type(property_type):
            raise PropertyTypeError("Property type {0} not accepted".format(property_type))
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
        elif property_type == "dateTime":
            accessor_str += "uint32_t strLength,\n"
            accessor_str += "WCharP dateTime\n"
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

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_buffer_accessor_function_definition(self, property_type):
        return self.__get_buffer_accessor_functions_def(property_type) + ";\n"

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_buffer_accessor_function_implementation(self, property_type):
        accessor_str = self.__get_buffer_accessor_functions_def(property_type) + "\n"
        accessor_str += "    {\n"
        accessor_str += "    if (buf == nullptr || bufferProperty == 0"
        if property_type == "string":
            accessor_str += " || str == nullptr || strLength == 0"
        elif property_type == "StringLength":
            accessor_str += " || outStringSize == nullptr"
        elif property_type == "dateTime":
            accessor_str += " || dateTime == nullptr || strLength == 0"
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
                        '        return {0};\n        }}\n\n' \
            .format("INVALID_PARAMETER",
                    self.__status_codes["INVALID_PARAMETER"].message,
                    "An invalid buffer pointer or invalid property pointer was passed into the get property function.")
        accessor_str += '    if(index >= buf->lCount)\n'
        accessor_str += '        {\n'
        accessor_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                        '        return {0};\n        }}\n\n' \
            .format("INVALID_PARAMETER",
                    self.__status_codes["INVALID_PARAMETER"].message,
                    "The index parameter passed into the get property function is out of bounds.")
        accessor_str += "    LP{0}{1}BUFFER {2}Buf = (LP{0}{1}BUFFER) buf->lItems[index];\n" \
            .format(self.__api.get_api_acronym(), self.get_upper_name(), self.get_lower_name())

        if (property_type == 'string'):
            accessor_str += "    if ({0}_BUFF_{1} == bufferProperty)\n".format(self.get_upper_name(),"OBJECTID")
            accessor_str += "       {\n"                    
            accessor_str += "       BeStringUtilities::Wcsncpy(str, strLength, {0}Buf->{1}.c_str());\n".format(self.get_lower_name(), "ObjectId")
            accessor_str += "       return {0};\n".format("SUCCESS")
            accessor_str += "       }\n\n"
        
        is_first_property = True
        for ecproperty in self.get_properties():
            if ecproperty.should_be_excluded:
                continue
            if ecproperty.type == property_type or \
                    (ecproperty.type == 'string' and property_type == 'StringLength'):

                if is_first_property:
                    accessor_str += "    if ({0}_BUFF_{1} == bufferProperty)\n".format(self.get_upper_name(),
                                                                                       ecproperty.name.upper())
                    is_first_property = False
                else:
                    accessor_str += "    else if ({0}_BUFF_{1} == bufferProperty)\n".format(self.get_upper_name(),
                                                                                            ecproperty.name.upper())
                accessor_str += "        {\n"
                property_access_str = '        if ({0}Buf->IsSet.find(WString("{1}", true)) == {0}Buf->IsSet.end() '
                property_access_str += '|| {0}Buf->IsSet[WString("{1}", true)] == false)\n'
                property_access_str += '            {{\n'
                if property_type == "string":
                    property_access_str += '            str = nullptr;\n'
                elif property_type == "StringLength":
                    property_access_str += '            outStringSize = nullptr;\n'
                elif property_type == "dateTime":
                    property_access_str += '            dateTime = nullptr;\n'
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
                elif property_type == "dateTime":
                    property_access_str += "        BeStringUtilities::Wcsncpy(dateTime, strLength, {0}Buf->{1}.c_str());\n"
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
                accessor_str += property_access_str.format(self.get_lower_name(), ecproperty.name,
                                                           'api->SetStatusMessage("{1}");\n'
                                                           '            api->SetStatusDescription("{2}");\n'
                                                           '            return {0}'
                                                           .format("PROPERTY_HAS_NOT_BEEN_SET",
                                                                   self.__status_codes['PROPERTY_HAS_NOT_BEEN_SET'].message,
                                                                   "{0} property is not set, so it can not be retrieved."
                                                                   .format(ecproperty.name)))
                accessor_str += "        }\n"
        if not is_first_property:
            accessor_str += "    else\n"
            accessor_str += '        {\n'
            accessor_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                            '        return {0};\n        }}\n' \
                .format("INVALID_PARAMETER",
                        self.__status_codes["INVALID_PARAMETER"].message,
                        "The bufferProperty is invalid. It did not match up with any of the buffer's properties.")
        accessor_str += '    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n' \
                        '    return {0};\n' \
            .format("SUCCESS",
                    self.__status_codes["SUCCESS"].message,
                    "The property retrieval function completed successfully.")
        accessor_str += "    }\n"
        return accessor_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_struct_typedef(self):
        struct_str = 'typedef struct _' + self.__api.get_api_acronym() + '_'
        struct_str += self.get_upper_name() + '_BUFFER \n'
        struct_str += '    {\n'
        struct_str += '    bmap<WString, bool> IsSet;\n'
        struct_str += '    WString ObjectId;\n'
        for ecproperty in self.get_properties():
            if ecproperty.should_be_excluded:
                continue
            if ecproperty.type == "string":
                struct_str += "    WString "
            elif ecproperty.type == "dateTime":
                struct_str += "    WString "
            elif ecproperty.type == "guid":
                struct_str += "    WString "
            elif ecproperty.type == "boolean":
                struct_str += "    bool "
            elif ecproperty.type == "int":
                struct_str += "    int32_t "
            elif ecproperty.type == "long":
                struct_str += "    int64_t "
            else:
                struct_str += "    " + ecproperty.type + " "
            struct_str += ecproperty.name + ";\n"
        struct_str += "    } " + self.__api.get_api_acronym() + self.get_upper_name()
        struct_str += "BUFFER, *LP" + self.__api.get_api_acronym()
        struct_str += self.get_upper_name() + "BUFFER;\n"
        return struct_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_enum(self):
        enum_str = "typedef enum\n"
        enum_str += "    {\n"
        enum_count = 1
        #add the objectid
        enum_str += "    {0}_BUFF_{1:30} = {2}, /**< \\b {3}. */\n".format(self.get_upper_name(), "OBJECTID", enum_count, "ObjectId (generated property for instance id) ")
        enum_count += 1
        #add other properties
        for ecproperty in self.get_properties():
            if ecproperty.should_be_excluded:
                continue
            enum_str += "    {0}_BUFF_{1:30} = {2}, /**< \\b {3}. */\n"\
                .format(self.get_upper_name(), ecproperty.name.upper(), enum_count,
                        ecproperty.name)
            enum_count += 1
        enum_str += "    }} {0}_BUFF_PROPERTY;\n".format(self.get_upper_name())
        return enum_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_buffer_stuffer_function_def(self):
        stuffer_str = "void {0}_BufferStuffer\n".format(self.get_name())
        stuffer_str += "(\n"
        stuffer_str += "LP{0}{1}BUFFER {2}Buf,\n".format(self.__api.get_api_acronym(), self.get_upper_name(), self.get_lower_name())
        stuffer_str += "ObjectIdCR objectId,\n"
        stuffer_str += "RapidJsonValueCR properties\n"
        stuffer_str += ")"
        return stuffer_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_buffer_stuffer_function_definition(self):
        return self.__get_buffer_stuffer_function_def() + ';\n'

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_buffer_stuffer_function_implementation(self):
        stuffer_str = self.__get_buffer_stuffer_function_def() + '\n'
        stuffer_str += "    {\n"
        stuffer_str += "    {0}Buf->ObjectId = WString(objectId.GetRemoteId().c_str(), true);\n".format(self.get_lower_name())
        for ecproperty in self.get_properties():
            if ecproperty.should_be_excluded:
                continue
            if ecproperty.type == "string":
                stuffer_str += '    if(properties.HasMember("{0}") && properties["{0}"].IsString())\n' \
                    .format(ecproperty.name)
                stuffer_str += '        {0}Buf->{1} = WString(properties["{1}"].GetString(), true);\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = (properties.HasMember("{1}") && properties["{1}"].IsString());\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            elif ecproperty.type == "dateTime":
                stuffer_str += '    if(properties.HasMember("{0}") && properties["{0}"].IsString())\n' \
                    .format(ecproperty.name)
                stuffer_str += '        {0}Buf->{1} = WString(properties["{1}"].GetString(), true);\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = (properties.HasMember("{1}") && properties["{1}"].IsString());\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            elif ecproperty.type == "guid":
                stuffer_str += '    if(properties.HasMember("{0}") && properties["{0}"].IsString())\n' \
                    .format(ecproperty.name)
                stuffer_str += '        {0}Buf->{1} = WString(properties["{1}"].GetString(), true);\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = (properties.HasMember("{1}") && properties["{1}"].IsString());\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            elif ecproperty.type == "boolean":
                stuffer_str += '    if(properties.HasMember("{0}") && properties["{0}"].IsBool())\n' \
                    .format(ecproperty.name)
                stuffer_str += '        {0}Buf->{1} = properties["{1}"].GetBool();\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = (properties.HasMember("{1}") && properties["{1}"].IsBool());\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            elif ecproperty.type == "int" or ecproperty.type == "long":
                stuffer_str += '    if(properties.HasMember("{0}") && properties["{0}"].IsInt())\n' \
                    .format(ecproperty.name)
                stuffer_str += '        {0}Buf->{1} = properties["{1}"].GetInt();\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = (properties.HasMember("{1}") && properties["{1}"].IsInt());\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            elif ecproperty.type == "double":
                stuffer_str += '    if(properties.HasMember("{0}") && properties["{0}"].IsDouble())\n' \
                    .format(ecproperty.name)
                stuffer_str += '        {0}Buf->{1} = properties["{1}"].GetDouble();\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = (properties.HasMember("{1}") && properties["{1}"].IsDouble());\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            else:
                raise PropertyTypeError("Property type {0} not accepted".format(ecproperty.type))
        stuffer_str += "    }\n"
        return stuffer_str
