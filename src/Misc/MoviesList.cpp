#include "MoviesList.h"

#include <CCFileClass.h>
#include <CCINIClass.h>
#include <Memory.h>
#include <StringTable.h>

#include <algorithm>
#include <memory>

MoviesList MoviesList::Instance;

template <typename T>
using UniqueGamePtr = std::unique_ptr<T, GameDeleter>;

bool MoviesList::PopulateMovieList(HWND const hWnd) const
{
	for(auto const& item : this->Array) {
		// add commons and unlocked, in order
		if(!item.Unlockable || item.Unlocked) {
			this->AddMovie(hWnd, item);
		}
	}

	return !this->Array.empty();
}

void MoviesList::Unlock(char const* const pFilename)
{
	if(auto const pItem = this->FindMovie(pFilename)) {
		pItem->Unlocked = true;
	}
}

void MoviesList::LoadListFromINI()
{
	Debug::Log("Reading MOVIEMD.INI\n");

	auto const pINI = UniqueGamePtr<CCINIClass>(GameCreate<CCINIClass>());
	auto const pFile = UniqueGamePtr<CCFileClass>(GameCreate<CCFileClass>("MOVIEMD.INI"));

	if(pINI && pFile && pFile->Exists()) {
		pINI->ReadCCFile(pFile.get());

		// create a list of all movies first
		auto const count = pINI->GetKeyCount("Movies");
		this->Array.reserve(static_cast<size_t>(count));

		for(int i = 0; i < count; ++i) {
			char buffer[0x20];
			auto const pKey = pINI->GetKeyName("Movies", i);
			if(int len = pINI->ReadString("Movies", pKey, "", buffer)) {
				if(!this->FindMovie(buffer)) {
					this->Array.emplace_back();
					auto& item = this->Array.back();
					item.FilenameBuffer.assign(buffer, buffer + len);
				}
			}
		}

		// then read all items
		for(auto& item : this->Array) {
			char buffer[0x20];
			auto const pID = item.FilenameBuffer.c_str();
			if(int len = pINI->ReadString(pID, "Description", "", buffer)) {
				item.DescriptionBuffer.assign(buffer, buffer + len);
			}

			item.DiskRequired = pINI->ReadInteger(pID, "DiskRequired", item.DiskRequired);
			item.Unlockable = pINI->ReadBool(pID, "Unlockable", item.Unlockable);

			// update the pointers (required because of small string optimization
			// and vector reallocation)
			item.Filename = item.FilenameBuffer.c_str();
			item.Description = item.DescriptionBuffer.c_str();
		}
	}

	// load unlocked state
	if(auto const pRA2MD = CCINIClass::INI_RA2MD) {
		for(auto& item : this->Array) {
			auto& value = item.Unlocked;
			value = pRA2MD->ReadBool("UnlockedMovies", item.Filename, value);
		}
	}
}

void MoviesList::WriteToINI() const
{
	if(auto const pINI = CCINIClass::INI_RA2MD) {
		for(auto& item : this->Array) {
			// only write if unlocked, to not reveal movie names
			if(auto const value = item.Unlockable && item.Unlocked) {
				pINI->WriteBool("UnlockedMovies", item.Filename, value);
			}
		}
	}
}

void MoviesList::AddMovie(HWND const hWnd, MovieUnlockableInfo const& movie) const
{
	if(movie.Filename && movie.Description) {
		auto const pName = StringTable::LoadString(movie.Description);
		auto const lparam = reinterpret_cast<LPARAM>(pName);
		auto const res = SendMessage(hWnd, WW_LB_ADDITEM, 0, lparam);
		if(res != -1) {
			auto const index = static_cast<WPARAM>(res);
			auto const data = reinterpret_cast<LPARAM>(&movie);
			SendMessage(hWnd, LB_SETITEMDATA, index, data);
		}
	}
}

MoviesList::Item* MoviesList::FindMovie(const char* const pFilename)
{
	auto const it = std::find_if(
		this->Array.begin(), this->Array.end(),
		[=](Item const& item) { return item.FilenameBuffer == pFilename; });

	return (it == this->Array.end()) ? nullptr : &*it;
}

DEFINE_HOOK(52C939, InitGame_MoviesList, 5)
{
	MoviesList::Instance.LoadListFromINI();
	return 0;
}

DEFINE_HOOK(5FBF80, GameOptionsClass_UnlockMovieIfNeeded_MoviesList, 5)
{
	GET_STACK(char const* const, pMovieName, STACK_OFFS(0x0, -0x4));
	MoviesList::Instance.Unlock(pMovieName);
	return 0;
}

DEFINE_HOOK(5FAFFB, Options_SaveToINI_MoviesList, 6)
{
	MoviesList::Instance.WriteToINI();
	return 0;
}

DEFINE_HOOK(5FC000, GameOptionsClass_PopulateMovieList, 6)
{
	//GET(GameOptionsClass* const, pThis, ECX);
	GET_STACK(HWND const, hWnd, STACK_OFFS(0x0, -0x4));
	return MoviesList::Instance.PopulateMovieList(hWnd) ? 0x5FC11Cu : 0u;
}
