#include <Windows.h>
#include "loader.h"
#include <sol/sol.hpp>
#include "lua_core.h"

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

// ����һ���ṹ�壬���ڴ洢��Ƶ���ݺ������Ϣ
typedef struct AudioData {
    Mix_Chunk* data; // ��Ƶ����ָ��
    Uint32 size; // ��Ƶ���ݴ�С
    Uint32 index; // ��ǰ����λ��
    int volume; // ������0-128��
    int paused; // �Ƿ���ͣ
} AudioData;

std::map<std::string, AudioData> audioList;

// ���²���λ�õĻص�����
void updateIndex(int channel) {
    // ������Ƶ�б��ҵ���Ӧ����Ƶ
    for (auto& pair : audioList) {
        AudioData* audio = &pair.second;
        // �����Ƶ�Ƿ���ͨ��ƥ��
        if (audio->data == Mix_GetChunk(channel)) {
            // ������Ƶ�Ĳ���λ��
            audio->index += audio->data->alen;
            break;
        }
    }
}

// ������Ƶ
bool loadAudio(std::string name, std::string path) {
    // �����Ƶ�б����Ƿ��Ѵ���ͬ����Ƶ
    if (audioList.count(name) > 0) {
        loader::LOG(loader::ERR) << "��Ƶ " << name << " �Ѵ��ڣ��޷�����" ;
        return true;
    }
    // ������Ƶ�ļ�
    AudioData audio;
    audio.data = Mix_LoadWAV(path.c_str());
    if (audio.data == NULL) {
        loader::LOG(loader::ERR) << "�޷�������Ƶ�ļ� " << path << "��������Ϣ��" << Mix_GetError() ;
        return false;
    }
    // ��ȡ��Ƶ���ݴ�С
    audio.size = audio.data->alen;
    // ��ʼ����������
    audio.index = 0;
    audio.volume = 128;
    audio.paused = 0;
    // ����Ƶ��ӵ���Ƶ�б���
    audioList[name] = audio;
    return true;
}

// ������Ƶ
bool playAudio(std::string name) {
    // �����Ƶ�б����Ƿ���ڸ���Ƶ
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "��Ƶ " << name << " �����ڣ��޷�����" ;
        return false;
    }
    // ��ȡ��Ƶ����
    AudioData* audio = &audioList[name];
    // �����Ƶ�Ƿ��Ѿ��������
    if (audio->index >= audio->size) {
        audio->index = 0;
    }
    // �����Ƶ�Ƿ��Ѿ���ͣ
    if (audio->paused == 1) {
        audio->index = 0;
    }
    // ������Ƶ�Ļص����������ڸ��²���λ�úͼ�ⲥ�Ž���
    Mix_ChannelFinished(updateIndex);
    // ������Ƶ
    int channel = Mix_PlayChannel(-1, audio->data, 0);
    if (channel == -1) {
        loader::LOG(loader::ERR) << "�޷�������Ƶ " << name << "��������Ϣ��" << Mix_GetError() ;
        return false;
    }
    // ������Ƶ������
    Mix_Volume(channel, audio->volume);
    return true;
}

// ��ͣ����
bool pauseAudio(std::string name) {
    // �����Ƶ�б����Ƿ���ڸ���Ƶ
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "��Ƶ " << name << " �����ڣ��޷���ͣ" ;
        return false;
    }
    // ��ȡ��Ƶ����
    AudioData* audio = &audioList[name];
    // �����Ƶ�Ƿ��Ѿ��������
    if (audio->index >= audio->size) {
        loader::LOG(loader::ERR) << "��Ƶ " << name << " �Ѿ�������ϣ��޷���ͣ" ;
        return false;
    }
    // �����Ƶ�Ƿ��Ѿ���ͣ
    if (audio->paused == 1) {
        return true;
    }
    // ��ͣ��Ƶ
    Mix_Pause(-1);
    audio->paused = 1;
    return true;
}

// �ָ���Ƶ����
bool resumeAudio(std::string name) {
    // �����Ƶ�б����Ƿ���ڸ���Ƶ
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "��Ƶ " << name << " �����ڣ��޷��ָ�" ;
        return false;
    }
    // ��ȡ��Ƶ����
    AudioData* audio = &audioList[name];
    // �����Ƶ�Ƿ��Ѿ��������
    if (audio->index >= audio->size) {
        loader::LOG(loader::ERR) << "��Ƶ " << name << " �Ѿ�������ϣ��޷��ָ�" ;
        return false;
    }
    // �����Ƶ�Ƿ��Ѿ���ͣ
    if (audio->paused == 0) {
        return true;
    }
    // �ָ���Ƶ
    Mix_Resume(-1);
    audio->paused = 0;
    return true;
}

