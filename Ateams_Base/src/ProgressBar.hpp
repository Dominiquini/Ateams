#include <iostream>
#include <string>

#include "Constants.hpp"

#ifndef _PROGRESS_BAR_
#define _PROGRESS_BAR_

#define DEFAULT_PROGRESS_SIZE 100
#define DEFAULT_PROGRESS_COLOR COLOR_RED

class ProgressBar {
public:

	ProgressBar(std::string label, std::string color, int progressBarLength) {
		this->label = label;
		this->color = color;
		this->progressBarLength = progressBarLength - label.size();

		this->maxProgress = -1;
		this->currentProgress = -1;

		this->amountOfFiller = 0;
		this->currUpdateVal = 0;
	}
	ProgressBar(std::string label) : ProgressBar(label, DEFAULT_PROGRESS_COLOR, DEFAULT_PROGRESS_SIZE) {
	}
	ProgressBar() : ProgressBar("", DEFAULT_PROGRESS_COLOR, DEFAULT_PROGRESS_SIZE) {
	}
	~ProgressBar() {
	}

	void init(int maxProgress) {
		this->maxProgress = std::max(0, maxProgress);

		std::cout << std::endl << color;

		update(0);
	}

	void end() {
		this->maxProgress = -1;

		update(currentProgress);

		std::cout << std::endl << COLOR_DEFAULT;
	}

	void update(int newProgress) {
		if (maxProgress != -1) {
			int oldProgress = currentProgress;
			currentProgress = std::min(maxProgress, newProgress);

			if (oldProgress != currentProgress) {
				amountOfFiller = (int) (progress() * (double) progressBarLength);

				print();
			}
		}
	}

	void print() {
		currUpdateVal %= pBarUpdater.length();

		std::cout << "\r" << label << firstPartOfpBar;
		for (int a = 0; a < amountOfFiller; a++) {
			std::cout << pBarFiller;
		}
		std::cout << pBarUpdater[currUpdateVal];
		for (int b = 0; b < progressBarLength - amountOfFiller; b++) {
			std::cout << " ";
		}
		std::cout << lastPartOfpBar << " (" << percentage() << "%)";

		currUpdateVal += 1;

		std::cout << std::flush;
	}

	float progress() {
		return maxProgress != 0 ? (float) currentProgress / (float) maxProgress : 1.0f;
	}

	int percentage() {
		return progress() * 100;
	}

private:

	const std::string firstPartOfpBar = "[", lastPartOfpBar = "]", pBarFiller = "|", pBarUpdater = "|";

	std::string label;
	std::string color;
	int maxProgress;
	int progressBarLength;

	int currentProgress;
	int amountOfFiller;
	int currUpdateVal;
};

#endif
