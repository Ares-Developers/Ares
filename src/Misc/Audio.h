#pragma once

#include <Audio.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

// do not change! this is not a game constant, but a technical one.
// memory will not be allocated below this address, thus only values
// below are guaranteed to be indexes. -AlexB
auto const MinimumAresSample = 0x10000;

class LooseAudioCache {
public:
	static LooseAudioCache Instance;

	struct LooseAudioFile {
		LooseAudioFile()
			: Offset(-1), Size(-1), Data()
		{ }

		int Offset;
		int Size;
		AudioSampleData Data;
	};

	LooseAudioFile& GetData(const char* pFilename) {
		return this->Files[pFilename];
	}

	int GetIndex(const char* pFilename) {
		auto it = this->Files.find(pFilename);
		if(it == this->Files.end()) {
			it = this->Files.emplace(pFilename, LooseAudioFile()).first;
		}

		return reinterpret_cast<int>(it->first.c_str());
	}

private:
	std::map<std::string, LooseAudioFile, std::less<>> Files;
};

class AresAudioHelper {
public:
	struct FileStruct {
		int Size;
		int Offset;
		RawFileClass* File;
		bool Allocated;
	};

	static AudioSampleData* GetData(int index);

	static FileStruct GetFile(int index);

	static const char* ToLooseFile(int index) {
		if(index >= MinimumAresSample) {
			return reinterpret_cast<const char*>(index);
		}
		return nullptr;
	}

	static int ToSampleIndex(int index) {
		if(index < MinimumAresSample) {
			return index;
		}
		return -1;
	}
};

class AudioBag {
public:
	AudioBag() = default;

	explicit AudioBag(const char* pFilename) : AudioBag() {
		this->Open(pFilename);
	}

	AudioBag(AudioBag&& other) {
		this->Entries = std::move(other.Entries);
		this->Bag = std::move(other.Bag);
	};

	CCFileClass* file() const {
		return this->Bag.get();
	}

	const std::vector<AudioIDXEntry>& entries() const {
		return this->Entries;
	}

private:
	void Open(const char* fileBase);

	template <typename T>
	using UniqueGamePtr = std::unique_ptr<T, GameDeleter>;

	UniqueGamePtr<CCFileClass> Bag;
	std::vector<AudioIDXEntry> Entries;
};

// a collection of audio bags, get it?
class AudioLuggage {
public:
	static AudioLuggage Instance;

	void Append(const char* pFileBase) {
		this->Bags.emplace_back(pFileBase);
	}

	AudioIDXData* Create(const char* pPath = nullptr);

	CCFileClass* GetFile(int index) const {
		return this->Files[static_cast<unsigned int>(index)];
	}

private:
	std::vector<AudioBag> Bags;
	std::vector<CCFileClass*> Files;
};
