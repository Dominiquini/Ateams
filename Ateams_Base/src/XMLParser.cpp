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

	if (strcasecmp(element, "Controller") == 0) {
		char *parameter = NULL;
		char *value = NULL;

		for (unsigned int ix = 0; ix < attrs.getLength(); ++ix) {
			parameter = XMLString::transcode(attrs.getQName(ix));
			value = XMLString::transcode(attrs.getValue(ix));

			if (!ctrl->setParameter(parameter, value)) {
				throw string("Invalid Parameter: ").append(parameter);
			}

			XMLString::release(&parameter);
			XMLString::release(&value);
		}
	} else if (strcasecmp(element, "Heuristics") == 0) {
	} else {
		Heuristic *newHeuristic = NULL;

		if (strcasecmp(element, "SimulatedAnnealing") == 0)
			newHeuristic = new SimulatedAnnealing();

		if (strcasecmp(element, "TabuSearch") == 0)
			newHeuristic = new TabuSearch();

		if (strcasecmp(element, "GeneticAlgorithm") == 0)
			newHeuristic = new GeneticAlgorithm();

		if (newHeuristic != NULL) {
			char *parameter = NULL;
			char *value = NULL;

			for (unsigned int ix = 0; ix < attrs.getLength(); ++ix) {
				parameter = XMLString::transcode(attrs.getQName(ix));
				value = XMLString::transcode(attrs.getValue(ix));

				if (!newHeuristic->setParameter(parameter, value)) {
					throw string("Invalid Parameter: ").append(parameter);
				}

				XMLString::release(&parameter);
				XMLString::release(&value);
			}

			ctrl->addHeuristic(newHeuristic);
		}
	}

	XMLString::release(&element);
}
