#include "Control.hpp"

#include "PugiXML.hpp"

using namespace pugi;

#ifndef _XML_PARSER_
#define _XML_PARSER_

#define TAG_CONTROLLER "Controller"
#define TAG_HEURISTICS "Heuristics"

class Control;

class XMLParser {
private:

	Control *ctrl;

public:

	XMLParser(Control *ctrl);
	~XMLParser();

	/* Parser do arquivo XML de configuracoes */
	void parseXML(char* parameters);
};

#endif
