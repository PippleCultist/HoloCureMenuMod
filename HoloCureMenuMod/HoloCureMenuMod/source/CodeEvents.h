#pragma once
#include "ModuleMain.h"
#include "Aurie/shared.hpp"
#include "YYToolkit/shared.hpp"

using namespace Aurie;
using namespace YYTK;

void loadModMenu();

void InputManagerStepAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void TitleScreenDrawBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void TextControllerCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void TitleCharacterDrawBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);