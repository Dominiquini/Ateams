#include "XMLParser.hpp"

using namespace xercesc;

XMLParser::XMLParser(Control *ctrl) {
	this->ctrl = ctrl;

	XMLPlatformUtils::Initialize();

	parser = XMLReaderFactory::createXMLReader();
	parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
	parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);

	parser->setContentHandler(this);
	parser->setErrorHandler(this);
}

XMLParser::~XMLParser() {
	delete parser;

	XMLPlatformUtils::Terminate();
}

void XMLParser::parseXML(char *parameters) {
	parser->parse(parameters);
}

/* Parser do arquivo XML de configuracoes */
void XMLParser::startElement(const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname, const Attributes &attrs) {
	char *element = XMLString::transcode(localname);

	if (strcmpi(element, "Controller") == 0) {
		char *parameter = NULL;
		char *value = NULL;

		for (unsigned int ix = 0; ix < attrs.getLength(); ++ix) {
			parameter = XMLString::transcode(attrs.getQName(ix));
			value = XMLString::transcode(attrs.getValue(ix));

			if (!ctrl->setParameter(parameter, value)) {
				throw string("Invalid Ateams Parameter: ").append(parameter);
			}

			XMLString::release(&parameter);
			XMLString::release(&value);
		}
	} else if (strcmpi(element, "Heuristics") == 0) {
		ctrl->newHeuristic(NULL);
	} else {
		Heuristic *newHeuristic = Control::instantiateHeuristic(element);

		if (newHeuristic != NULL) {
			char *parameter = NULL;
			char *value = NULL;

			for (unsigned int ix = 0; ix < attrs.getLength(); ++ix) {
				parameter = XMLString::transcode(attrs.getQName(ix));
				value = XMLString::transcode(attrs.getValue(ix));

				if (!newHeuristic->setParameter(parameter, value)) {
					throw string("Invalid Heuristic Parameter: ").append(parameter).append(" (").append(newHeuristic->getParameters().name).append(") ");
				}

				XMLString::release(&parameter);
				XMLString::release(&value);
			}

			ctrl->newHeuristic(newHeuristic);
		}
	}

	XMLString::release(&element);
}
