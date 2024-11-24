Drop the contents of your entire Steamworks SDK in this directory.
In the same directory as the game/editor executable (or in your working directory if running from Visual Studio) you need to add two files:
 - make a steam_appid.txt file which simply contains the number 480 (the developer steam id).
 - copy over the "Octave\External\Steam\redistributable_bin\win64\steam_api64.dll" file.

With both of those files in your working directory location, you will hopefully be able to connect to steam when launching the game or editor.