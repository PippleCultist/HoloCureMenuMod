#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>
#include "ModuleMain.h"
#include "CallbackManager/CallbackManagerInterface.h"
#include "HoloCureMenuInterface.h"
#include "CodeEvents.h"
#include "ScriptFunctions.h"
#include "BuiltinFunctions.h"
using namespace Aurie;
using namespace YYTK;

extern std::vector<std::string> modMenuGridNames;
extern std::vector<std::shared_ptr<menuGridData>> modMenuGridList;

RValue GMLVarIndexMapGMLHash[1001];

TRoutine origStructGetFromHashFunc;
TRoutine origStructSetFromHashFunc;

CallbackManagerInterface* callbackManagerInterfacePtr = nullptr;
YYTKInterface* g_ModuleInterface = nullptr;
YYRunnerInterface g_RunnerInterface;

PFUNC_YYGMLScript origDrawTextOutlineScript = nullptr;

HoloCureMenuInterface holoCureMenuInterface;

CInstance* globalInstance = nullptr;

int objInputManagerIndex = -1;
int objTitleScreenIndex = -1;
int sprHudInitButtonsIndex = -1;
int sprHudScrollArrows2 = -1;
int sprHudOptionButton = -1;
int jpFont = -1;
int rmCharSelect = -1;

AurieStatus moduleInitStatus = AURIE_MODULE_INITIALIZATION_FAILED;

void initHooks()
{
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "struct_get_from_hash", nullptr, nullptr, &origStructGetFromHashFunc)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "struct_get_from_hash");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "struct_set_from_hash", nullptr, nullptr, &origStructSetFromHashFunc)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "struct_set_from_hash");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "draw_text", DrawTextBefore, nullptr, nullptr)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "draw_text");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "show_debug_message", ShowDebugMessageBefore, nullptr, nullptr)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "show_debug_message");
		return;
	}

	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_InputManager_Step_0", InputManagerStepAfter, nullptr)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_InputManager_Step_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_TitleScreen_Draw_0", TitleScreenDrawBefore, nullptr)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_TitleScreen_Draw_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_TitleCharacter_Draw_0", TitleCharacterDrawBefore, nullptr)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_TitleCharacter_Draw_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_TextController_Create_0", nullptr, TextControllerCreateAfter)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_TextController_Create_0");
		return;
	}

	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_Confirmed@gml_Object_obj_TitleScreen_Create_0", ConfirmedTitleScreenBefore, nullptr, nullptr)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Script_Confirmed@gml_Object_obj_TitleScreen_Create_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_draw_text_outline", nullptr, nullptr, &origDrawTextOutlineScript)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Script_draw_text_outline");
		return;
	}

	g_RunnerInterface = g_ModuleInterface->GetRunnerInterface();
	g_ModuleInterface->GetGlobalInstance(&globalInstance);

	objTitleScreenIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "obj_TitleScreen" }).ToDouble());
	objInputManagerIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "obj_InputManager" }).ToDouble());
	sprHudInitButtonsIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "hud_initButtons" }).ToDouble());
	sprHudScrollArrows2 = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "hud_scrollArrows2" }).ToDouble());
	sprHudOptionButton = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "hud_OptionButton" }).ToDouble());
	jpFont = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "jpFont" }).ToDouble());
	rmCharSelect = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "rm_CharSelect" }).ToDouble());

	AurieStatus status = AURIE_SUCCESS;

	for (int i = 0; i < std::extent<decltype(VariableNamesStringsArr)>::value; i++)
	{
		if (!AurieSuccess(status))
		{
			DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to get hash for %s", VariableNamesStringsArr[i]);
		}
		GMLVarIndexMapGMLHash[i] = std::move(g_ModuleInterface->CallBuiltin("variable_get_hash", { VariableNamesStringsArr[i] }));
	}

	callbackManagerInterfacePtr->LogToFile(MODNAME, "Finished initializing");

	moduleInitStatus = AURIE_SUCCESS;
}

void runnerInitCallback(FunctionWrapper<void(int)>& dummyWrapper)
{
	AurieStatus status = AURIE_SUCCESS;
	status = ObGetInterface("callbackManager", (AurieInterfaceBase*&)callbackManagerInterfacePtr);
	if (!AurieSuccess(status))
	{
		printf("Failed to get callback manager interface. Make sure that CallbackManagerMod is located in the mods/Aurie directory.\n");
		return;
	}

	callbackManagerInterfacePtr->RegisterInitFunction(initHooks);
}

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	modMenuGridNames.push_back("Play");
	modMenuGridList.push_back(std::shared_ptr<menuGridData>(nullptr));
	ObCreateInterface(Module, &holoCureMenuInterface, "HoloCureMenuInterface");

	// Gets a handle to the interface exposed by YYTK
	// You can keep this pointer for future use, as it will not change unless YYTK is unloaded.
	g_ModuleInterface = GetInterface();

	// If we can't get the interface, we fail loading.
	if (g_ModuleInterface == nullptr)
	{
		DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to get YYTK interface");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	g_ModuleInterface->CreateCallback(
		Module,
		EVENT_RUNNER_INIT,
		runnerInitCallback,
		0
	);
	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	return moduleInitStatus;
}