#pragma once
#include <string>
#include <raudio.h>
#include <vector>


struct PlayingAudioMusic
{
	Music m;
	bool keepPlaying;
	std::string name;
};

struct Audio
{

	Audio();

	void initAudioDrivers();

	void setMasterVolume(float volume);
	void playSound(const char* name, float volume);
	void keepPlayingMusic(const char* name, float volume);
	void updateAudioStream();

	const int MAX_SOUNDS = 10;
	const int MAX_MUSICS = 4;

	std::vector< Sound > sounds;
	std::vector< PlayingAudioMusic > musics;

};

