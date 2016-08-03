from xml.dom import minidom
from openpyxl import load_workbook

from SchemaWriter.Helpers.Api import Api
from SchemaWriter.Helpers.ECSchema import ECSchema
from SchemaWriter.HeaderWriters.SchemaApiPublicHeaderWriter import CallStatus
from SchemaWriter.HeaderWriters.SchemaApiPublicHeaderWriter import SchemaApiPublicHeaderWriter
from SchemaWriter.SourceWriters.SchemaApiSourceWriter import SchemaApiSourceWriter
from SchemaWriter.HeaderWriters.SchemaBufferHeaderWriter import SchemaBufferHeaderWriter
from SchemaWriter.HeaderWriters.SchemaBufferPublicHeaderWriter import SchemaBufferPublicHeaderWriter
from SchemaWriter.SourceWriters.SchemaBufferSourceWriter import SchemaBufferSourceWriter
from BufferHeaderWriter import BufferHeaderWriter
from BufferSourceWriter import BufferSourceWriter

###########################################################################################################
# ------------------------------------------------------------------------------------------------------- #
# -------------------------------------  pyCApiGen Configuration  --------------------------------------- #
# ------------------------------------------------------------------------------------------------------- #
###########################################################################################################

#############################################
# ------------- Status Codes -------------- #
#############################################
status_codes = {
    'ERROR400': CallStatus(400, 'The response from the server contained a 400, Bad Request, http error'),
    'ERROR401': CallStatus(401, 'The response from the server contained a 401, Unauthorized, http error'),
    'ERROR403': CallStatus(403, 'The response from the server contained a 403, Forbidden, http error'),
    'ERROR404': CallStatus(404, 'The response from the server contained a 404, Not Found, http error'),
    'ERROR409': CallStatus(409, 'The response from the server contained a 409, Conflict, http error'),
    'ERROR500': CallStatus(500, 'The response from the server contained a 500, Internal Server, http error'),
    'SUCCESS': CallStatus(0, 'Successful operation'),
    'INVALID_PARAMETER': CallStatus(-100, 'Invalid parameter passed to function'),
    'PROPERTY_HAS_NOT_BEEN_SET': CallStatus(-101, 'The buffer property passed to function has not been set in the buffer'),
    'INTERNAL_MEMORY_ERROR': CallStatus(-102, 'Memory failed to initialize interally.'),
    'LOGIN_FAILED': CallStatus(-200, 'The login attempt was not completed successfully.'),
    'SSL_REQUIRED': CallStatus(-201, 'SSL is required for communication with the server.'),
    'NOT_ENOUGH_RIGHTS': CallStatus(-202, 'The user does not have the proper rights.'),
    'REPOSITORY_NOT_FOUND': CallStatus(-204, 'The repository you attempted to query was not found.'),
    'SCHEMA_NOT_FOUND': CallStatus(-205, 'The schema you attempted to query was not found.'),
    'CLASS_NOT_FOUND': CallStatus(-206, 'The class you attempted to query was not found.'),
    'PROPERTY_NOT_FOUND': CallStatus(-207, 'The property you attempted to query was not found.'),
    'INSTANCE_NOT_FOUND': CallStatus(-210, 'The instance you attempted to query was not found.'),
    'FILE_NOT_FOUND': CallStatus(-211, 'The file you attempted to query was not found.'),
    'NOT_SUPPORTED': CallStatus(-212, 'The action you attempted is not supported.'),
    'NO_SERVER_LICENSE': CallStatus(-213, 'A server license was not found.'),
    'NO_CLIENT_LICENSE': CallStatus(-214, 'A client license was not found.'),
    'TO_MANY_BAD_LOGIN_ATTEMPTS': CallStatus(-215, 'To many unsuccessful login attempts have happened.'),
}

