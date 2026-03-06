
.
├── common //整个文件夹从mpp端复制过来
├── include
│   ├── sample_smartIPC_demo_config_ty.h  //可用对比文件更新
│   └── sample_smartIPC_demo_ty.h  //可用对比文件更新
├── README.md
└── src
    ├── sample_smartIPC_demo.bak  //用于备份原来的sample_smartIPC_demo.c，方便后续通过对比文件更新升级。
    ├── sample_smartIPC_demo_ty.c //适配涂鸦接口的sample，微调
    ├── tkl_audio.c  //音频接口
    ├── tkl_video_enc.c
    ├── tkl_video_in.c //视频接口
    ├── audio_utils.c //播放aac音频接口
    └── util.c //二维码配网



mpp的版本记录：
commit b721867706f2d77ab9b63d05d3938bd0cc65f45f (HEAD -> tina-dev, tina/sunxi-platform-tina-v821, m/tina-dev)
Author: eric_wang <eric_wang@allwinnertech.com>
Date:   Mon Sep 15 08:34:18 2025 +0800

    sample_OnlineVenc support JPEG test.

    Change-Id: I468bb581db12c9f32e1600aee2b9f42dc7030f47
