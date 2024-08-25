# HoloCure Menu Mod
This mod provides an interface where other mods can register menu elements for HoloCure.
## Usage Steps
- Download `HoloCureMenuMod.dll` and `HoloCureMenuInterface.h` from the latest version of the mod https://github.com/PippleCultist/HoloCureMenuMod/releases
  - Put `HoloCureMenuMod.dll` in the `mods/Aurie` directory of the target game
  - To create a mod that uses the HoloCure Menu mod, include `HoloCureMenuInterface.h` in the project and use the HoloCureMenuInterface
    - To get the HoloCureMenuInterface in your mod, use `ObGetInterface("HoloCureMenuInterface", (AurieInterfaceBase*&)holoCureMenuInterfacePtr);` in ModuleInitialize
    - Note: The header only provides a declaration of the HoloCureMenuInterface struct and the available functions. Do not attempt to define any of the functions. The function definition will be found at runtime from `HoloCureMenuMod.dll`.
