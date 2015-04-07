#pragma once

#include <set>
#include <string>

// do not change! this is not a game constant, but a technical one.
// memory will not be allocated below this address, thus only values
// below are guaranteed to be indexes. -AlexB
auto const MinimumAresSample = 0x10000;

class LooseAudioCache {
public:
	int GetIndex(const char* pFilename) {
		auto it = this->Files.find(pFilename);
		if(it == this->Files.end()) {
			it = this->Files.emplace(pFilename).first;
		}

		return reinterpret_cast<int>(it->c_str());
	}

private:
	std::set<std::string> Files;
};
