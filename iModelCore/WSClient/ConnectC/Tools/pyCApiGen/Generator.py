from xml.dom import minidom
from openpyxl import load_workbook

from HeaderWriters.CBufferHeaderWriter import CBufferHeaderWriter
from HeaderWriters.CPublicApiHeaderWriter import CallStatus
from HeaderWriters.CPublicApiHeaderWriter import CPublicApiHeaderWriter
from HeaderWriters.CPublicBufferHeaderWriter import CPublicBufferHeaderWriter
from SourceWriters.CApiSourceWriter import CApiSourceWriter
from SourceWriters.CBufferSourceWriter import CBufferSourceWriter
from Writer import Api
from ExcludedECClass import ExcludedECClass

###########################################################################################################
# ------------------------------------------------------------------------------------------------------- #
# -----------------------------     Configuration Variables    ------------------------------------------ #
# ------------------------------------------------------------------------------------------------------- #
###########################################################################################################
xmldoc = minidom.parse('N~3AGlobalSchema.01.00.xml')
ecclasses = xmldoc.getElementsByTagName('ECClass')
api = Api('CWSCC', 'ConnectWebServicesClientC', 'ConnectWsgGlobal', 'BentleyCONNECT.Global--CONNECT.GLOBAL')

#############################################
# ------------ Excluded Classes ----------- #
#############################################
workbook = load_workbook('autoGenFilter.xlsx', use_iterators=True)
worksheet = workbook.get_sheet_by_name('autoGenFilter')
excluded_classes = {}
for row in worksheet.iter_rows(range_string=worksheet.calculate_dimension(), row_offset=1):
    excluded_classes[row[0].value] = (ExcludedECClass(row))

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
srcDir = "../../"
publicApiDir = srcDir + "../PublicAPI/WebServices/ConnectC/"

####################################################################
# -----------------------------------------------------------------#
# ---------------------------C-API---------------------------------#
# -----------------------------------------------------------------#
####################################################################
publicApiHeaderWriter = CPublicApiHeaderWriter('GlobalSchema',
                                               ecclasses,
                                               publicApiDir + '{0}GenPublic.h'.format(api.get_api_acronym()),
                                               api,
                                               status_codes,
                                               excluded_classes)
publicApiHeaderWriter.write_header()
apiSourceWriter = CApiSourceWriter('GlobalSchema',
                                   ecclasses,
                                   srcDir + '{0}Gen.cpp'.format(api.get_api_acronym()),
                                   api,
                                   status_codes,
                                   excluded_classes)
apiSourceWriter.write_source()

####################################################################
# -----------------------------------------------------------------#
# --------------------------C-Buffer-------------------------------#
# -----------------------------------------------------------------#
####################################################################
publicBufferHeaderWriter = CPublicBufferHeaderWriter(ecclasses,
                                                     publicApiDir + '{0}GenBufferPublic.h'.format(api.get_api_acronym()),
                                                     api,
                                                     status_codes,
                                                     excluded_classes)
publicBufferHeaderWriter.write_header()
headerWriter = CBufferHeaderWriter(ecclasses,
                                   srcDir + '{0}GenBuffer.h'.format(api.get_api_acronym()),
                                   api,
                                   status_codes,
                                   excluded_classes)
headerWriter.write_header()
sourceWriter = CBufferSourceWriter(ecclasses,
                                   srcDir + '{0}GenBuffer.cpp'.format(api.get_api_acronym()),
                                   api,
                                   status_codes,
                                   excluded_classes)
sourceWriter.write_source()