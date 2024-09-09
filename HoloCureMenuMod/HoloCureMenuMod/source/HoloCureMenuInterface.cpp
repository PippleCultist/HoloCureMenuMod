#include "HoloCureMenuInterface.h"
#include "ModuleMain.h"
#include "YYToolkit/shared.hpp"
#include "CallbackManager/CallbackManagerInterface.h"
#include "CommonFunctions.h"

using namespace Aurie;
using namespace YYTK;

#define fa_left 0
#define fa_center 1
#define fa_right 2

extern HoloCureMenuInterface holoCureMenuInterface;
extern std::shared_ptr<menuGridData> curMenuGrid;
extern std::shared_ptr<menuGridData> modMenuGrid;
extern std::vector<std::string> modMenuGridNames;
extern std::vector<std::shared_ptr<menuGridData>> modMenuGridList;
extern std::string curMenuID;
extern CallbackManagerInterface* callbackManagerInterfacePtr;

int prevActionOneKey = -1;
int prevActionTwoKey = -1;

AurieStatus HoloCureMenuInterface::Create()
{
	return AURIE_SUCCESS;
}

void HoloCureMenuInterface::Destroy()
{

}

void HoloCureMenuInterface::QueryVersion(
	OUT short& Major,
	OUT short& Minor,
	OUT short& Patch
)
{
	Major = 1;
	Minor = 0;
	Patch = 0;
}

std::shared_ptr<menuData> curClickedField = nullptr;
int cursorTimer = 0;

AurieStatus HoloCureMenuInterface::CreateMenuGrid(
	IN const std::string& ModName,
	IN std::string menuGridName,
	IN std::shared_ptr<menuGridData>& prevMenuGridPtr,
	OUT std::shared_ptr<menuGridData>& menuGridPtr
)
{
	menuGridPtr = std::shared_ptr<menuGridData>(new menuGridData());
	menuGridPtr->prevMenu = prevMenuGridPtr;
	if (prevMenuGridPtr == nullptr)
	{
		menuGridPtr->prevMenu = modMenuGrid;
		modMenuGridNames.push_back(menuGridName);
		modMenuGridList.push_back(menuGridPtr);
	}
	return AURIE_SUCCESS;
}

AurieStatus HoloCureMenuInterface::DeleteMenuGrid(
	IN const std::string& ModName,
	IN std::shared_ptr<menuGridData>& menuGridPtr
)
{
	for (int i = 0; i < modMenuGridList.size(); i++)
	{
		if (modMenuGridList[i].get() == menuGridPtr.get())
		{
			modMenuGridList.erase(modMenuGridList.begin() + i);
			modMenuGridNames.erase(modMenuGridNames.begin() + i);
			break;
		}
	}
	return AURIE_SUCCESS;
}

AurieStatus HoloCureMenuInterface::CreateMenuColumn(
	IN const std::string& ModName,
	IN std::shared_ptr<menuGridData>& menuGridPtr,
	OUT std::shared_ptr<menuColumnData>& menuColumnPtr
)
{
	menuColumnPtr = std::shared_ptr<menuColumnData>(new menuColumnData());
	menuGridPtr->addMenuColumn(menuColumnPtr);
	return AURIE_SUCCESS;
}

AurieStatus HoloCureMenuInterface::AddMenuData(
	IN const std::string& ModName,
	IN std::shared_ptr<menuColumnData>& menuColumnPtr,
	IN std::shared_ptr<menuData>& addMenuDataPtr
)
{
	menuColumnPtr->addMenuData(addMenuDataPtr);
	return AURIE_SUCCESS;
}

