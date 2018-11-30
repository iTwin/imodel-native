class ECProperty(object):
    def __init__(self, name, property_type, is_read_only, should_be_excluded):
        self.name = name
        self.type = property_type
        self.is_read_only = is_read_only
        self.should_be_excluded = should_be_excluded
