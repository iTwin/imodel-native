#ifndef XML_PARSER_H_PRIVATE__
#define XML_PARSER_H_PRIVATE__

#include <libxml/parser.h>
#include <libxml/xmlversion.h>

/**
 * XML_VCTXT_DTD_VALIDATED:
 *
 * Set after xmlValidateDtdFinal was called.
 */
#define XML_VCTXT_DTD_VALIDATED (1u << 0)
/**
 * XML_VCTXT_USE_PCTXT:
 *
 * Set if the validation context is part of a parser context.
 */
#define XML_VCTXT_USE_PCTXT (1u << 1)

XML_HIDDEN void
xmlErrMemory(xmlParserCtxtPtr ctxt, const char *extra);
XML_HIDDEN void
__xmlErrEncoding(xmlParserCtxtPtr ctxt, xmlParserErrors xmlerr,
                 const char *msg, const xmlChar *str1,
                 const xmlChar *str2) LIBXML_ATTR_FORMAT(3,0);
XML_HIDDEN void
xmlHaltParser(xmlParserCtxtPtr ctxt);
XML_HIDDEN int
xmlParserGrow(xmlParserCtxtPtr ctxt);
XML_HIDDEN void
xmlParserShrink(xmlParserCtxtPtr ctxt);

XML_HIDDEN void
xmlDetectEncoding(xmlParserCtxtPtr ctxt);
XML_HIDDEN void
xmlSetDeclaredEncoding(xmlParserCtxtPtr ctxt, xmlChar *encoding);
XML_HIDDEN const xmlChar *
xmlGetActualEncoding(xmlParserCtxtPtr ctxt);

XML_HIDDEN xmlParserNsData *
xmlParserNsCreate(void);
XML_HIDDEN void
xmlParserNsFree(xmlParserNsData *nsdb);
/*
 * These functions allow SAX handlers to attach extra data to namespaces
 * efficiently and should be made public.
 */
XML_HIDDEN int
xmlParserNsUpdateSax(xmlParserCtxtPtr ctxt, const xmlChar *prefix,
                     void *saxData);
XML_HIDDEN void *
xmlParserNsLookupSax(xmlParserCtxtPtr ctxt, const xmlChar *prefix);

#define XML_INPUT_BUF_STATIC		(1u << 1)
#define XML_INPUT_BUF_ZERO_TERMINATED	(1u << 2)
#define XML_INPUT_UNZIP			(1u << 3)

/* Internal parser option */
#define XML_PARSE_UNZIP     (1 << 24)

XML_HIDDEN xmlParserInputPtr
xmlNewInputURL(xmlParserCtxtPtr ctxt, const char *url, const char *publicId,
               const char *encoding, int flags);
XML_HIDDEN xmlParserInputPtr
xmlNewInputMemory(xmlParserCtxtPtr ctxt, const char *url,
                  const void *mem, size_t size,
                  const char *encoding, int flags);
XML_HIDDEN xmlParserInputPtr
xmlNewInputString(xmlParserCtxtPtr ctxt, const char *url, const char *str,
                  const char *encoding, int flags);
XML_HIDDEN xmlParserInputPtr
xmlNewInputFd(xmlParserCtxtPtr ctxt, const char *filename, int fd,
              const char *encoding, int flags);
XML_HIDDEN xmlParserInputPtr
xmlNewInputIO(xmlParserCtxtPtr ctxt, const char *url,
              xmlInputReadCallback ioRead,
              xmlInputCloseCallback ioClose,
              void *ioCtxt,
              const char *encoding, int flags);
XML_HIDDEN xmlParserInputPtr
xmlNewInputPush(xmlParserCtxtPtr ctxt, const char *url,
                const char *chunk, int size, const char *encoding);

XML_HIDDEN xmlChar *
xmlExpandEntitiesInAttValue(xmlParserCtxtPtr ctxt, const xmlChar *str,
                            int normalize);

#endif /* XML_PARSER_H_PRIVATE__ */