// ���Ʋ��Ž���
bool seekAudio(std::string name, Uint32 position) {
    // �����Ƶ�б����Ƿ���ڸ���Ƶ
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "��Ƶ " << name << " �����ڣ��޷����ƽ���" ;
        return false;
    }
    // ��ȡ��Ƶ����
    AudioData* audio = &audioList[name];
    // �����Ƶ�Ƿ��Ѿ��������
    if (audio->index >= audio->size) {
        loader::LOG(loader::ERR) << "��Ƶ " << name << " �Ѿ�������ϣ��޷����ƽ���" ;
        return false;
    }
    // ���λ���Ƿ�Ϸ�
    if (position < 0 || position > audio->size) {
        loader::LOG(loader::ERR) << "λ�� " << position << " ���Ϸ����޷����ƽ���" ;
        return false;
    }
    // ������Ƶ�Ĳ���λ��
    audio->index = position;
    // ���²�����Ƶ
    int channel = Mix_PlayChannel(-1, audio->data, 0);
    if (channel == -1) {
        loader::LOG(loader::ERR) << "�޷�������Ƶ " << name << "��������Ϣ��" << Mix_GetError() ;
        return false;
    }
    // ������Ƶ������
    Mix_Volume(channel, audio->volume);
    // ������Ƶ�Ļص����������ڸ��²���λ�úͼ�ⲥ�Ž���
    Mix_ChannelFinished(updateIndex);
    return true;
}

// ���Ʋ�������
bool setVolume(std::string name, int volume) {
    // �����Ƶ�б����Ƿ���ڸ���Ƶ
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "��Ƶ " << name << " �����ڣ��޷���������" ;
        return false;
    }
    // ��ȡ��Ƶ����
    AudioData* audio = &audioList[name];
    // ��������Ƿ�Ϸ�
    if (volume < 0 || volume > 128) {
        loader::LOG(loader::ERR) << "���� " << volume << " ���Ϸ����޷���������" ;
        return false;
    }
    // ������Ƶ������
    audio->volume = volume;
    Mix_Volume(-1, volume);
    return true;
}

// ��ȡ��ǰ���Ž���
int getCurrentPosition(std::string name) {
    // �����Ƶ�б����Ƿ���ڸ���Ƶ
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "��Ƶ " << name << " �����ڣ��޷���ȡ����" ;
        return -1;
    }
    // ��ȡ��Ƶ����
    AudioData* audio = &audioList[name];
    // ������Ƶ�ĵ�ǰ����λ��
    return audio->index;
}

// ��ȡ��ǰ��Ƶ��С
int getAudioSize(std::string name) {
    // �����Ƶ�б����Ƿ���ڸ���Ƶ
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "��Ƶ " << name << " �����ڣ��޷���ȡ��Ƶ��С";
        return -1;
    }
    // ��ȡ��Ƶ����
    AudioData* audio = &audioList[name];
    // ������Ƶ�ĵ�ǰ����λ��
    return audio->size;
}

// �ͷ���Ƶ��Դ
void freeAudio(std::string name) {
    // �����Ƶ�б����Ƿ���ڸ���Ƶ
    if (audioList.count(name) == 0) {
        return;
    }
    // ��ȡ��Ƶ����
    AudioData* audio = &audioList[name];
    // �ͷ���Ƶ����
    Mix_FreeChunk(audio->data);
    // ����Ƶ�б���ɾ����Ƶ
    audioList.erase(name);
}

DWORD WINAPI AttachThread(LPVOID lParam) {
    SDL_Init(SDL_INIT_EVERYTHING);
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Init(MIX_INIT_OGG);
	while (true) {

		if (LuaCore::reloadTime != 0 && LuaCore::reload != LuaCore::reloadTime) {
			LuaCore::reload = LuaCore::reloadTime;
			LuaCore::init = false;
		}

		if (!LuaCore::init)
		{
			LuaCore::init = true;
			for (std::string file_name : LuaCore::getLuaFiles()) {
				LuaCore::LuaScriptData luae = LuaCore::getLuas()[file_name];
				if (luae.start) {
                    sol::state_view lua(luae.L);
                    lua.set_function("loadAudio", loadAudio);
                    lua.set_function("playAudio", playAudio);
                    lua.set_function("pauseAudio", pauseAudio);
                    lua.set_function("resumeAudio", resumeAudio);
                    lua.set_function("seekAudio", seekAudio);
                    lua.set_function("setVolume", setVolume);
                    lua.set_function("getCurrentPosition", getCurrentPosition);
                    lua.set_function("getAudioSize", getAudioSize);
                    lua.set_function("deleteAudio", freeAudio);
				}
			}
		}
		Sleep(100);
	}

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        CreateThread(nullptr, 0, &AttachThread, static_cast<LPVOID>(hModule), 0, nullptr);
        break;
    }
    }
    return TRUE;
}

