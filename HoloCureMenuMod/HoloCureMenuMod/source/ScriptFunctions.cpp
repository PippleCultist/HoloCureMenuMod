#include "ModuleMain.h"
#include "ScriptFunctions.h"
#include "HoloCureMenuInterface.h"
#include "CommonFunctions.h"
#include "CodeEvents.h"

extern std::shared_ptr<menuGridData> curMenuGrid;
extern CallbackManagerInterface* callbackManagerInterfacePtr;

RValue& ConfirmedTitleScreenBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	if (curMenuGrid == nullptr)
	{
		int currentOption = static_cast<int>(lround(getInstanceVariable(Self, GML_currentOption).ToDouble()));
		if (currentOption == 3)
		{
			setInstanceVariable(Self, GML_canControl, RValue(false));
			loadModMenu();
			callbackManagerInterfacePtr->CancelOriginalFunction();
		}
	}
	return ReturnValue;
}