AurieStatus HoloCureMenuInterface::SwapToMenuGrid(
	IN const std::string& ModName,
	IN std::shared_ptr<menuGridData>& menuGridPtr
)
{
	if (menuGridPtr == nullptr)
	{
		curMenuID = "";
		RValue titleScreen = g_ModuleInterface->CallBuiltin("instance_find", { objTitleScreenIndex, 0 });
		setInstanceVariable(titleScreen, GML_canControl, RValue(true));
	}
	if (menuGridPtr == nullptr || menuGridPtr == modMenuGrid)
	{
		if (prevActionOneKey != -1)
		{
			g_ModuleInterface->CallBuiltin("variable_global_get", { "theButtons" })[0] = static_cast<double>(prevActionOneKey);
			prevActionOneKey = -1;
			g_ModuleInterface->CallBuiltin("variable_global_get", { "theButtons" })[1] = static_cast<double>(prevActionTwoKey);
			prevActionTwoKey = -1;
			RValue setKeyboardControlsMethod = g_ModuleInterface->CallBuiltin("variable_global_get", { "SetKeyboardControls" });
			RValue setKeyboardControlsArray = g_ModuleInterface->CallBuiltin("array_create", { 0 });
			g_ModuleInterface->CallBuiltin("method_call", { setKeyboardControlsMethod, setKeyboardControlsArray });
		}
	}
	else
	{
		if (prevActionOneKey == -1)
		{
			RValue& actionOneKey = g_ModuleInterface->CallBuiltin("variable_global_get", { "theButtons" })[0];
			prevActionOneKey = static_cast<int>(lround(actionOneKey.m_Real));
			actionOneKey = -1;
			RValue& actionTwoKey = g_ModuleInterface->CallBuiltin("variable_global_get", { "theButtons" })[1];
			prevActionTwoKey = static_cast<int>(lround(actionTwoKey.m_Real));
			actionTwoKey = -1;
			RValue setKeyboardControlsMethod = g_ModuleInterface->CallBuiltin("variable_global_get", { "SetKeyboardControls" });
			RValue setKeyboardControlsArray = g_ModuleInterface->CallBuiltin("array_create", { 0 });
			g_ModuleInterface->CallBuiltin("method_call", { setKeyboardControlsMethod, setKeyboardControlsArray });
		}
	}
	curMenuGrid = menuGridPtr;
	if (curMenuGrid != nullptr && curMenuGrid->onEnterFunc)
	{
		curMenuGrid->onEnterFunc();
	}
	return AURIE_SUCCESS;
}

AurieStatus HoloCureMenuInterface::GetSelectedMenuData(
	IN const std::string& ModName,
	OUT std::shared_ptr<menuData>& menuDataPtr
)
{
	menuDataPtr = curMenuGrid->getSelectedMenuColumn()->getSelectedMenuData();
	return AURIE_SUCCESS;
}

AurieStatus HoloCureMenuInterface::GetCurrentMenuGrid(
	IN const std::string& ModName,
	OUT std::shared_ptr<menuGridData>& menuGridPtr
)
{
	menuGridPtr = curMenuGrid;
	return AURIE_SUCCESS;
}

void clickField()
{
	cursorTimer = 0;
	auto& curColumnPtr = curMenuGrid->menuColumnsPtrList[curMenuGrid->curSelectedColumnIndex];
	curClickedField = curColumnPtr->menuDataPtrList[curColumnPtr->curSelectedIndex];
	RValue keyboardString = curClickedField->textField;
	g_ModuleInterface->SetBuiltin("keyboard_string", nullptr, NULL_INDEX, keyboardString);
}

void unclickField()
{
	curClickedField = nullptr;
}

