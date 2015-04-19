#pragma once

#include "../Ares.h"

#include <Unsorted.h>

#include <string>
#include <vector>

class MoviesList
{
	struct Item : public MovieUnlockableInfo
	{
		std::string FilenameBuffer;
		std::string DescriptionBuffer;
		bool Unlockable = true;
		bool Unlocked = false;
	};

public:
	static MoviesList Instance;

	bool PopulateMovieList(HWND const hWnd) const;
	void Unlock(char const* pFilename);
	void LoadListFromINI();
	void WriteToINI() const;

private:
	void AddMovie(HWND const hWnd, MovieUnlockableInfo const& movie) const;
	Item* FindMovie(const char* pFilename);

	std::vector<Item> Array;
};
