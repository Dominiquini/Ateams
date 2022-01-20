#include "XMLParser.hpp"

using namespace pugi;

XMLParser::XMLParser(Control *ctrl) {
	this->ctrl = ctrl;
}

XMLParser::~XMLParser() {
}

void XMLParser::parseXML(char *parameters) {
	try {
		xml_document parametersFile;

		if (!parametersFile.load_file(parameters)) {
			throw string("Unable To Parse: ") + string(parameters);
		}

		xml_node ctrlNode = parametersFile.child(TAG_CONTROLLER);

		for (xml_attribute ctrlAttr : ctrlNode.attributes()) {
			auto parameter = ctrlAttr.name();
			auto value = ctrlAttr.value();

			if (!ctrl->setParameter(parameter, value)) {
				throw string("Invalid Control Parameter: ").append(parameter);
			}
		}

		ctrl->clearHeuristics(true);

		xml_node heuristicsNode = ctrlNode.child(TAG_HEURISTICS);

		if (heuristicsNode.empty()) {
			throw string("No Heuristics Defined!");
		}

		for (xml_node heuristicNode : heuristicsNode.children()) {
			auto heuristicName = heuristicNode.name();

			Heuristic *newHeuristic = Control::instantiateHeuristic(heuristicName);

			if (newHeuristic == NULL) {
				throw string("Invalid Heuristic Name: ").append(heuristicName);
			}

			for (xml_attribute heuristicAttr : heuristicNode.attributes()) {
				auto parameter = heuristicAttr.name();
				auto value = heuristicAttr.value();

				if (!newHeuristic->setParameter(parameter, value)) {
					throw string("Invalid Heuristic Parameter: ").append(parameter).append(" (").append(newHeuristic->getParameters().name).append(") ");
				}
			}

			ctrl->insertHeuristic(newHeuristic, true);
		}
	} catch (...) {
		throw string("Unknown Error: ") + string(parameters);
	}
}