void menuGridData::processInput(bool isMouseLeftPressed, bool isMouseRightPressed, bool isActionOnePressed, bool isActionTwoPressed, bool isEnterPressed, bool isEscPressed,
	bool isMoveUpPressed, bool isMoveDownPressed, bool isMoveLeftPressed, bool isMoveRightPressed)
{
	auto curMenuColumnPtr = menuColumnsPtrList[curSelectedColumnIndex];
	if (isMouseLeftPressed || isActionOnePressed || isEnterPressed)
	{
		// Confirm
		unclickField();
		if (!curMenuID.empty())
		{
			auto& menuDataPtr = curMenuColumnPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex];
			auto funcPtr = menuDataPtr->clickMenuFunc;
			if (funcPtr != nullptr)
			{
				funcPtr();
			}
			else if (menuDataPtr->menuDataType == MENUDATATYPE_NumberField || menuDataPtr->menuDataType == MENUDATATYPE_TextBoxField)
			{
				clickField();
			}
		}
	}
	else if (isMouseRightPressed || isActionTwoPressed || isEscPressed)
	{
		// Return
		if (curClickedField == nullptr)
		{
			if (prevMenu == nullptr)
			{
				curMenuGrid = nullptr;
				holoCureMenuInterface.SwapToMenuGrid(MODNAME, curMenuGrid);
			}
			else
			{
				holoCureMenuInterface.SwapToMenuGrid(MODNAME, prevMenu);
			}
		}
		unclickField();
	}
	else if (isMoveUpPressed)
	{
		// Up
		if (curClickedField == nullptr)
		{
			int nextMenuDataIndex = curMenuColumnPtr->getMaxVisibleMenuDataIndex(curMenuColumnPtr->curSelectedIndex);
			if (nextMenuDataIndex != -1)
			{
				curMenuColumnPtr->curSelectedIndex = nextMenuDataIndex;
				curMenuID = curMenuColumnPtr->menuDataPtrList[nextMenuDataIndex]->menuID;
			}
		}
	}
	else if (isMoveDownPressed)
	{
		// Down
		if (curClickedField == nullptr)
		{
			int nextMenuDataIndex = curMenuColumnPtr->getMinVisibleMenuDataIndex(curMenuColumnPtr->curSelectedIndex);
			if (nextMenuDataIndex != -1)
			{
				curMenuColumnPtr->curSelectedIndex = nextMenuDataIndex;
				curMenuID = curMenuColumnPtr->menuDataPtrList[nextMenuDataIndex]->menuID;
			}
		}
	}
	else if (isMoveLeftPressed)
	{
		// Left
		if (curClickedField == nullptr)
		{
			auto& menuDataPtr = curMenuColumnPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex];
			if (menuDataPtr->menuDataType == MENUDATATYPE_Selection)
			{
				menuDataPtr->curSelectionTextIndex--;
				if (menuDataPtr->curSelectionTextIndex < 0)
				{
					menuDataPtr->curSelectionTextIndex = static_cast<int>(menuDataPtr->selectionText.size()) - 1;
				}
				return;
			}
			int nextColumnIndex = getMaxVisibleMenuColumnIndex(curSelectedColumnIndex);
			if (nextColumnIndex != -1)
			{
				curSelectedColumnIndex = nextColumnIndex;
				auto& curColumnListPtr = menuColumnsPtrList[curSelectedColumnIndex];
				if (curMenuColumnPtr->curSelectedIndex == -1)
				{
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Menu index is set to -1");
					g_ModuleInterface->Print(CM_RED, "Menu index is set to -1");
					return;
				}
				if (!curColumnListPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex]->isVisible)
				{
					curMenuColumnPtr->curSelectedIndex = curMenuColumnPtr->getMinVisibleMenuDataIndex(-1);
					if (curMenuColumnPtr->curSelectedIndex == -1)
					{
						callbackManagerInterfacePtr->LogToFile(MODNAME, "Menu index not found");
						g_ModuleInterface->Print(CM_RED, "Menu index not found");
						return;
					}
				}
				curMenuID = curColumnListPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex]->menuID;
			}
		}
	}
	else if (isMoveRightPressed)
	{
		// Right
		if (curClickedField == nullptr)
		{
			auto& menuDataPtr = curMenuColumnPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex];
			if (menuDataPtr->menuDataType == MENUDATATYPE_Selection)
			{
				menuDataPtr->curSelectionTextIndex++;
				if (menuDataPtr->curSelectionTextIndex >= menuDataPtr->selectionText.size())
				{
					menuDataPtr->curSelectionTextIndex = 0;
				}
				return;
			}
			int nextColumnIndex = getMinVisibleMenuColumnIndex(curSelectedColumnIndex);
			if (nextColumnIndex != -1)
			{
				curSelectedColumnIndex = nextColumnIndex;
				auto& curColumnListPtr = menuColumnsPtrList[curSelectedColumnIndex];
				if (curMenuColumnPtr->curSelectedIndex == -1)
				{
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Menu index is set to -1");
					g_ModuleInterface->Print(CM_RED, "Menu index is set to -1");
					return;
				}
				if (!curColumnListPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex]->isVisible)
				{
					curMenuColumnPtr->curSelectedIndex = curMenuColumnPtr->getMinVisibleMenuDataIndex(-1);
					if (curMenuColumnPtr->curSelectedIndex == -1)
					{
						callbackManagerInterfacePtr->LogToFile(MODNAME, "Menu index not found");
						g_ModuleInterface->Print(CM_RED, "Menu index not found");
						return;
					}
				}
				curMenuID = curColumnListPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex]->menuID;
			}
		}
	}
}

