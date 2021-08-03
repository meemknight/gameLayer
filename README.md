
# GameLayer

---

This project offers you all you need to make a game (or other graphical program) in c++.
It comes preinstaled with some libraries that you can remove if you want to.
You can draw to the screen with opengl or draw pixel by pixel and you can play music or sounds very easily.

---
 
# Configure:

 	1. Unpack dependences.rar
	2. Open the solution with visual studio
	3. Right click solution -> properties - >Common properties -> Startup Project
	4. Select Multiple Startup projects
	5. Set gameSetup to none and windowsPlatformLayer to start
	6. Also set the gameSetup the first project in this list
	7. Then on project dependences select windowsPlatformLayer and make it dependent on gameSetup

---

# How to use it:

All the work that you do should be in the gameSetup project. There you can write your code in main.cpp. Don't remove gameStructs freeListAllocator and utility. You can and add new files if needed.
In main.cpp there are 3 functions of interest: onCreate, onReload and gameLogic. On create is called when the game is created and you can initialize your variables there, 
load textures and set the window dimensions. gameLogic is called per frame and there you write your game logic.

In gameStructs.h there is a struct called GameMemory. There you declare your variables that you want to be persistent after the frame has ended like player position.
Also, in gameStructs.h there is a struct called HeapMemory. You can change the heap memory of the program there. It is not dynamic so you should set
enough memory from the beginning.


The functions that I have talked about recieve some pointers to some variables that are usefull for your program:
	* deltaTime: measured in seconds, the time passed since last frame
	* allocator: the global allocator pointer (for internal usage, it overwrited the global allocator)
	* console: the internal console. (more on this later)
	* input: the game input (more on this later)
	* mem: the game memory, here you have things declared in GameMemory structure from the gameStructs.h file
	* heapMemory: (for internal usage, yes it is just your heap memory)
	* volatileMemory: used for temporary allocations (more on this later)
	* windowBuffer: used for drawing to the screen (more on this later)
	* windowSettings: used to set the window width, height and other things (more on this later)
	* platformFunctions: it has some usefull functions like readEntirFile or writeEntireFile
	
# What not to do:

There are some features that this setup has and it has a few limitations:
	- You should not use global variables (I'll go into detail later)
	- You should never use global variables that allocate memory (This will crash the program)
	* You can instead declare variables that allocate memory in gameStructs.h like a string or a vector
	- Don't use malloc, use new / delete
## You should not include iostream because it declares global variable that allocate memory. Use the custom console instead or printf
	
---

# Hot code reloading

The best part about this setup is that it supports live code editing. You can recompile the code by pressing ctrl + f5 while the game is running.
Furthermore, you can record the input of the player and play it back while you edit the code. Press alt + r to record,
alt + s to stop and save the recording and alt + p to play the recording. 
Press alt + s to stop the playback. 
* If you modify things in the GameMemory structure you will have to restart your game however.

---

# Internal console

Press alt + ~ to bring up the console. You can write to it using:

* console.log() gray logs
* console.elog() red error logs
* console.wlog() yellow warning logs
* console.glog() green logs
* console.blog() blue logs

In the console you can modify "serialized" variabels. 
You can declare some basic types as serialized variabels: int float bool char.
For example: in GameMemory you cand type SERIALIZE(int, test, 10); which is equal to int test = 10;
In the console you can type var test = 12 to modify it or type vars to display all the variables.

# input

* input->keyBoard[Button::Space].held -> space is held
* input->keyBoard[Button::D].released -> d letter is released
* input->leftMouse.pressed -> left mouse button is pressed
* input->mouseX -> mouse position on the x axe
* input->windowActive -> true if the window is active
* input->anyController.X.pressed -> true if x is pressed on any controller
* input->controllers[0].LThumb.x -> for controller 0, x axes of the left thumb, there are 4 controllers in total
* input->keyBoard[Button::A].typed -> symulates typing letters. It will be true when pressed and then after a small pause few times a seccond like helding a key down when typing.
* input->windowActive -> true if the window is active. 
* input->typedCharacters -> a null terminated string with the typed characters. Good for input boxes. The 8 aschii value means back space. Will take into account if shift was pressed to convert letters to upper or numbers to symbols etc. 

# volatileMemory

Used for per frame allocations. Faster than using new. You don't have to free the memory, it gets freed at the end of the function call(frame).

* volatileMemory->allocate(10) -> allocates 10 bytes of memory
* volatileMemory->allocateAndClear(KB(1)) -> allocate one KB of memory and cleares it to 0

There is a maximum size of volatile memory like there is a maximum size of heap memory. You can change it in the VolatileMemory structure in gameStructs.h

# windowBuffer

Used for drawing with pixels.

* windowBuffer->clear(); // clears the screen
* windowBuffer->drawAt(); puts pixel
* windowBuffer->w //width of the screen
* windowBuffer->h //height of the screen

## ! You can't draw both with opengl and pixel by pixel

# windowSettings

Used for changing the screen size and more.

* windowSettings->drawWithOpenGl = true; // enables opengl or pixel by pixel mode described earlier
* windowSettings->w = 600; // if you do this every frame wou can lock the window width to 600 pixels for example
* windowSettings->vsyncWithOpengl = true; //set vsync (when drawing with opengl)
* windowSettings->lockFpsIfNotVsync = 60; //set to 0 to turn off, set the framerate when vsync is off or not working (note there is a maximum limit at around 300 fps that will always stay on)
* windowSettings->force_16_9_AspectRatioOnFullScreen = true; //when in full screen it will set the rezolution of the monutor to the highest rezolution that has a 16:9 aspect ratio
* windowSettings->windowTitle ->a null terminated string with the title of the window. If left empty the window will have the fps in the title.

# music and sounds

* platformFunctions->playSound("resources/weird.wav", 0.1); // will play a sound at 0.1 volume.
* platformFunctions->keepPlayingMusic("resources/jungle.wav", 0.08); // will play music at 0.08 volume while this is called every frame. If you want to stop the music stop calling the function.
* platformFunctions->setMasterVolume(1.f); // sets the master volume to max

# Opengl

I have my own [2d library](https://github.com/meemknight/gl2d) that you can use to draw to the screen if you want. glew is also installed and initialized properly.

# Shipping

In windowsPlatformLayer project there is a file buildConfig.h. You might want to change flags there when shipping your program. There are all explained there.
Your game is the exe and the dll and the folder with resources that you will have to manually copy in the release folder if you choose to use one. Other visual studio files are not used.

