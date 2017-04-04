#!Python
#--------------------------------------------------------------------------------------
#
#     $Source: ConnectC/Tools/pyCApiGen/SchemaWriter/Helpers/ECRelationshipClass.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------

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