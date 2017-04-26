## 描述

 - JZ2440 V2 开发板烧录固件

## uImage_3.4.2-dm9000-201170411

 - 描述: 内核3.4.2固件,无裁剪
 - 支持jffs2文件格式，不支持yaffs格式
 - 支持dm9000网卡， nfs可以正常通讯
 
## u-boot-1.1.6_kernel-2M.bin 

 -  u-boot-1.1.6 编译

  
## u-boot-1.1.6_kernel-4M.bin 

 - 重新修改分区，kernel为4M 

## fs_mini_mdev_new.jffs2

 - busybox-1.20.0 制作的文件系统
 - [源码][1]

##　uImage_wm8976

 - linux-3.4.2 打补丁`linux-3.4.2_alsa_wm8976_uda1341_jz2440_mini2440_tq2440.patch`
 - cp config_jz2440 .config 配置jz2440，支持WM8976音频驱动
 - 主要支持wm8976音频驱动

  [1]: https://github.com/wisezhao/wds_study/tree/FileSystem-jffs2
