# BF2 HUD fix
## About
Simple hack that adjusts HUD to the correct aspect ratio and therefore fixes stretching. You can see how it works [here](https://www.youtube.com/watch?v=Mv0N_RxxfDI)

This is a partial remake of **RFX.dll** by [BadSanta12345](github.com/BadSanta12345) from Alpha Project mod. I decided to recreate it as original one had several issues namely:
- crashed the game on startup in fullscreen mode (Windows 10)
- caused pure virtual function call exception when hitting "disconnect"
- cursor movement was restricted to the screen center, which dictated the position of things like the spawn menu and scoreboard

Although he shared [his source code](https://www.github.com/BadSanta12345/RFX/) he did not include the fix for stretched HUD in there. I rewrote it using parts of his original code and some reverse engineering fixing all above issues.

## Usage
Works only for BF2 v1.5 patch. To use this you need a DLL injector or modification of BF2.exe's IAT with CFF Explorer (just use Google).

## Node offsetting
If you appply this fix your entire HUD will be squished into a 4:3 rectangle in the screen center, but I guess you'd like to have some stuff like minimap, chat etc moved to the sides, right? To do this:

1. Open `Battlefield 2\mods\bf2\Menu_server.zip\HUD\HudSetupMain.con` with a text editor.
2. Add `run ApplyOffsets.con` line on top and save it.
3. Put `ApplyOffsets.con` (template can be found [here](https://pastebin.com/FujRND7L)) file inside your `Menu_server.zip\HUD`.
4. Edit `ApplyOffsets.con` and add nodes to the list.
## Credits
- [BadSanta12345](https://www.github.com/BadSanta12345/)
