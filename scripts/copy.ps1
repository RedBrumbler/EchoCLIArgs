

$root = "${PSScriptRoot}/.."
$build = "${root}/build"
$yodel = "${yodel}/libecho-cli-args.so"

& ${PSScriptRoot}/build.ps1

& adb push ${yodel} /sdcard/ModData/com.readyatdawn.r15/Modloader/early_mods/libecho-cli-args.so
