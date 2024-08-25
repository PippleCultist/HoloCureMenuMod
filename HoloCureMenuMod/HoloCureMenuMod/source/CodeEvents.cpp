#include "HoloCureMenuInterface.h"
#include "ModuleMain.h"
#include "CommonFunctions.h"

extern CallbackManagerInterface* callbackManagerInterfacePtr;
extern HoloCureMenuInterface holoCureMenuInterface;

std::shared_ptr<menuGridData> curMenuGrid = nullptr;
std::string curMenuID;
std::vector<std::string> modMenuGridNames;
std::vector<std::shared_ptr<menuGridData>> modMenuGridList;
int modMenuGridPage = 0;

void clickButton()
{
	int menuGridIndex = modMenuGridPage * 8 + curMenuGrid->getSelectedMenuColumn()->curSelectedIndex;
	if (menuGridIndex >= modMenuGridList.size())
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Invalid mod menu index %d out of %d", menuGridIndex, static_cast<int>(modMenuGridList.size()));
		return;
	}
	// Assume that this should be the play button
	if (modMenuGridList[menuGridIndex] == nullptr)
	{
		curMenuGrid = nullptr;
		holoCureMenuInterface.SwapToMenuGrid(MODNAME, curMenuGrid);
		g_ModuleInterface->CallBuiltin("room_goto", { rmCharSelect });
		return;
	}
	holoCureMenuInterface.SwapToMenuGrid(MODNAME, modMenuGridList[menuGridIndex]);
}

void reloadGridMenu()
{
	auto& menuDataList = curMenuGrid->menuColumnsPtrList[0]->menuDataPtrList;
	for (size_t i = 0; i < menuDataList.size() - 2; i++)
	{
		menuDataList[i]->labelName = "";
		menuDataList[i]->isVisible = false;
	}
	for (int i = 0; i < 8 && modMenuGridPage * 8 + i < modMenuGridNames.size(); i++)
	{
		auto& menuData = menuDataList[i];
		menuData->labelName = modMenuGridNames[modMenuGridPage * 8 + i];
		menuData->isVisible = true;
	}
}

void prevButton()
{
	if (modMenuGridPage > 0)
	{
		modMenuGridPage--;
		reloadGridMenu();
	}
}

void nextButton()
{
	if (modMenuGridPage < (modMenuGridList.size() - 1) / 8)
	{
		modMenuGridPage++;
		reloadGridMenu();
	}
}

std::shared_ptr<menuGridData> modMenuGrid = std::shared_ptr<menuGridData>(
	new menuGridData({
		std::shared_ptr<menuColumnData>(new menuColumnData({
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 0, 180, 20, "TITLEMENU_MenuGridButton", "", false, clickButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "TITLEMENU_MenuGridButton", "", false, clickButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "TITLEMENU_MenuGridButton", "", false, clickButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 3, 180, 20, "TITLEMENU_MenuGridButton", "", false, clickButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 4, 180, 20, "TITLEMENU_MenuGridButton", "", false, clickButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 5, 180, 20, "TITLEMENU_MenuGridButton", "", false, clickButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 6, 180, 20, "TITLEMENU_MenuGridButton", "", false, clickButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 7, 180, 20, "TITLEMENU_MenuGridButton", "", false, clickButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 8, 180, 20, "TITLEMENU_PrevButton", "Prev", true, prevButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 9, 180, 20, "TITLEMENU_NextButton", "Next", true, nextButton, nullptr, MENUDATATYPE_Button)),
		}, 0)
		)
	}, 0, nullptr)
);

void loadModMenu()
{
	holoCureMenuInterface.SwapToMenuGrid(MODNAME, modMenuGrid);
	reloadGridMenu();
}

void InputManagerStepAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	CInstance* Self = std::get<0>(Args);
	if (curMenuGrid != nullptr)
	{
		curMenuGrid->processInput(
			g_ModuleInterface->CallBuiltin("mouse_check_button_pressed", { 1 }).AsBool(),
			g_ModuleInterface->CallBuiltin("mouse_check_button_pressed", { 2 }).AsBool(),
			getInstanceVariable(Self, GML_actionOnePressed).AsBool(),
			getInstanceVariable(Self, GML_actionTwoPressed).AsBool(),
			getInstanceVariable(Self, GML_enterPressed).AsBool(),
			getInstanceVariable(Self, GML_escPressed).AsBool(),
			getInstanceVariable(Self, GML_moveUpPressed).AsBool(),
			getInstanceVariable(Self, GML_moveDownPressed).AsBool(),
			getInstanceVariable(Self, GML_moveLeftPressed).AsBool(),
			getInstanceVariable(Self, GML_moveRightPressed).AsBool()
		);
	}
}

void TitleScreenDrawBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	CInstance* Self = std::get<0>(Args);
	if (curMenuGrid != nullptr)
	{
		curMenuGrid->draw(Self);
	}
}

void TitleCharacterDrawBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	if (curMenuGrid != nullptr)
	{
		callbackManagerInterfacePtr->CancelOriginalFunction();
	}
}

void TextControllerCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	RValue textContainer = g_ModuleInterface->CallBuiltin("variable_global_get", { "TextContainer" });
	textContainer["titleButtons"]["eng"][0] = "Play Modded";
}