# jc3-handling-editor
A in-game realtime handling editor, also supports auto load of saved profiles.

## Dependencies
* Visual Studio 2015
* Python 2.7
* Just Cause 3 Mod Loader (or other method of injecting DLL at startup)

## Installation
* Clone this repo
* Run `python configure.py`
* Run `python build.py`
* Either copy or symlink the resulting DLL (in `out\Release`) to your `Just Cause 3\mods` directory
	* If Just Cause 3 Mod Loader is not already installed, acquire a copy (should be named `dinput8.dll`) and copy it into your `Just Cause 3` directory
* Launch game and toggle menu with F8