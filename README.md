﻿﻿# ASongPlayer

## 介绍
音视频播放器

## 环境
- 操作系统: win10 64位
- QT版本: QT6.0.24
- 编译器: MSVC 2019 64bit

## 软件架构
### 前端
- QT6.0.24
### 后端
- 解码：FFmpeg4.4
- 视频渲染：SDL2.0
- 音频播放：QAudioSink+QIODevice

## 功能
- 支持基本的音视频播放/暂停/停止
- 支持音量控制、静音、解除静音
- 支持单次播放、单个循环、列表循环和随机播放四种播放模式
- 支持进度条跳转
- 支持媒体库播放列表
- 支持查看媒体文件详情信息
- 支持下一帧/上一帧和下五帧/上五帧的进度微调
- 支持0.5x、2.0x、4.0x和8.0x倍速播放
- 支持一倍速倒放（仅限视频）
- 支持进度条预览
