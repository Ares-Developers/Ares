#pragma once

#include "../Ares.h"

enum class YRDialogID : int {
	None = 0,
	CampaignMenu = 148,
	GameOptionsMenu = 181,
	OneButtonMessageBox = 206,
	MainMenu = 226,
	SinglePlayerMenu = 256,
	MoviesAndCreditsMenu = 257
};

namespace DialogConstants {
	const int SidebarLabel = 1684;
	const int StatusLabel = 1685;
	const int LargeVideo = 1818;
	const int SidebarVideo = 1820;

	const int WorkAreaWidth = 632;
	const int WorkAreaHeight = 568;

	namespace MainDialog {
		const int SidebarVersion = 1821;
		const int ExitGameButton = 1006;
		const int SinglePlayerButton = 1667;
		const int WestwoodOnlineButton = 1668;
		const int OptionsButton = 1372;
		const int NetworkButton = 1400;
		const int MoviesAndCreditsButton = 1670;
	}

	namespace OneButtonMessageBox {
		const int MessageText = 1456;
		const int OkButton = 1454;
	}

	namespace SinglePlayerDialog {
		const int NewCampaignButton = 1672;
		const int LoadSavedGameButton = 1673;
		const int SkirmishButton = 1401;
		const int BackButton = 1670;
	}

	namespace CampaignDialog {
		const int CampaignList = 1109;
		const int LoadButton = 1038;
		const int BackButton = 1670;
		const int DifficultySlider = 1295;
		const int DifficultyLabel = 1822;
		const int DifficultyText = 1648;
		const int AlliedImage = 1770;
		const int SovietImage = 1772;
		const int ThirdImage = 1771;
		const int FourthImage = 1773;
		const int AlliedLabel = 1959;
		const int SovietLabel = 1960;
		const int ThirdLabel = 1961;
		const int FourthLabel = 1962;
	}

	namespace MoviesAndCreditsDialog {
		const int SneakPeeksButton = 1677;
		const int PlayMoviesButton = 1678;
		const int ViewCreditsButton = 1679;
		const int BackButton = 1670;
	}

	namespace GameOptionsDialog {
		const int LoadGameButton = 1310;
		const int SaveGameButton = 1311;
		const int DeleteGameButton = 1312;
		const int GameControlsButton = 1313;
		const int AbortMissionButton = 1313;
		const int ResumeMissionButton = 1670;
	}
}

class Interface
{
public:
	Interface() = delete;
	~Interface() = delete;

	struct MenuItem {
		int nIDDlgItem;
		Ares::UISettings::UIAction uiaAction;
	};

	static YRDialogID lastDialogTemplateID;
	static int nextReturnMenu;
	static int nextAction;
	static const wchar_t* nextMessageText;

	static int slots[4];

	static bool invokeClickAction(Ares::UISettings::UIAction, const char*, int*, int);
	static void updateMenuItems(HWND hWnd, const MenuItem* items, size_t count);
	static void updateMenu(HWND hDlg, YRDialogID iID);
	static Ares::UISettings::UIAction parseUIAction(const char*, Ares::UISettings::UIAction);
	static int getSlotIndex(int);

private:
	static void moveItem(HWND, RECT, POINT);
	static void swapItems(HWND, int, int);
};
