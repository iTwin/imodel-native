class ECRelObject (object):
    def __init__(self, name, type):
        self.name = name
        self.type = type
        
class ECRelSource(object):
    def __init__(self, cardinality, polymorphic, relationship_object):
        self.cardinality = cardinality
        self.polymorphic = polymorphic
        self.relationship_object = relationship_object

class ECRelTarget(object):
    def __init__(self, cardinality, polymorphic, relationship_object_list):
        self.cardinality = cardinality
        self.polymorphic = polymorphic
        self.relationship_object_list = []
        self.relationship_object_list = relationship_object_list;

    def add_rel_object(self, name, type):
        self.relationship_object_list.append(ECRelObject(name, type))

class ECRelationshipClass(object):
    def __init__(self, name, is_domain_class, strength, strength_direction, source, target):
        self.name = name
        self.is_domain_class = is_domain_class
        self.strength = strength
        self.strength_direction = strength_direction
        self.source = source;
        self.target = target;
