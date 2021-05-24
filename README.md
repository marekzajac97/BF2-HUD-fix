# BF2 HUD fix
## About
Simple hack that adjusts HUD to the correct aspect ratio and fixes stretching. You can see how it works [here](https://www.youtube.com/watch?v=Mv0N_RxxfDI)

This is a remake of one of the features from an old **RFX.dll** by [BadSanta12345](github.com/BadSanta12345). Although [his source code](https://www.github.com/BadSanta12345/RFX/) is publicly avalible it sadly does not include the fix for stretched HUD. Also, the original RFX.dll from Alpha Project has several issues (crashes in fullscreen mode and when hitting disconnect, restricted cursor movement). I managed to rewrote it using parts of his code and some reverse engineering.

## Download
Head over to [Releases](https://github.com/marekzajac97/BF2-HUD-fix/releases/) for a download link. Works only with BF2 v1.5 patch. To run it you need MS VC++ 2019 Redistributable (x86) installed on your system.

## Instalation
Copy the `hudfix.dll`, `BF2.exe` and `hud_config.txt` to the main Battlefield 2 folder. Copy the contents of the `Menu_server` folder into `Battlefield 2\mods\bf2\Menu_server.zip`

Note: BF2.exe is a regular executable with modified IAT, alternatively you can use an DLL injector.