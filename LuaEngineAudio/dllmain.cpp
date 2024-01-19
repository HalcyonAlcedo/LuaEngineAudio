#include <Windows.h>
#include "loader.h"
#include <sol/sol.hpp>
#include "lua_core.h"

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

// 定义一个结构体，用于存储音频数据和相关信息
typedef struct AudioData {
    Mix_Chunk* data; // 音频数据指针
    Uint32 size; // 音频数据大小
    Uint32 index; // 当前播放位置
    int volume; // 音量（0-128）
    int paused; // 是否暂停
} AudioData;

std::map<std::string, AudioData> audioList;

// 更新播放位置的回调函数
void updateIndex(int channel) {
    // 遍历音频列表，找到对应的音频
    for (auto& pair : audioList) {
        AudioData* audio = &pair.second;
        // 检查音频是否与通道匹配
        if (audio->data == Mix_GetChunk(channel)) {
            // 更新音频的播放位置
            audio->index += audio->data->alen;
            break;
        }
    }
}

// 加载音频
bool loadAudio(std::string name, std::string path) {
    // 检查音频列表中是否已存在同名音频
    if (audioList.count(name) > 0) {
        loader::LOG(loader::ERR) << "音频 " << name << " 已存在，无法加载" ;
        return true;
    }
    // 加载音频文件
    AudioData audio;
    audio.data = Mix_LoadWAV(path.c_str());
    if (audio.data == NULL) {
        loader::LOG(loader::ERR) << "无法加载音频文件 " << path << "，错误信息：" << Mix_GetError() ;
        return false;
    }
    // 获取音频数据大小
    audio.size = audio.data->alen;
    // 初始化其他属性
    audio.index = 0;
    audio.volume = 128;
    audio.paused = 0;
    // 将音频添加到音频列表中
    audioList[name] = audio;
    return true;
}

// 播放音频
bool playAudio(std::string name) {
    // 检查音频列表中是否存在该音频
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "音频 " << name << " 不存在，无法播放" ;
        return false;
    }
    // 获取音频数据
    AudioData* audio = &audioList[name];
    // 检查音频是否已经播放完毕
    if (audio->index >= audio->size) {
        audio->index = 0;
    }
    // 检查音频是否已经暂停
    if (audio->paused == 1) {
        audio->index = 0;
    }
    // 设置音频的回调函数，用于更新播放位置和检测播放结束
    Mix_ChannelFinished(updateIndex);
    // 播放音频
    int channel = Mix_PlayChannel(-1, audio->data, 0);
    if (channel == -1) {
        loader::LOG(loader::ERR) << "无法播放音频 " << name << "，错误信息：" << Mix_GetError() ;
        return false;
    }
    // 设置音频的音量
    Mix_Volume(channel, audio->volume);
    return true;
}

// 暂停播放
bool pauseAudio(std::string name) {
    // 检查音频列表中是否存在该音频
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "音频 " << name << " 不存在，无法暂停" ;
        return false;
    }
    // 获取音频数据
    AudioData* audio = &audioList[name];
    // 检查音频是否已经播放完毕
    if (audio->index >= audio->size) {
        loader::LOG(loader::ERR) << "音频 " << name << " 已经播放完毕，无法暂停" ;
        return false;
    }
    // 检查音频是否已经暂停
    if (audio->paused == 1) {
        return true;
    }
    // 暂停音频
    Mix_Pause(-1);
    audio->paused = 1;
    return true;
}

// 恢复音频播放
bool resumeAudio(std::string name) {
    // 检查音频列表中是否存在该音频
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "音频 " << name << " 不存在，无法恢复" ;
        return false;
    }
    // 获取音频数据
    AudioData* audio = &audioList[name];
    // 检查音频是否已经播放完毕
    if (audio->index >= audio->size) {
        loader::LOG(loader::ERR) << "音频 " << name << " 已经播放完毕，无法恢复" ;
        return false;
    }
    // 检查音频是否已经暂停
    if (audio->paused == 0) {
        return true;
    }
    // 恢复音频
    Mix_Resume(-1);
    audio->paused = 0;
    return true;
}

// 控制播放进度
bool seekAudio(std::string name, Uint32 position) {
    // 检查音频列表中是否存在该音频
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "音频 " << name << " 不存在，无法控制进度" ;
        return false;
    }
    // 获取音频数据
    AudioData* audio = &audioList[name];
    // 检查音频是否已经播放完毕
    if (audio->index >= audio->size) {
        loader::LOG(loader::ERR) << "音频 " << name << " 已经播放完毕，无法控制进度" ;
        return false;
    }
    // 检查位置是否合法
    if (position < 0 || position > audio->size) {
        loader::LOG(loader::ERR) << "位置 " << position << " 不合法，无法控制进度" ;
        return false;
    }
    // 设置音频的播放位置
    audio->index = position;
    // 重新播放音频
    int channel = Mix_PlayChannel(-1, audio->data, 0);
    if (channel == -1) {
        loader::LOG(loader::ERR) << "无法播放音频 " << name << "，错误信息：" << Mix_GetError() ;
        return false;
    }
    // 设置音频的音量
    Mix_Volume(channel, audio->volume);
    // 设置音频的回调函数，用于更新播放位置和检测播放结束
    Mix_ChannelFinished(updateIndex);
    return true;
}

// 控制播放音量
bool setVolume(std::string name, int volume) {
    // 检查音频列表中是否存在该音频
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "音频 " << name << " 不存在，无法控制音量" ;
        return false;
    }
    // 获取音频数据
    AudioData* audio = &audioList[name];
    // 检查音量是否合法
    if (volume < 0 || volume > 128) {
        loader::LOG(loader::ERR) << "音量 " << volume << " 不合法，无法控制音量" ;
        return false;
    }
    // 设置音频的音量
    audio->volume = volume;
    Mix_Volume(-1, volume);
    return true;
}

// 获取当前播放进度
int getCurrentPosition(std::string name) {
    // 检查音频列表中是否存在该音频
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "音频 " << name << " 不存在，无法获取进度" ;
        return -1;
    }
    // 获取音频数据
    AudioData* audio = &audioList[name];
    // 返回音频的当前播放位置
    return audio->index;
}

// 获取当前音频大小
int getAudioSize(std::string name) {
    // 检查音频列表中是否存在该音频
    if (audioList.count(name) == 0) {
        loader::LOG(loader::ERR) << "音频 " << name << " 不存在，无法获取音频大小";
        return -1;
    }
    // 获取音频数据
    AudioData* audio = &audioList[name];
    // 返回音频的当前播放位置
    return audio->size;
}

// 释放音频资源
void freeAudio(std::string name) {
    // 检查音频列表中是否存在该音频
    if (audioList.count(name) == 0) {
        return;
    }
    // 获取音频数据
    AudioData* audio = &audioList[name];
    // 释放音频数据
    Mix_FreeChunk(audio->data);
    // 从音频列表中删除音频
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

