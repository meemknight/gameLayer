
#GameLayer

---

This project offers you all you need to make a game (or other graphical program) in c++.
It comes preinstaled with some libraries that you can remove if you want to.

---

Configure:
	Unpack dependences.rar
	Open the solution with visual studio
	Right click solution -> properties - >Common properties -> Startup Project
	Select Multiple Startup projects
	Set gameSetup to none and windowsPlatformLayer to start
	Also set the gameSetup the first project in this list
	Then on project dependences select windowsPlatformLayer and make it dependent on gameSetup
