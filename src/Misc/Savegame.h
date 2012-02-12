#ifndef ARES_SAVEGAME_H
#define ARES_SAVEGAME_H

class AresSaveStream : public std::vector<byte> {
public:
	AresSaveStream(size_t Reserve = 0x1000) : std::vector<byte>() {
		this->reserve(Reserve);
	};
};

#endif
