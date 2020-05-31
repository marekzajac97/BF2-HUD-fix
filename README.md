# BF2 HUD fix
Simple hack that adjusts HUD to the correct aspect ratio and therefore fixes stretching. You can see how it works [here](https://www.youtube.com/watch?v=Mv0N_RxxfDI)

This is a partial remake of **RFX.dll** by BadSanta12345 from Alpha Project mod. I decided to recreate it as original one had several issues namely:
- crashed the game on startup in fullscreen mode (Windows 10)
- caused pure virtual function call exception when hitting "disconnect"
- cursor movement was restricted to the screen center, which dictated the position of things like the spawn menu and scoreboard

Although he shared [his source code](github.com/BadSanta12345/RFX/) he did not include the fix for stretched HUD in there. I rewrote it using parts of his original code and some reverse engineering, which solved the first 2 issues. Also figured out the way extended the mouse area fixing the 3rd problem.

Works only for BF2 v1.5 patch. To use this you need a DLL injector or modification of BF2.exe's IAT with CFF Explorer (just use Google). All credits go to [BadSanta12345](github.com/BadSanta12345)
