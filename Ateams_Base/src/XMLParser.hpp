#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/Attributes.hpp>

#include <xercesc/util/Xerces_autoconf_config.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>

#include "Control.hpp"

using namespace xercesc;

#ifndef _XML_PARSER_
#define _XML_PARSER_

#define TAG_CONTROLLER "Controller"
#define TAG_HEURISTICS "Heuristics"

class Control;

class XMLParser: public DefaultHandler {
private:

	Control *ctrl;

	SAX2XMLReader *parser;

public:

	XMLParser(Control *ctrl);
	~XMLParser();

	void parseXML(char* parameters);

private:

	/* Parser do arquivo XML de configuracoes */
	void startElement(const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes &attrs);
};

#endif
