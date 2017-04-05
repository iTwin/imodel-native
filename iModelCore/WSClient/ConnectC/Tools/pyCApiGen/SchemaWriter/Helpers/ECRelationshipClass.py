#!Python
#--------------------------------------------------------------------------------------
#
#     $Source: ConnectC/Tools/pyCApiGen/SchemaWriter/Helpers/ECRelationshipClass.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
from SchemaWriter.Helpers.ECProperty import ECProperty

#-------------------------------------------------------------------------------------------
# bsiclass                                      Robert.Priest      04/2017
#-------------------------------------------------------------------------------------------
class ECRelObject (object):
    
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, type):
        self.name = name
        self.type = type

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_name(self):
        return self.name
        
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_type(self):
        return self.type
        
#-------------------------------------------------------------------------------------------
# bsiclass                                      Robert.Priest      04/2017
#-------------------------------------------------------------------------------------------
class ECRelSource(object):
    
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def __init__(self, cardinality, polymorphic, relationship_object):
        self.cardinality = cardinality
        self.polymorphic = polymorphic
        self.relationship_object = relationship_object

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_name(self):
        return self.relationship_object.get_name()

#-------------------------------------------------------------------------------------------
# bsiclass                                      Robert.Priest      04/2017
#-------------------------------------------------------------------------------------------
class ECRelTarget(object):
    
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def __init__(self, cardinality, polymorphic, relationship_object_list):
        self.cardinality = cardinality
        self.polymorphic = polymorphic
        self.relationship_object_list = []
        self.relationship_object_list = relationship_object_list;

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def add_rel_object(self, name, type):
        self.relationship_object_list.append(ECRelObject(name, type))

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def relates_to_object(self, objectName, objectType="Class"):
        matching = [ o for o in self.relationship_object_list if o.get_name().lower() == objectName.lower() ]
        if ( len(matching) > 0):
            return True
        return False

#-------------------------------------------------------------------------------------------
# bsiclass                                      Robert.Priest      04/2017
#-------------------------------------------------------------------------------------------
class ECRelationshipClass(object):
    
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, is_domain_class, strength, strength_direction, source, target):
        self.name = name
        self.is_domain_class = is_domain_class
        self.strength = strength
        self.strength_direction = strength_direction
        self.source = source;
        self.target = target;
        self.ecproperties = []

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_name(self):
        return self.name

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_source(self):
        return self.source
    
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_target(self):
        return self.target

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_properties(self):
        return self.ecproperties

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def set_properties(self, properties):
        self.ecproperties = properties

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def add_property(self, property):
        self.ecproperties.append(property)

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_unique_property_types(self):
        unique_properties = []
        for ecproperty in self.get_properties():
            if ecproperty.type not in unique_properties:
                unique_properties.append(ecproperty.type)
        return unique_properties

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def does_contain_property_type(self, property_type):
        if property_type == "StringLength":
            property_type = 'string'
        return property_type in self.get_unique_property_types()