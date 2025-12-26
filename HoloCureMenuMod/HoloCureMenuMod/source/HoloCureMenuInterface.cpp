#include "HoloCureMenuInterface.h"
#include "ModuleMain.h"
#include "YYToolkit/YYTK_Shared.hpp"
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
	RValue titleScreen = g_ModuleInterface->CallBuiltin("instance_find", { objTitleScreenIndex, 0 });
	if (menuGridPtr == nullptr)
	{
		curMenuID = "";
		setInstanceVariable(titleScreen, GML_canControl, RValue(true));
	}
	else
	{
		setInstanceVariable(titleScreen, GML_canControl, RValue(false));
	}
	if (menuGridPtr == nullptr || menuGridPtr == modMenuGrid)
	{
		EnableActionButtons(ModName);
	}
	else
	{
		DisableActionButtons(ModName);
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

AurieStatus HoloCureMenuInterface::DisableActionButtons(
	IN const std::string& ModName
)
{
	if (prevActionOneKey == -1)
	{
		RValue& actionOneKey = g_ModuleInterface->CallBuiltin("variable_global_get", { "theButtons" })[0];
		prevActionOneKey = static_cast<int>(lround(actionOneKey.ToDouble()));
		actionOneKey = -1;
		RValue& actionTwoKey = g_ModuleInterface->CallBuiltin("variable_global_get", { "theButtons" })[1];
		prevActionTwoKey = static_cast<int>(lround(actionTwoKey.ToDouble()));
		actionTwoKey = -1;
		RValue setKeyboardControlsMethod = g_ModuleInterface->CallBuiltin("variable_global_get", { "SetKeyboardControls" });
		RValue setKeyboardControlsArray = g_ModuleInterface->CallBuiltin("array_create", { 0 });
		g_ModuleInterface->CallBuiltin("method_call", { setKeyboardControlsMethod, setKeyboardControlsArray });
	}
	return AURIE_SUCCESS;
}

AurieStatus HoloCureMenuInterface::EnableActionButtons(
	IN const std::string& ModName
)
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
	return AURIE_SUCCESS;
}

void clickField()
{
	cursorTimer = 0;
	auto& curColumnPtr = curMenuGrid->menuColumnsPtrList[curMenuGrid->curSelectedColumnIndex];
	curClickedField = curColumnPtr->menuDataPtrList[curColumnPtr->curSelectedIndex];
	RValue keyboardString;
	if (curClickedField->menuDataType == MENUDATATYPE_NumberField)
	{
		keyboardString = static_cast<menuDataNumberField*>(curClickedField.get())->textField.c_str();
	}
	else if (curClickedField->menuDataType == MENUDATATYPE_TextBoxField)
	{
		keyboardString = static_cast<menuDataTextBoxField*>(curClickedField.get())->textField.c_str();
	}
	else
	{
		keyboardString = "";
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Invalid click type for HoloCureMenuMod");
	}
	g_ModuleInterface->SetBuiltin("keyboard_string", nullptr, NULL_INDEX, keyboardString);
}

void unclickField()
{
	curClickedField = nullptr;
}

bool clickedMenuFunc(std::shared_ptr<menuData> menuDataPtr)
{
	menuFunc funcPtr = nullptr;
	switch (menuDataPtr->menuDataType)
	{
		case MENUDATATYPE_Button:
		{
			funcPtr = static_cast<menuDataButton*>(menuDataPtr.get())->clickMenuFunc;
			break;
		}
		case MENUDATATYPE_NumberField:
		{
			funcPtr = static_cast<menuDataNumberField*>(menuDataPtr.get())->clickMenuFunc;
			break;
		}
		case MENUDATATYPE_TextBoxField:
		{
			funcPtr = static_cast<menuDataTextBoxField*>(menuDataPtr.get())->clickMenuFunc;
			break;
		}
		case MENUDATATYPE_Selection:
		{
			funcPtr = static_cast<menuDataSelection*>(menuDataPtr.get())->clickMenuFunc;
			break;
		}
	}
	if (funcPtr != nullptr)
	{
		funcPtr();
		return true;
	}
	return false;
}