void menuGridData::resetMenu()
{
	curSelectedColumnIndex = defaultCurSelectedColumnIndex;
	for (auto& menuColumnPtr : menuColumnsPtrList)
	{
		menuColumnPtr->resetToDefault();
	}
}

void menuColumnData::resetToDefault()
{
	curSelectedIndex = defaultCurSelectedIndex;
	for (auto& menuDataPtr : menuDataPtrList)
	{
		menuDataPtr->resetToDefault();
	}
}

void menuData::resetToDefault()
{
	labelName = defaultLabelName;
	isVisible = defaultIsVisible;
}

int menuColumnData::getMinVisibleMenuDataIndex(int curMenuDataIndex)
{
	for (int i = curMenuDataIndex + 1; i < menuDataPtrList.size(); i++)
	{
		if (menuDataPtrList[i]->isVisible)
		{
			return i;
		}
	}
	return -1;
}

int menuColumnData::getMaxVisibleMenuDataIndex(int curMenuDataIndex)
{
	for (int i = curMenuDataIndex - 1; i >= 0; i--)
	{
		if (menuDataPtrList[i]->isVisible)
		{
			return i;
		}
	}
	return -1;
}

int menuGridData::getMinVisibleMenuColumnIndex(int curMenuColumnIndex)
{
	for (int i = curMenuColumnIndex + 1; i < menuColumnsPtrList.size(); i++)
	{
		if (menuColumnsPtrList[i]->getMinVisibleMenuDataIndex(-1) != -1)
		{
			return i;
		}
	}
	return -1;
}

int menuGridData::getMaxVisibleMenuColumnIndex(int curMenuColumnIndex)
{
	for (int i = curMenuColumnIndex - 1; i >= 0; i--)
	{
		if (menuColumnsPtrList[i]->getMinVisibleMenuDataIndex(-1) != -1)
		{
			return i;
		}
	}
	return -1;
}

std::shared_ptr<menuColumnData>& menuGridData::getSelectedMenuColumn()
{
	return menuColumnsPtrList[curSelectedColumnIndex];
}

std::shared_ptr<menuData>& menuColumnData::getSelectedMenuData()
{
	return menuDataPtrList[curSelectedIndex];
}

void splitWrappingText(std::vector<std::string>& textList, std::string drawStr, double sizeOfLineWrap)
{
	if (drawStr.empty())
	{
		textList.push_back("");
		return;
	}
	double drawnTextSize = g_ModuleInterface->CallBuiltin("string_width", { drawStr }).m_Real;
	while (drawnTextSize >= sizeOfLineWrap)
	{
		int low = 1;
		int high = static_cast<int>(drawStr.size() - 1);
		int numCharDrawn = 1;
		while (low <= high)
		{
			int mid = (high + low) / 2;
			numCharDrawn = mid;
			double curDrawnTextSize = g_ModuleInterface->CallBuiltin("string_width", { drawStr.substr(0, numCharDrawn) }).m_Real;
			if (curDrawnTextSize == sizeOfLineWrap)
			{
				break;
			}
			else if (curDrawnTextSize > sizeOfLineWrap)
			{
				high = mid - 1;
			}
			else
			{
				low = mid + 1;
			}
		}
		textList.push_back(drawStr.substr(0, numCharDrawn));
		drawStr = drawStr.substr(numCharDrawn);
		drawnTextSize = g_ModuleInterface->CallBuiltin("string_width", { drawStr }).m_Real;
	}
	if (drawnTextSize != 0)
	{
		textList.push_back(drawStr);
	}
}

