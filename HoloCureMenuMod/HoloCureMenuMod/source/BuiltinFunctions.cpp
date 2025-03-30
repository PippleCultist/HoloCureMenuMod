#include "BuiltinFunctions.h"
#include "ModuleMain.h"
#include "CallbackManager/CallbackManagerInterface.h"

extern CallbackManagerInterface* callbackManagerInterfacePtr;

void DrawTextBefore(RValue& Result, CInstance* Self, CInstance* Other, int numArgs, RValue* Args)
{
	if (numArgs >= 3)
	{
		// The only draw_text at 320, 20 is the gamemode, so just assume that will be the case for now
		if (lround(Args[0].m_Real) == 320 && lround(Args[1].m_Real) == 20)
		{
			Args[2] = std::string_view("MODDED " + Args[2].ToString());
		}
	}
}

void ShowDebugMessageBefore(RValue& Result, CInstance* Self, CInstance* Other, int numArgs, RValue* Args)
{
	callbackManagerInterfacePtr->CancelOriginalFunction();
}