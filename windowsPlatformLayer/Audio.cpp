#include "Audio.h"
#include "windowsFunctions.h"
#undef PlaySound

extern PlatformFunctions platformFunctions;

Audio::Audio()
{
	musics.reserve(MAX_MUSICS);
	sounds.reserve(MAX_SOUNDS);
	
}

void Audio::initAudioDrivers()
{
	InitAudioDevice();
}

void Audio::setMasterVolume(float volume)
{

	SetMasterVolume(volume);

}

void Audio::playSound(const char* name, float volume)
{

	if(sounds.size() < MAX_SOUNDS)
	{
		Sound s = LoadSound(name);
		PlaySound(s);
		SetSoundVolume(s, volume);

		if(IsSoundPlaying(s))
		{

			sounds.push_back(s);
			auto& m = sounds[sounds.size() - 1];

		}else
		{
			std::string errMessage = "Error loading Sound: ";
			errMessage += name;

			platformFunctions.console.elog(errMessage.c_str());

		}


	}

}

void Audio::keepPlayingMusic(const char* name, float volume)
{

	int found = 0;

	for (auto &i: musics)
	{
		if(i.name == name)
		{
			i.keepPlaying = true;
			SetMusicVolume(i.m, volume);
			found = true;
			break;
		}
	
	}

	if(!found)
	{
		PlayingAudioMusic m;
		m.name = name;
		
		m.m = LoadMusicStream(name);
		PlayMusicStream(m.m);
		SetMusicVolume(m.m, volume);

		if(IsMusicPlaying(m.m))
		{
						
			musics.push_back(m);

		}

	}

}

void Audio::updateAudioStream()
{

	for(int i=0; i<sounds.size(); i++)
	{
		if(!IsSoundPlaying(sounds[i]))
		{
			UnloadSound(sounds[i]);
		}

		sounds.erase(sounds.begin() + i);
		i--;
	}


	for(auto &i: musics)
	{
		if(i.keepPlaying)
		{
			i.keepPlaying = 0;
			
			if(!IsMusicPlaying(i.m))
			{
				PlayMusicStream(i.m);
			}

			UpdateMusicStream(i.m);

		}else
		{
			if (IsMusicPlaying(i.m))
			{
				StopMusicStream(i.m);
			}
		
		}
	
	}

}
