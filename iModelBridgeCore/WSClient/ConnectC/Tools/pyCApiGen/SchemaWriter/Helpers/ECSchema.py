#!Python
#--------------------------------------------------------------------------------------
#
#     $Source: ConnectC/Tools/pyCApiGen/SchemaWriter/Helpers/ECSchema.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
from SchemaWriter.Helpers.ECClass import ECClass
from SchemaWriter.Helpers.ECProperty import ECProperty
from SchemaWriter.Helpers.PropertyTypeError import PropertyTypeError
from SchemaWriter.Helpers.ECRelationshipClass import ECRelObject
from SchemaWriter.Helpers.ECRelationshipClass import ECRelSource
from SchemaWriter.Helpers.ECRelationshipClass import ECRelTarget
from SchemaWriter.Helpers.ECRelationshipClass import ECRelationshipClass

#-------------------------------------------------------------------------------------------
# bsiclass                                                            2016
#-------------------------------------------------------------------------------------------
class ECSchema(object):
    def __init__(self, url_descriptor, repository_id, filename, name, api, status_codes):
        self.__url_descriptor = url_descriptor
        self.__repository_id = repository_id
        self.__filename = filename
        self.__name = name
        self.__api = api
        self.__status_codes = status_codes
        self.__ecclasses = []
        self.__ecRelationshipclasses = []
        self.__ecRelationshipSpecs = {}
        self.__ecschema_xmldoc = None

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def get_filename(self):
        return self.__filename

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def get_name(self):
        return self.__name

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def get_upper_name(self):
        return self.get_name().upper()

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def get_lower_name(self):
        return self.get_name().lower()

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def get_url_descriptor(self):
        return self.__url_descriptor

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def get_repository_id(self):
        return self.__repository_id

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def add_ecclass(self, autogenclass_row):
        self.__ecclasses.append(ECClass(autogenclass_row, self.__api, self.__status_codes))

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def get_classes(self):
        return self.__ecclasses

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def has_ecclass_with_property_type(self, property_type):
        for ecclass in self.get_classes():
            if ecclass.does_contain_property_type(property_type):
                return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def has_included_ecclass_with_property_type(self, property_type):
        for ecclass in self.get_classes():
            if not ecclass.should_exclude_entire_class() and ecclass.does_contain_property_type(property_type) :
                return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest       04/2017
    #-------------------------------------------------------------------------------------------
    def get_relationships(self):
        return self.__ecRelationshipclasses

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest       04/2017
    #-------------------------------------------------------------------------------------------
    def has_ecrelationship_with_property_type(self, property_type):
        for ecrelclass in self.get_relationships():
            if ecrelclass.does_contain_property_type(property_type):
                return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def init_xml(self, xmldoc):
        self.__ecschema_xmldoc = xmldoc
        self._read_ecrelationshipclasses_from_xml()
        ecclasses = self.__ecschema_xmldoc.getElementsByTagName('ECClass')
        for ecclass_xml in ecclasses:
            ecclass = self.__get_ecclass_from_name(ecclass_xml.attributes['typeName'].value)
            ecclass.init_xml(ecclass_xml)
            ecclass.set_relationship_class_list(self.__get_relationships_that_target_class(ecclass.get_name()))
        
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def __get_relationships_that_target_class(self, target_class_name):
            relationships = []
            matching = [s for s in self.__ecRelationshipclasses if s.get_target().relates_to_object(target_class_name) ]                                    
            if (len(matching) > 0):
                return matching
            return relationships
            
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def _read_ecrelationshipclasses_from_xml(self):        
        #init relationship class
        ecRelationshipclasses = self.__ecschema_xmldoc.getElementsByTagName('ECRelationshipClass')
        if (None is ecRelationshipclasses):
            return            
        for xml_ecrclass in ecRelationshipclasses:
            self.__ecRelationshipclasses.append(self._create_ecrelationship_class(xml_ecrclass))

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def _create_ecrelationship_class(self, xml):
            source_xml = xml.getElementsByTagName('Source')
            target_xml = xml.getElementsByTagName('Target')
            if (source_xml is None or target_xml is None):
                return None
            
            # source info
            src_cardinality = source_xml[0].attributes["cardinality"].value
            src_polymorphic = source_xml[0].attributes["polymorphic"].value
            # get the relationship object (only supporting classes right now. Refactor for more)
            # there can only be one source, I believe. so just need the first one.
            rel_object_xml = source_xml[0].getElementsByTagName('Class')
            src_rel_object = ECRelObject(rel_object_xml[0].attributes["class"].value, "class")
            ec_rel_source = ECRelSource(src_cardinality, src_polymorphic, src_rel_object)

            # target info
            tgt_cardinality = target_xml[0].attributes["cardinality"].value
            tgt_polymorphic = target_xml[0].attributes["polymorphic"].value
            # you can definitely have multiple target objects
            rel_object_xml = target_xml[0].getElementsByTagName('Class')
            tgt_rel_objects = []
            for obj_xml in rel_object_xml:
                tgt_rel_objects.append(ECRelObject(obj_xml.attributes["class"].value, "class"))
            ec_rel_target = ECRelTarget(tgt_cardinality, tgt_polymorphic, tgt_rel_objects);

            # now set up the class
            schema_name = self.get_name()
            name = xml.attributes["typeName"].value
            is_domain_class = xml.attributes["typeName"].value
            strength = xml.attributes["typeName"].value
            strenth_direction = xml.attributes["typeName"].value
            ec_rel_class = ECRelationshipClass( self.__api,
                                                self.__status_codes,
                                                schema_name,
                                                name, 
                                                is_domain_class, 
                                                strength, 
                                                strenth_direction, 
                                                ec_rel_source, 
                                                ec_rel_target);
            
            ecproperties_xml = xml.getElementsByTagName('ECProperty')
            for ecproperty_xml in ecproperties_xml:
                ec_rel_class.add_property(
                        ECProperty( ecproperty_xml.attributes["propertyName"].value,
                                    ecproperty_xml.attributes["typeName"].value,
                                    ecproperty_xml.hasAttribute("readOnly") and ecproperty_xml.attributes["readOnly"].value,
                                    False ))
                                
            #set which functions should include a buffer for this relationship
            for ecRelClassSpec in self.__ecRelationshipSpecs:
                item = self.__ecRelationshipSpecs.get(ec_rel_class.get_name(), None)
                if(item != None):
                    ec_rel_class.set_include(item[0], item[1], item[2], item[3], item[4])
                    
            return ec_rel_class

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def add_ecrelationship_specification(self, name, on_create, on_read, on_update, on_delete, on_readall):
        self.__ecRelationshipSpecs[name] = \
                            [on_create, on_read, on_update, on_delete, on_readall]

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def __get_ecclass_from_name(self, name):
        for ecclass in self.__ecclasses:
            if ecclass.get_name() == name:
                return ecclass
        raise NameError("The name passed into __get_ecclass_from_name does not exist in the ecclasses array. Please make sure that all"
                        " ecclasses which are to be generated from the schemaFile ({0}), exist as a row in the autoGenClasses.xlsx file."
                        .format(self.get_name()))

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
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
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def get_buffer_accessor_function_definition(self, property_type):
        return self.__get_buffer_accessor_function_def(property_type) + ';\n'

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
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
            elif property_type == "dateTime":
                accessor_str += "(api, buf, bufferProperty, index, strLength, dateTime);\n"
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

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def __get_buffer_free_function_def(self):
        free_str = "CallStatus {0}_DataBufferFree\n".format(self.get_name())
        free_str += "(\n"
        free_str += "LP{0} api,\nH{0}BUFFER buf\n".format(self.__api.get_upper_api_acronym())
        free_str += ")"
        return free_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                             06/2016
    #-------------------------------------------------------------------------------------------
    def get_buffer_free_function_implementation(self):
        free_str = self.__get_buffer_free_function_def() + '\n'
        free_str += "    {\n"
        free_str += "    for (uint64_t index = 0; index < buf->lItems.size(); index++)\n"
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
        for ecrelclass in self.get_relationships():
            if ecrelclass.should_exclude_entire_class():
                continue
            free_str += "                case BUFF_TYPE_{0}:\n".format(ecrelclass.get_upper_name())
            free_str += "                    {\n"
            free_str += "                    LP{0}{1}BUFFER {2}Buf = (LP{0}{1}BUFFER) buf->lItems[index];\n"\
                .format(self.__api.get_upper_api_acronym(), ecrelclass.get_upper_name(), ecrelclass.get_lower_name())
            free_str += "                    delete {0}Buf;\n".format(ecrelclass.get_lower_name())
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
