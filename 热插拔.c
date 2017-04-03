14.1 
描述 ：hot plug event机制: 主要分析class_device_create、mdev_main函数的实现过程

之前写的设备驱动，注册，创建类，然后创建设备：为了让mdev根据这些信息来创建设备节点
① 分析 class_device_create()
    class_device_create()
		class_device_register(class_dev)
			 class_device_add(class_dev)
			 	kobject_uevent(&class_dev->kobj, KOBJ_ADD);
				 	kobject_uevent_env(kobj, action, NULL); 	（kobject_uevent.c）
						//action_string "add";
						action_string = action_to_string(action);	
						//分配保存环境变量的内存
						/* environment values */
						buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
						//设置环境变量
						envp [i++] = scratch;
						scratch += sprintf(scratch, "ACTION=%s", action_string) + 1;
						envp [i++] = scratch;
						scratch += sprintf (scratch, "DEVPATH=%s", devpath) + 1;
						envp [i++] = scratch;
						scratch += sprintf(scratch, "SUBSYSTEM=%s", subsystem) + 1;
						/*调用应用程序：比如:mdev
						* 如何确定uevent_helper， 查看 cat /etc/init.d/rcS
						* 启动脚本 echo /sbin/mdev > /proc/sys/kernel/hotplug
						* 设置了uevent_helper 为 "/sbin/mdev”
						*/
						argv [0] = uevent_helper;   //  = "/sbim/mdev"
						argv [1] = (char *)subsystem;
						argv [2] = NULL;
						call_usermodehelper (argv[0], argv, envp, 0);

② 分析 busybox 中的 mdev.c (并不是内核的函数，在busybox中)
  #加载按键驱动打印的 环境变量
	envp[1] = PATH=/sbin:/bin:/usr/sbin:/usr/bin
	envp[2] = ACTION=add
	envp[3] = DEVPATH=/class/button_drv/button
	envp[4] = SUBSYSTEM=button_drv
	envp[5] = SEQNUM=743
	envp[6] = MAJOR=221
	envp[7] = MINOR=0
	mdev_main
		temp = /sys/class/button_drv/button 
		make_device(temp, 0);
			/*确定 设备文件名 类型 主次设备号*/
			device_name = bb_basename(path);  // ="buttons"
			mknod(device_name, mode | type, makedev(major, minor)
			根据 /sys/class/button_drv/button/dev的内容确定主次设备号
			根据temp中的第5个字符串'c'  确定是字符设备节点

14.2 
描述：mdev.conf的使用

小实验： 接上U盘，想自动挂载，怎么办？
先实验： 开发板插入U盘， ‘
	# ls -l /dev/sda* 
		## /dev/sda  代表整个U盘
		## /dev/sda1 代表U盘的第一个分区
	# mount /dev/sda1 /mnt 
		## cat /proc/mounts
		## ls /mnt
	# umount /mnt 

① mdev.conf 配置文件
  # 使用说明参见  \busybox-1.7.0\docs\下的mdev.txt
  # mdev.conf 格式如下 <device regex> <uid>:<gid> <octal permissions> [<@|$|*> <command>]
    ## device regex：正则表达式，表示哪一个设备
	## uid: 表示用户
	## git: 组ID
	## octal permissions: 以八进制表示的属性
	## @ : 创建设备节点之前执行命令  $ :删除设备节点之后执行命令  *:创建设备节点之后 和 删除设备节点之前 执行命令
	## command :要执行的命令

  正则表达式 
	# . 表示任意字符，换行符除外
	# * 表示重复0次 或 更多次
	# + 表示重复1次 或 更多次
	# ？ 表示重复0次 或 1次
	# [-] 表示这些字符里面的某一个

② 写mdev.conf配置文件

  # 如何写正则表达式
    ## leds、led1、led2、led3 :如何用一个正则表达式表示
	  ### leds?[123]?
  # 新建mdev.conf
  	## vi /etc/mdev.conf 
比如加载led.ko驱动，修改权限为例

方法1. 
leds 0:0 777
led1 0:0 777
led2 0:0 777
led3 0:0 777

方法2.用正则表达式匹配
  # leds?[123]? 0:0 777 

方法3. 添加命令
  # leds?[123]? 0:0 777 @ echo create /dev/$MDEV(环境变量表示哪个节点) > /dev/console 


方法4: 加载驱动时，创建设备； 卸载驱动时，删除设备；
  #  leds?[123]? 0:0 777 * if [ $ACTION = "add"];then echo create /dev/$MDEV > /dev/console; else echo remove /dev/$MDEV > /dev/console;fi

方法5： 把命令写入一个脚本 （实验不成功）

# add_remove_led.sh

 leds?[123]? 0:0 777 *  /bin/add_remove_led.sh (脚本的目录)
#脚本内容如下
#!/bin/sh
if[ $ACTION = "add"]；
then 
	echo create /dev/$MDEV > /dev/console;
else
	echo remove /dev/$MDEV > /dev/console;
fi 

# 给脚本添加执行权限
  ## chmod +x /bin/add_remove_led.sh

方法6： U盘自动加载
  # ls -l /dev/sda*
  # sda[1-9]+ 0:0 777 * if [ $ACTION = "add"];then mount create /dev/$MDEV > /mnt; else umount /mnt;fi

方法7： 脚本方式

# sda[1-9]+ 0:0 777 * /bin/add_remove_udisk.sh

add_remove_udisk.sh
#!/bin/sh
if [ $ACTION = "add"]
then 
		mount /dev/$MDEV /Mmnt;
else 
		umount /mnt;
fi


# 实验过程中出现的问题

	# 把命令写入一个脚本 （实验不成功）