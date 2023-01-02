# This file outlines the reason for storing the ecdb file and the bug associated with it.

# --------------------------------------------------------------------------------------------------------------------------------------------------------------
#			The Bug / Issue
# --------------------------------------------------------------------------------------------------------------------------------------------------------------
- As part of a fix done for concurrent querying, we added code to the Schema Reader that updates ExtendedTypeName values in memory on the ec_Property table.
- When this in-memory change is done, the ExtendedTypeName values of a few properties are modified to new values. All these properties have their ExtendedTypeName values set to 'Id' in the Schema XML.
- In the 'ECDbSystem' schema and 'ClassECSqlSystemProperties' class, we have a property called 'ECInstanceId' whose ExtendedTypeName value is already 'Id' in the SchemaXML.
- When a profile upgrade being done and schemas are being imported, the code assumes ECInstanceId property wasn't changed at all (since it was set to 'Id' in memory, but is still NULL in DB).
- The unintended consequence of all the above is, when we upgrade from a schema that has a NULL ExtendedTypeName for the ECInstanceId property, the code assumes no change was done (since in-memory it was set to 'Id'), and hence it doesn't trigger an sql update which would have updated the value to 'Id' in the DB.
- Hence, in DB the value remains NULL and the DB and memory are now inconsistent.

# ------------------------------------------------------------------------------------------------------------------------------------
#			PURPOSE
# ------------------------------------------------------------------------------------------------------------------------------------
- The file 'upgradedimodel2.ecdb' is where we can see the above issue.
- Main purpose of archiving this file is for testing in the future to make sure this bug has been fixed completely.