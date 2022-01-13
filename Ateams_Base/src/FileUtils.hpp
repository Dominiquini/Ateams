#include <algorithm>
#include <string>
#include <cctype>

#ifndef _FILE_UTILS_
#define _FILE_UTILS_

class FileUtils {
public:

	/**
	 * FUNCTION: getFilePath
	 * USE: Returns the path from a given file path
	 * @param path: The path of the file
	 * @return: The path from the given file path
	 */
	static std::string getFilePath(const std::string &path) {
		auto pathEnd = path.find_last_of("/\\");
		auto pathName = pathEnd == std::string::npos ? path : path.substr(0, pathEnd);
		return pathName;
	}

	/**
	 * FUNCTION: getFileName
	 * USE: Returns the file name from a given file path
	 * @param path: The path of the file
	 * @return: The file name from the given file path
	 */
	static std::string getFileName(const std::string &path) {
		auto fileNameStart = path.find_last_of("/\\");
		auto fileName = fileNameStart == std::string::npos ? path : path.substr(fileNameStart + 1);
		return fileName;
	}

	/**
	 * FUNCTION: getFileExtension
	 * USE: Returns the file extension from a given file path
	 * @param path: The path of the file
	 * @return: The file extension from the given file path
	 */
	static std::string getFileExtension(const std::string &path) {
		auto fileName = getFileName(path);
		auto extStart = fileName.find_last_of('.');
		auto ext = extStart == std::string::npos ? "" : fileName.substr(extStart + 1);
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
			return static_cast<unsigned char>(std::tolower(c));
		});
		return ext;
	}
};

#endif
