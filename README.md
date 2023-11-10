# Echo CLI Args

A mod for EchoVR on quest to support reading in command line arguments, since the game normally does not support these.

# Building

To build, a setup for quest modding using qpm, ndk, ninja and cmake is required.
Then when in the root directory of the repo you can run `./scripts/build.ps1` and the mod will build.

# Installation

Install the [yodel](https://github.com/RedBrumbler/Yodel) modloader and drop the .so file for this mod into `/sdcard/ModData/com.readyatdawn.r15/Modloader/early_mods/`
Then your command line args you wish to use should be placed at `/sdcard/ModData/com.readyatdawn.r15/Mods/echo-cli-args/args.txt` from where they will be read and used within the game

# Args

Known args to be useful for the game can be found on the [echopedia](https://echopedia.gg/wiki/Echo_VR_Command_Line_Arguments)

# Resources
 - Easier time patching echovr using [EchoPatcher](https://github.com/Lauriethefish/EchoPatcher) by @Lauriethefish