void menuGridData::draw(CInstance* Self)
{
	curMenuID = "";
	double textColor[2]{ 0xFFFFFF, 0 };
	RValue inputManager = g_ModuleInterface->CallBuiltin("instance_find", { objInputManagerIndex, 0 });
	for (int i = 0; i < menuColumnsPtrList.size(); i++)
	{
		auto& curMenuColumnPtr = menuColumnsPtrList[i];
		for (int j = 0; j < curMenuColumnPtr->menuDataPtrList.size(); j++)
		{
			auto& curMenuDataPtr = curMenuColumnPtr->menuDataPtrList[j];
			if (!curMenuDataPtr->isVisible)
			{
				continue;
			}

			int offsetX = 0;
			if (curMenuDataPtr->menuDataType == MENUDATATYPE_Button)
			{
				offsetX -= 15;
			}
			RValue mouseX;
			RValue mouseY;
			g_ModuleInterface->GetBuiltin("mouse_x", nullptr, NULL_INDEX, mouseX);
			g_ModuleInterface->GetBuiltin("mouse_y", nullptr, NULL_INDEX, mouseY);
			if (mouseX.m_Real >= curMenuDataPtr->xPos + offsetX && mouseX.m_Real <= curMenuDataPtr->xPos + offsetX + curMenuDataPtr->width
				&& mouseY.m_Real >= curMenuDataPtr->yPos && mouseY.m_Real <= curMenuDataPtr->yPos + curMenuDataPtr->height)
			{
				curMenuID = curMenuDataPtr->menuID;
				curSelectedColumnIndex = i;
				curMenuColumnPtr->curSelectedIndex = j;
			}
			int isOptionSelected = (curSelectedColumnIndex == i) && (curMenuColumnPtr->curSelectedIndex == j);
			g_ModuleInterface->CallBuiltin("draw_set_font", { jpFont });
			if (curMenuDataPtr->menuDataType == MENUDATATYPE_Button)
			{
				offsetX += 90;
				auto curTextColor = textColor[isOptionSelected];
				g_ModuleInterface->CallBuiltin("draw_set_halign", { fa_center });
				g_ModuleInterface->CallBuiltin("draw_sprite_ext", { sprHudInitButtonsIndex, isOptionSelected, curMenuDataPtr->xPos + offsetX, curMenuDataPtr->yPos, 1, 1, 0, static_cast<double>(0xFFFFFF), 1 });
				g_ModuleInterface->CallBuiltin("draw_text_color", { curMenuDataPtr->xPos + offsetX, curMenuDataPtr->yPos + 9, curMenuDataPtr->labelName, curTextColor, curTextColor, curTextColor, curTextColor, 1 });
			}
			else if (curMenuDataPtr->menuDataType == MENUDATATYPE_TextBoxField || curMenuDataPtr->menuDataType == MENUDATATYPE_NumberField)
			{
				auto curTextColor = textColor[1];
				g_ModuleInterface->CallBuiltin("draw_set_halign", { fa_right });
				g_ModuleInterface->CallBuiltin("draw_set_color", { curTextColor });
				g_ModuleInterface->CallBuiltin("draw_text", { curMenuDataPtr->xPos + offsetX, curMenuDataPtr->yPos + 9, curMenuDataPtr->labelName });
				g_ModuleInterface->CallBuiltin("draw_set_halign", { fa_left });
				g_ModuleInterface->CallBuiltin("draw_rectangle_color", { curMenuDataPtr->xPos + offsetX, curMenuDataPtr->yPos, curMenuDataPtr->xPos + offsetX + curMenuDataPtr->width, curMenuDataPtr->yPos + curMenuDataPtr->height, textColor[0], textColor[0], textColor[0], textColor[0], false });
				std::vector<std::string> textList;
				if (curClickedField != nullptr && curClickedField->menuID == curMenuDataPtr->menuID)
				{
					RValue keyboardString;
					g_ModuleInterface->GetBuiltin("keyboard_string", nullptr, NULL_INDEX, keyboardString);
					cursorTimer = (cursorTimer + 1) % 120;
					auto lineColor = textColor[1 - cursorTimer / 60];
					splitWrappingText(textList, std::string(keyboardString.AsString()), curMenuDataPtr->width);

					if (textList.size() <= curMenuDataPtr->height / 20)
					{
						curClickedField->textField = keyboardString.AsString();
					}
					else
					{
						keyboardString = curClickedField->textField;
						g_ModuleInterface->SetBuiltin("keyboard_string", nullptr, NULL_INDEX, keyboardString);
						textList.pop_back();
					}
					int cursorOffsetX = static_cast<int>(lround(g_ModuleInterface->CallBuiltin("string_width", { textList[textList.size() - 1] }).m_Real));
					int cursorOffsetY = (static_cast<int>(textList.size()) - 1) * 20;
					g_ModuleInterface->CallBuiltin("draw_line_width_colour", { curMenuDataPtr->xPos + offsetX + cursorOffsetX, curMenuDataPtr->yPos + cursorOffsetY + 2, curMenuDataPtr->xPos + offsetX + cursorOffsetX, curMenuDataPtr->yPos + cursorOffsetY + 12, 1, lineColor, lineColor });
				}
				else
				{
					splitWrappingText(textList, curMenuDataPtr->textField, curMenuDataPtr->width);
				}
				for (int i = 0; i < textList.size(); i++)
				{
					g_ModuleInterface->CallBuiltin("draw_text", { curMenuDataPtr->xPos, curMenuDataPtr->yPos + i * 20 + 2, textList[i] });
				}
			}
			else if (curMenuDataPtr->menuDataType == MENUDATATYPE_Image)
			{
				if (curMenuDataPtr->curFrameCount != -1)
				{
					curMenuDataPtr->curFrameCount++;
					if (curMenuDataPtr->curFrameCount >= 60 / curMenuDataPtr->fps)
					{
						curMenuDataPtr->curFrameCount = 0;
						curMenuDataPtr->curSubImageIndex++;
						if (curMenuDataPtr->curSubImageIndex >= curMenuDataPtr->curSprite->numFrames)
						{
							curMenuDataPtr->curSubImageIndex = 0;
						}
					}
				}
				if (curMenuDataPtr->curSprite != nullptr)
				{
					g_ModuleInterface->CallBuiltin("draw_sprite", { curMenuDataPtr->curSprite->spriteRValue, curMenuDataPtr->curSubImageIndex, curMenuDataPtr->xPos, curMenuDataPtr->yPos });
				}
			}
			else if (curMenuDataPtr->menuDataType == MENUDATATYPE_Text)
			{
				if (curMenuDataPtr->labelNameFunc != nullptr)
				{
					curMenuDataPtr->labelNameFunc();
				}
				g_ModuleInterface->CallBuiltin("draw_set_halign", { fa_left });
				g_ModuleInterface->CallBuiltin("draw_text", { curMenuDataPtr->xPos, curMenuDataPtr->yPos, curMenuDataPtr->labelName });
			}
			else if (curMenuDataPtr->menuDataType == MENUDATATYPE_Selection)
			{
				offsetX += 90;
				auto curTextColor = textColor[isOptionSelected];
				g_ModuleInterface->CallBuiltin("draw_set_halign", { fa_left });
				g_ModuleInterface->CallBuiltin("draw_sprite_ext", { sprHudOptionButton, isOptionSelected, curMenuDataPtr->xPos + offsetX, curMenuDataPtr->yPos, 1, 1, 0, static_cast<double>(0xFFFFFF), 1 });
				g_ModuleInterface->CallBuiltin("draw_text_color", { curMenuDataPtr->xPos + offsetX - 78, curMenuDataPtr->yPos + 8, curMenuDataPtr->labelName, curTextColor, curTextColor, curTextColor, curTextColor, 1 });
				if (isOptionSelected)
				{
					g_ModuleInterface->CallBuiltin("draw_sprite_ext", { sprHudScrollArrows2, 0, curMenuDataPtr->xPos + offsetX + 40 - 38, curMenuDataPtr->yPos + 13, 1, 1, 0, static_cast<double>(0xFFFFFF), 1 });
					g_ModuleInterface->CallBuiltin("draw_sprite_ext", { sprHudScrollArrows2, 1, curMenuDataPtr->xPos + offsetX + 40 + 38, curMenuDataPtr->yPos + 13, 1, 1, 0, static_cast<double>(0xFFFFFF), 1 });
				}
				if (curMenuDataPtr->curSelectionTextIndex >= 0 && curMenuDataPtr->curSelectionTextIndex < curMenuDataPtr->selectionText.size())
				{
					g_ModuleInterface->CallBuiltin("draw_set_halign", { fa_center });
					g_ModuleInterface->CallBuiltin("draw_text_color", { curMenuDataPtr->xPos + offsetX + 40, curMenuDataPtr->yPos + 8, curMenuDataPtr->selectionText[curMenuDataPtr->curSelectionTextIndex], curTextColor, curTextColor, curTextColor, curTextColor, 1 });
				}
			}
		}
	}
}