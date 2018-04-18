#!Python
#--------------------------------------------------------------------------------------
#
#     $Source: mki/BisSchemasValidation/SchemaCheckup.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import re
from lxml import etree

FILE_PREFIX = "file:///"
XMLSchemaNamespace = "http://www.w3.org/2001/XMLSchema"
ANNOTATION_XPATH = "//xsd:pattern[@value=\"%s\"]/../xsd:annotation/xsd:documentation"

def remove_prefix(s, p):
    if s.startswith(p):
        return s[len(p):]
    return s

def write_log(logFile, status, message):
    logFile.write("+++ LOG> %-5s SchemaCheckup   - %s\n\n" % (status, message)) 

'''
If the error message is about a failed regex pattern match, attempts to replace the message to make it easier to read.
The message is replaced using the text of the annotation element, if one exists.
@arg message: str - error message
@arg xmlschema_doc: lxml.etree._XSLTResultTree
@return: str - modified message, if the error is a failed regex pattern match and an annotation element exists for the pattern.
             - original message, otherwise.
'''
def attemptReplaceRegexErrorMessage(message, xmlschema_doc):
    regex = re.compile("\[facet 'pattern'\] The value '(?P<name>.+?)' is not accepted by the pattern '(.+?)'\.")
    match = regex.search(message)
    if not match:
        return message

    pattern = match.groups()[1] # enumeration's regex pattern
    annotation = xmlschema_doc.getroot().xpath(ANNOTATION_XPATH % pattern, namespaces = {"xsd": XMLSchemaNamespace})
    if len(annotation) == 0:
        return message

    # replace the error message using the the annotation
    return regex.sub("[facet 'enumeration'] The value '\g<1>' does not belong to the set {%s}'." % annotation[0].text, message)

'''
Logs the error message.
Attempts to replace a regex error message with a more readable error
@arg error: lxml.etree._LogEntry
@arg xmlschema_doc: lxml.etree._XSLTResultTree
'''
def log_error(error, xmlschema_doc, logFile):
    message = attemptReplaceRegexErrorMessage(error.message, xmlschema_doc)
    write_log(logFile, "ERROR", "%s - %s(%s)" % (message, remove_prefix(error.filename, FILE_PREFIX), error.line))

'''
Validate an XML document against an XSD
@arg xml_path: str - path to xml document
@arg xsd_path: str - path to xsd schema
@arg xsl_path: str - path to xsl style sheet (optional)
@return bool - true if succeeds, false otherwise
'''
def validate(xml_path, xsd_path, xsl_path, logFile):
    write_log(logFile, "INFO", "Validating %s against %s" % (xml_path, xsd_path))

    # parse XSD schema
    xmlschema_doc = etree.parse(xsd_path)

    # transform XSD using XSL style sheet
    xsl_doc = etree.parse(xsl_path)
    transform = etree.XSLT(xsl_doc)
    xmlschema_doc = transform(xmlschema_doc)
    
    xmlschema = etree.XMLSchema(xmlschema_doc)
    
    # parse XML document
    xml_doc = etree.parse(xml_path)

    # validate XML document against XSD
    result = xmlschema.validate(xml_doc)
    
    # log errors
    errorLog = xmlschema.error_log
    for error in errorLog:
        log_error(error, xmlschema_doc, logFile)

    if result:
        write_log(logFile, "INFO", "Schema Checkup succeeded.")
    else:
        write_log(logFile, "INFO", "Schema Checkup failed. See above errors.")

    return result
