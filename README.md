# LuaEngineAudio
LuaEngine的音频组件，可实现游戏内播放音频的功能

## 功能列表
- loadAudio
- playAudio
- pauseAudio
- resumeAudio
- seekAudio
- setVolume
- getCurrentPosition
- deleteAudio

## 示例
  loadAudio('test','nativePC/test.ogg')
  playAudio('test')
  setVolume('test', 128)

## 说明
loadAudio可加载ogg和wav格式的音频文件

setVolume取值范围是0-128