void menuGridData::processInput(bool isMouseLeftPressed, bool isMouseRightPressed, bool isActionOnePressed, bool isActionTwoPressed, bool isEnterPressed, bool isEscPressed,
	bool isMoveUpPressed, bool isMoveDownPressed, bool isMoveLeftPressed, bool isMoveRightPressed)
{
	std::shared_ptr<menuColumnData> curMenuColumnPtr = nullptr;
	if (curSelectedColumnIndex < menuColumnsPtrList.size())
	{
		curMenuColumnPtr = menuColumnsPtrList[curSelectedColumnIndex];
	}

	if (isMouseLeftPressed || isActionOnePressed || isEnterPressed)
	{
		// Confirm
		unclickField();
		if (!curMenuID.empty() && curMenuColumnPtr != nullptr)
		{
			auto& menuDataPtr = curMenuColumnPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex];
			if (!clickedMenuFunc(menuDataPtr) && menuDataPtr->menuDataType == MENUDATATYPE_NumberField || menuDataPtr->menuDataType == MENUDATATYPE_TextBoxField)
			{
				clickField();
			}
		}
	}
	else if (isMouseRightPressed || isActionTwoPressed || isEscPressed)
	{
		// Return
		if (curMenuGrid->onReturnFunc)
		{
			curMenuGrid->onReturnFunc();
		}
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
		if (curClickedField == nullptr && curMenuColumnPtr != nullptr)
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
		if (curClickedField == nullptr && curMenuColumnPtr != nullptr)
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
		if (curClickedField == nullptr && curMenuColumnPtr != nullptr)
		{
			auto& menuDataPtr = curMenuColumnPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex];
			if (menuDataPtr->menuDataType == MENUDATATYPE_Selection)
			{
				menuDataSelection* curMenuDataSelection = static_cast<menuDataSelection*>(menuDataPtr.get());
				curMenuDataSelection->curSelectionTextIndex--;
				if (curMenuDataSelection->curSelectionTextIndex < 0)
				{
					curMenuDataSelection->curSelectionTextIndex = static_cast<int>(curMenuDataSelection->selectionText.size()) - 1;
				}
				auto funcPtr = curMenuDataSelection->clickMenuFunc;
				if (funcPtr != nullptr)
				{
					funcPtr();
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
					DbgPrintEx(LOG_SEVERITY_ERROR, "Menu index is set to -1");
					return;
				}
				if (!curColumnListPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex]->isVisible)
				{
					curMenuColumnPtr->curSelectedIndex = curMenuColumnPtr->getMinVisibleMenuDataIndex(-1);
					if (curMenuColumnPtr->curSelectedIndex == -1)
					{
						callbackManagerInterfacePtr->LogToFile(MODNAME, "Menu index not found");
						DbgPrintEx(LOG_SEVERITY_ERROR, "Menu index not found");
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
		if (curClickedField == nullptr && curMenuColumnPtr != nullptr)
		{
			auto& menuDataPtr = curMenuColumnPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex];
			if (menuDataPtr->menuDataType == MENUDATATYPE_Selection)
			{
				menuDataSelection* curMenuDataSelection = static_cast<menuDataSelection*>(menuDataPtr.get());
				curMenuDataSelection->curSelectionTextIndex++;
				if (curMenuDataSelection->curSelectionTextIndex >= curMenuDataSelection->selectionText.size())
				{
					curMenuDataSelection->curSelectionTextIndex = 0;
				}
				auto funcPtr = curMenuDataSelection->clickMenuFunc;
				if (funcPtr != nullptr)
				{
					funcPtr();
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
					DbgPrintEx(LOG_SEVERITY_ERROR, "Menu index is set to -1");
					return;
				}
				if (!curColumnListPtr->menuDataPtrList[curMenuColumnPtr->curSelectedIndex]->isVisible)
				{
					curMenuColumnPtr->curSelectedIndex = curMenuColumnPtr->getMinVisibleMenuDataIndex(-1);
					if (curMenuColumnPtr->curSelectedIndex == -1)
					{
						callbackManagerInterfacePtr->LogToFile(MODNAME, "Menu index not found");
						DbgPrintEx(LOG_SEVERITY_ERROR, "Menu index not found");
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
	double drawnTextSize = g_ModuleInterface->CallBuiltin("string_width", { drawStr.c_str()}).ToDouble();
	while (drawnTextSize >= sizeOfLineWrap)
	{
		int low = 1;
		int high = static_cast<int>(drawStr.size() - 1);
		int numCharDrawn = 1;
		while (low <= high)
		{
			int mid = (high + low) / 2;
			numCharDrawn = mid;
			double curDrawnTextSize = g_ModuleInterface->CallBuiltin("string_width", { drawStr.substr(0, numCharDrawn).c_str() }).ToDouble();
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
		drawnTextSize = g_ModuleInterface->CallBuiltin("string_width", { drawStr.c_str() }).ToDouble();
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
	if (drawFunc != nullptr)
	{
		drawFunc();
	}
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
			if (mouseX.ToDouble() >= curMenuDataPtr->xPos + offsetX && mouseX.ToDouble() < curMenuDataPtr->xPos + offsetX + curMenuDataPtr->width
				&& mouseY.ToDouble() >= curMenuDataPtr->yPos && mouseY.ToDouble() < curMenuDataPtr->yPos + curMenuDataPtr->height)
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
				g_ModuleInterface->CallBuiltin("draw_text_color", { curMenuDataPtr->xPos + offsetX, curMenuDataPtr->yPos + 9, curMenuDataPtr->labelName.c_str(), curTextColor, curTextColor, curTextColor, curTextColor, 1});
			}
			else if (curMenuDataPtr->menuDataType == MENUDATATYPE_TextBoxField || curMenuDataPtr->menuDataType == MENUDATATYPE_NumberField)
			{
				auto curTextColor = textColor[1];
				g_ModuleInterface->CallBuiltin("draw_set_halign", { fa_right });
				g_ModuleInterface->CallBuiltin("draw_set_color", { curTextColor });
				g_ModuleInterface->CallBuiltin("draw_text", { curMenuDataPtr->xPos + offsetX, curMenuDataPtr->yPos + 9, curMenuDataPtr->labelName.c_str() });
				g_ModuleInterface->CallBuiltin("draw_set_halign", { fa_left });
				g_ModuleInterface->CallBuiltin("draw_rectangle_color", { curMenuDataPtr->xPos + offsetX, curMenuDataPtr->yPos, curMenuDataPtr->xPos + offsetX + curMenuDataPtr->width, curMenuDataPtr->yPos + curMenuDataPtr->height, textColor[0], textColor[0], textColor[0], textColor[0], false });
				std::vector<std::string> textList;
				if (curClickedField != nullptr && curClickedField->menuID == curMenuDataPtr->menuID)
				{
					RValue keyboardString;
					g_ModuleInterface->GetBuiltin("keyboard_string", nullptr, NULL_INDEX, keyboardString);
					cursorTimer = (cursorTimer + 1) % 120;
					auto lineColor = textColor[1 - cursorTimer / 60];
					splitWrappingText(textList, keyboardString.ToString(), curMenuDataPtr->width);

					if (curMenuDataPtr->menuDataType == MENUDATATYPE_TextBoxField)
					{
						auto curTextBoxField = static_cast<menuDataTextBoxField*>(curMenuDataPtr.get());
						if (textList.size() <= curMenuDataPtr->height / 20)
						{
							curTextBoxField->textField = keyboardString.ToString();
						}
						else
						{
							keyboardString = curTextBoxField->textField.c_str();
							g_ModuleInterface->SetBuiltin("keyboard_string", nullptr, NULL_INDEX, keyboardString);
							textList.pop_back();
						}
					}
					else
					{
						auto curNumberField = static_cast<menuDataNumberField*>(curMenuDataPtr.get());
						if (textList.size() <= curMenuDataPtr->height / 20)
						{
							curNumberField->textField = keyboardString.ToString();
						}
						else
						{
							keyboardString = curNumberField->textField.c_str();
							g_ModuleInterface->SetBuiltin("keyboard_string", nullptr, NULL_INDEX, keyboardString);
							textList.pop_back();
						}
					}
					int cursorOffsetX = static_cast<int>(lround(g_ModuleInterface->CallBuiltin("string_width", { textList[textList.size() - 1].c_str() }).ToDouble()));
					int cursorOffsetY = (static_cast<int>(textList.size()) - 1) * 20;
					g_ModuleInterface->CallBuiltin("draw_line_width_colour", { curMenuDataPtr->xPos + offsetX + cursorOffsetX, curMenuDataPtr->yPos + cursorOffsetY + 2, curMenuDataPtr->xPos + offsetX + cursorOffsetX, curMenuDataPtr->yPos + cursorOffsetY + 12, 1, lineColor, lineColor });
				}
				else
				{
					if (curMenuDataPtr->menuDataType == MENUDATATYPE_TextBoxField)
					{
						splitWrappingText(textList, static_cast<menuDataTextBoxField*>(curMenuDataPtr.get())->textField, curMenuDataPtr->width);
					}
					else
					{
						splitWrappingText(textList, static_cast<menuDataNumberField*>(curMenuDataPtr.get())->textField, curMenuDataPtr->width);
					}
				}
				for (int i = 0; i < textList.size(); i++)
				{
					g_ModuleInterface->CallBuiltin("draw_text", { curMenuDataPtr->xPos, curMenuDataPtr->yPos + i * 20 + 2, textList[i].c_str() });
				}
			}
			else if (curMenuDataPtr->menuDataType == MENUDATATYPE_Image)
			{
				auto curImage = static_cast<menuDataImageField*>(curMenuDataPtr.get());
				if (curImage->curFrameCount != -1)
				{
					curImage->curFrameCount++;
					if (curImage->curFrameCount >= 60 / curImage->fps)
					{
						curImage->curFrameCount = 0;
						curImage->curSubImageIndex++;
						if (curImage->curSubImageIndex >= curImage->curSprite->numFrames)
						{
							curImage->curSubImageIndex = 0;
						}
					}
				}
				if (curImage->curSprite != nullptr)
				{
					g_ModuleInterface->CallBuiltin("draw_sprite", { curImage->curSprite->spriteRValue, curImage->curSubImageIndex, curImage->xPos, curImage->yPos });
				}
			}
			else if (curMenuDataPtr->menuDataType == MENUDATATYPE_Text)
			{
				auto curText = static_cast<menuDataText*>(curMenuDataPtr.get());
				if (curText->labelNameFunc != nullptr)
				{
					curText->labelNameFunc();
				}
				g_ModuleInterface->CallBuiltin("draw_set_halign", { fa_left });
				g_ModuleInterface->CallBuiltin("draw_text", { curMenuDataPtr->xPos, curMenuDataPtr->yPos, curMenuDataPtr->labelName.c_str() });
			}
			else if (curMenuDataPtr->menuDataType == MENUDATATYPE_Selection)
			{
				auto curSelection = static_cast<menuDataSelection*>(curMenuDataPtr.get());
				offsetX += 90;
				auto curTextColor = textColor[isOptionSelected];
				g_ModuleInterface->CallBuiltin("draw_set_halign", { fa_left });
				g_ModuleInterface->CallBuiltin("draw_sprite_ext", { sprHudOptionButton, isOptionSelected, curMenuDataPtr->xPos + offsetX, curMenuDataPtr->yPos, 1, 1, 0, static_cast<double>(0xFFFFFF), 1 });
				g_ModuleInterface->CallBuiltin("draw_text_color", { curMenuDataPtr->xPos + offsetX - 78, curMenuDataPtr->yPos + 8, curMenuDataPtr->labelName.c_str(), curTextColor, curTextColor, curTextColor, curTextColor, 1});
				if (isOptionSelected)
				{
					g_ModuleInterface->CallBuiltin("draw_sprite_ext", { sprHudScrollArrows2, 0, curMenuDataPtr->xPos + offsetX + 40 - 38, curMenuDataPtr->yPos + 13, 1, 1, 0, static_cast<double>(0xFFFFFF), 1 });
					g_ModuleInterface->CallBuiltin("draw_sprite_ext", { sprHudScrollArrows2, 1, curMenuDataPtr->xPos + offsetX + 40 + 38, curMenuDataPtr->yPos + 13, 1, 1, 0, static_cast<double>(0xFFFFFF), 1 });
				}
				if (curSelection->curSelectionTextIndex >= 0 && curSelection->curSelectionTextIndex < curSelection->selectionText.size())
				{
					g_ModuleInterface->CallBuiltin("draw_set_halign", { fa_center });
					g_ModuleInterface->CallBuiltin("draw_text_color", { curMenuDataPtr->xPos + offsetX + 40, curMenuDataPtr->yPos + 8, curSelection->selectionText[curSelection->curSelectionTextIndex].c_str(), curTextColor, curTextColor, curTextColor, curTextColor, 1});
				}
			}
			else if (curMenuDataPtr->menuDataType == MENUDATATYPE_TextOutline)
			{
				auto curTextOutline = static_cast<menuDataTextOutline*>(curMenuDataPtr.get());
				if (curTextOutline->labelNameFunc != nullptr)
				{
					curTextOutline->labelNameFunc();
				}
				RValue** args = new RValue*[10];
				args[0] = new RValue(curTextOutline->xPos);
				args[1] = new RValue(curTextOutline->yPos);
				args[2] = new RValue(curTextOutline->labelName);
				args[3] = new RValue(curTextOutline->outlineWidth);
				args[4] = new RValue(curTextOutline->outlineColor);
				args[5] = new RValue(curTextOutline->numOutline);
				args[6] = new RValue(curTextOutline->linePixelSeparation);
				args[7] = new RValue(curTextOutline->pixelsBeforeLineBreak);
				args[8] = new RValue(curTextOutline->textColor);
				args[9] = new RValue(curTextOutline->alpha);
				g_ModuleInterface->CallBuiltin("draw_set_font", { jpFont });
				g_ModuleInterface->CallBuiltin("draw_set_halign", { 1 });
				RValue returnVal;
				origDrawTextOutlineScript(Self, nullptr, returnVal, 10, args);
			}
		}
	}
}