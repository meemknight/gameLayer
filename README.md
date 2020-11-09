
# GameLayer

---

This project offers you all you need to make a game (or other graphical program) in c++.
It comes preinstaled with some libraries that you can remove if you want to.
You can draw to the screen with opengl or draw pixel by pixel

---
 
# Configure:
 	- Unpack dependences.rar
	- Open the solution with visual studio
	- Right click solution -> properties - >Common properties -> Startup Project
	- Select Multiple Startup projects
	- Set gameSetup to none and windowsPlatformLayer to start
	- Also set the gameSetup the first project in this list
	- Then on project dependences select windowsPlatformLayer and make it dependent on gameSetup

---

# How to use it:

All the work that you do should be in the gameSetup project. There you can modift main.cpp and gameStructs.h and add new files if needed.
In main.cpp there are 3 functions of interest: onCreate, onReload and gameLogic. On create is called when the game is created and you can initialize your variables there, 
load textures and set the window dimensions. gameLogic is called per frame and there you write your game logic.

In gameStructs.h there is a struct called GameMemory. There you declare your variables that you want to be persistent after the frame like player position.
Also, in gameStructs.h there is a struct called HeapMemory. You can change the heap memory of the program there. It is not dynamic so you should set
enough memory from the beginning.


The that i have talked about recieve some pointers to some variables that are usefull for your program:
	- deltaTime: measured in seconds, the time passed since last frame
	- allocator: the global allocator pointer (for internal usage)
	- console: the internal console. (more on this later)
	- input: the game input (more on this later)
	- mem: the game memory, here you have things declared in GameMemory structure from the gameStructs.h file
	- heapMemory: (for internal usage)
	- volatileMemory: used for temporary allocations (more on this later)
	- windowBuffer: used for drawing to the screen (more on this later)
	- windowSettings: used to set the window width, height and other things (more on this later)
	- platformFunctions: it has some usefull functions like readEntirFile or writeEntireFile
	
# What not to do:

There are some features that this setup has and it has a few limitations:
	- You should not use global variables (I'll go into detail later)
	- You should never use global variables that allocate memory (This will crash the program)
	* You can instead declare variables that allocate memory in gameStructs.h like a string or a vector
## You should not include iostream because it declares global variable that allocate memory. Use the custom console instead or printf

# Hot code reloading

The best part about this setup is that it supports live code editing. You can recompile the code by pressing ctrl + f5 while the game is running.
Furthermore, you can record the input of the player and play it back while you edit the code. Press alt + r to record,
alt + s to stop and save the recording and alt + p to play the recording. 
Press alt + s to stop the playback. 
* If you modify things in the GameMemory structure you will have to restart your game however.


