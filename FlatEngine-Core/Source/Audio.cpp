#include "Audio.h"
#include "GameObject.h"
#include "FlatEngine.h"


namespace FlatEngine
{
	Audio::Audio(long myID, long parentID)
	{
		SetType(T_Audio);
		SetID(myID);
		SetParentID(parentID);
		m_sounds = std::vector<SoundData>();
	}

	Audio::~Audio()
	{
	}

	std::string Audio::GetData()
	{
		json soundData = json::array();

		for (SoundData sound : m_sounds)
		{
			json soundJson = {
				{ "path", sound.path },
				{ "name", sound.name },
				{ "b_isMusic", sound.b_isMusic }
			};
			soundData.push_back(soundJson);
		}

		json jsonData = {
			{ "type", "Audio" },
			{ "id", GetID() },
			{ "_isCollapsed", IsCollapsed() },
			{ "_isActive", IsActive() },
			{ "soundData", soundData }
		};

		std::string data = jsonData.dump();
		// Return dumped json object with required data for saving
		return data;
	}

	void Audio::SetPath(std::string soundName, std::string newPath)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				sound.path = newPath;
				LoadAudio(sound);
			}
		}
	}

	std::vector<SoundData> &Audio::GetSounds()
	{
		return m_sounds;
	}

	std::string Audio::GetPath(std::string soundName)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				return sound.path;
			}
		}
		return "";
	}

	bool Audio::ContainsName(std::string soundName)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				return true;
			}
		}
		return false;
	}

	void Audio::SetIsMusic(std::string soundName, bool b_isMusic)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				sound.b_isMusic = b_isMusic;
				LoadAudio(sound);
				break;
			}
		}
	}

	void Audio::LoadAudio(SoundData &soundData)
	{
		if (soundData.path != "" && soundData.sound != nullptr)
		{
			if (FlatEngine::FilepathHasExtension(soundData.path, ".mp3"))
			{
				soundData.sound->LoadMusic(soundData.path);
				soundData.b_isMusic = true;
			}
			else
			{
				soundData.sound->LoadEffect(soundData.path);
				soundData.b_isMusic = false;
			}
		}
	}

	bool Audio::IsMusic(std::string soundName)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				return sound.b_isMusic;
			}
		}
		return false;
	}

	void Audio::AddSound(std::string soundName, std::string soundPath)
	{
		SoundData soundData;
		soundData.name = soundName;
		soundData.path = soundPath;
		soundData.sound = std::make_shared<Sound>();

		for (std::vector<SoundData>::iterator iter = m_sounds.begin(); iter != m_sounds.end(); iter++)
		{
			if (iter->name == soundName)
			{
				m_sounds.erase(iter);
				break;
			}
		}

		LoadAudio(soundData);
		m_sounds.push_back(soundData);
	}

	void Audio::SetSounds(std::vector<SoundData> sounds)
	{
		m_sounds = sounds;
	}

	void Audio::RemoveSound(std::string soundName)
	{
		for (std::vector<SoundData>::iterator iter = m_sounds.begin(); iter != m_sounds.end(); iter++)
		{
			if (iter->name == soundName)
			{
				m_sounds.erase(iter);
				break;
			}
		}
	}

	void Audio::PlaySound(std::string soundName)
	{
		bool b_audioFound = false;
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				Play(sound.name);
				b_audioFound = true;
			}	
		}
		if (!b_audioFound)
		{
			LogError("Audio sound " + soundName + " not found.");
		}
	}

	void Audio::PauseSound(std::string soundName)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				Pause(sound.name);
			}
		}
	}

	bool Audio::IsPaused(std::string soundName)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				return sound.sound->IsPaused();
			}
		}
		return false;
	}

	void Audio::StopSound(std::string soundName)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				Stop(sound.name);
			}
		}
	}

	void Audio::StopAll()
	{
		for (SoundData sound : m_sounds)
		{
			Stop(sound.name);
		}
	}

	void Audio::Play(std::string soundName, int channel)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				if (sound.b_isMusic)
				{
					sound.sound->PlayMusic();
				}
				else
				{
					sound.sound->PlayEffect(channel);
				}
			}
		}
	}

	void Audio::Pause(std::string soundName, int channel)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				sound.sound->PauseMusic();
				sound.sound->HaultChannel(channel);
			}
		}
	}

	void Audio::Stop(std::string soundName, int channel)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				sound.sound->StopMusic();
				sound.sound->HaultChannel(channel);
			}
		}
	}

	bool Audio::IsMusicPlaying(std::string soundName)
	{
		for (SoundData sound : m_sounds)
		{
			if (sound.name == soundName)
			{
				return sound.sound->IsMusicPlaying();
			}
		}
		return false;
	}
}