############################################
# -------- Schemafile Parsing ------------ #
############################################
api = Api('CWSCC', 'ConnectWebServicesClientC')
workbook = load_workbook('autoGenClasses.xlsx', use_iterators=True)
worksheet = workbook.get_sheet_by_name('autoGenClasses')
unique_ecschemas = {}
for row in worksheet.iter_rows(range_string=worksheet.calculate_dimension(), row_offset=2):
    if row[0].value is None or row[1].value is None or row[2].value is None:
        break
    if row[0].value + row[1].value + row[2].value + row[3].value not in unique_ecschemas:
        unique_ecschemas[row[0].value + row[1].value + row[2].value + row[3].value] = \
            ECSchema(row[0].value, row[1].value, row[2].value, row[3].value, api, status_codes)
    unique_ecschemas[row[0].value + row[1].value + row[2].value + row[3].value].add_ecclass(row)

for ecschema in unique_ecschemas.values():
    xmldoc = minidom.parse(ecschema.get_filename())
    ecschemasXml = xmldoc.getElementsByTagName('ECSchema')
    for ecschemaXml in ecschemasXml:
        if ecschemaXml.attributes['schemaName'].value == ecschema.get_name():
            if ecschemaXml.attributes['xmlns'].value != 'http://www.bentley.com/schemas/Bentley.ECXML.2.0':
                raise IOError("{0} file's xmlns is not support. This tool only supports the "
                              "'http://www.bentley.com/schemas/Bentley.ECXML.2.0' format"
                              .format(ecschema.get_name()))
            ecschema.init_xml(ecschemaXml)
            break

#############################################
# ------------ Autogeneration ------------- #
#############################################
srcDir = "../../"
publicApiDir = srcDir + "../PublicApi/WebServices/ConnectC/"
for ecschema in unique_ecschemas.values():
    ####################################################################
    # -----------------------------------------------------------------#
    # -----------------------SCHEMA-LEVEL------------------------------#
    # ----------------------------API----------------------------------#
    ####################################################################
    schemaApiPublicHeaderWriter = SchemaApiPublicHeaderWriter(ecschema,
                                                              publicApiDir + '{0}/{1}GenPublic.h'
                                                              .format(ecschema.get_url_descriptor(), ecschema.get_name()),
                                                              api,
                                                              status_codes)
    schemaApiPublicHeaderWriter.write_header()

    schemaApiSourceWriter = SchemaApiSourceWriter(ecschema,
                                                  srcDir + '{0}/{1}Gen.cpp'
                                                  .format(ecschema.get_url_descriptor(), ecschema.get_name()),
                                                  api,
                                                  status_codes)
    schemaApiSourceWriter.write_source()

    ####################################################################
    # -----------------------------------------------------------------#
    # -----------------------SCHEMA-LEVEL------------------------------#
    # ---------------------------BUFFER--------------------------------#
    ####################################################################
    schemaBufferHeaderWriter = SchemaBufferHeaderWriter(ecschema,
                                                        srcDir + '{0}/{1}BufferGen.h'
                                                        .format(ecschema.get_url_descriptor(), ecschema.get_name()),
                                                        api,
                                                        status_codes)
    schemaBufferHeaderWriter.write_header()

    schemaBufferPublicHeaderWriter = SchemaBufferPublicHeaderWriter(ecschema,
                                                                    publicApiDir + '{0}/{1}BufferGenPublic.h'
                                                                    .format(ecschema.get_url_descriptor(), ecschema.get_name()),
                                                                    api,
                                                                    status_codes)
    schemaBufferPublicHeaderWriter.write_header()

    schemaBufferSourceWriter = SchemaBufferSourceWriter(ecschema,
                                                        srcDir + '{0}/{1}BufferGen.cpp'
                                                        .format(ecschema.get_url_descriptor(), ecschema.get_name()),
                                                        api,
                                                        status_codes)
    schemaBufferSourceWriter.write_source()

####################################################################
# -----------------------------------------------------------------#
# --------------------------API-LEVEL------------------------------#
# -----------------------------------------------------------------#
####################################################################
bufferHeaderWriter = BufferHeaderWriter(unique_ecschemas.values(),
                                        srcDir + '{0}BufferGen.h'.format(api.get_upper_api_acronym()),
                                        api,
                                        status_codes)
bufferHeaderWriter.write_header()

bufferSourceWriter = BufferSourceWriter(unique_ecschemas.values(),
                                        srcDir + '{0}BufferGen.cpp'.format(api.get_upper_api_acronym()),
                                        api,
                                        status_codes)
bufferSourceWriter.write_source()
