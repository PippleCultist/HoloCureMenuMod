#pragma once
#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>
using namespace Aurie;
using namespace YYTK;

#define VERSION_NUM "v1.1.0"
#define MODNAME "Holocure Menu Mod " VERSION_NUM

#define SOME_ENUM(DO) \
	DO(idletime) \
	DO(canControl) \
	DO(actionOnePressed) \
	DO(actionTwoPressed) \
	DO(enterPressed) \
	DO(escPressed) \
	DO(moveUpPressed) \
	DO(moveDownPressed) \
	DO(moveLeftPressed) \
	DO(moveRightPressed) \
	DO(currentOption) \

#define MAKE_ENUM(VAR) GML_ ## VAR,
enum VariableNames
{
	SOME_ENUM(MAKE_ENUM)
};

#define MAKE_STRINGS(VAR) #VAR,
const char* const VariableNamesStringsArr[] =
{
	SOME_ENUM(MAKE_STRINGS)
};

extern RValue GMLVarIndexMapGMLHash[1001];
extern CInstance* globalInstance;
extern YYTKInterface* g_ModuleInterface;
extern YYRunnerInterface g_RunnerInterface;

extern TRoutine origStructGetFromHashFunc;
extern TRoutine origStructSetFromHashFunc;
extern PFUNC_YYGMLScript origDrawTextOutlineScript;

extern int objInputManagerIndex;
extern int objTitleScreenIndex;
extern int sprHudInitButtonsIndex;
extern int sprHudScrollArrows2;
extern int sprHudOptionButton;
extern int jpFont;
extern int rmCharSelect;