14.1 
���� ��hot plug event����: ��Ҫ����class_device_create��mdev_main������ʵ�ֹ���

֮ǰд���豸������ע�ᣬ�����࣬Ȼ�󴴽��豸��Ϊ����mdev������Щ��Ϣ�������豸�ڵ�
�� ���� class_device_create()
    class_device_create()
		class_device_register(class_dev)
			 class_device_add(class_dev)
			 	kobject_uevent(&class_dev->kobj, KOBJ_ADD);
				 	kobject_uevent_env(kobj, action, NULL); 	��kobject_uevent.c��
						//action_string "add";
						action_string = action_to_string(action);	
						//���䱣�滷���������ڴ�
						/* environment values */
						buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
						//���û�������
						envp [i++] = scratch;
						scratch += sprintf(scratch, "ACTION=%s", action_string) + 1;
						envp [i++] = scratch;
						scratch += sprintf (scratch, "DEVPATH=%s", devpath) + 1;
						envp [i++] = scratch;
						scratch += sprintf(scratch, "SUBSYSTEM=%s", subsystem) + 1;
						/*����Ӧ�ó��򣺱���:mdev
						* ���ȷ��uevent_helper�� �鿴 cat /etc/init.d/rcS
						* �����ű� echo /sbin/mdev > /proc/sys/kernel/hotplug
						* ������uevent_helper Ϊ "/sbin/mdev��
						*/
						argv [0] = uevent_helper;   //  = "/sbim/mdev"
						argv [1] = (char *)subsystem;
						argv [2] = NULL;
						call_usermodehelper (argv[0], argv, envp, 0);

�� ���� busybox �е� mdev.c (�������ں˵ĺ�������busybox��)
  #���ذ���������ӡ�� ��������
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
			/*ȷ�� �豸�ļ��� ���� �����豸��*/
			device_name = bb_basename(path);  // ="buttons"
			mknod(device_name, mode | type, makedev(major, minor)
			���� /sys/class/button_drv/button/dev������ȷ�������豸��
			����temp�еĵ�5���ַ���'c'  ȷ�����ַ��豸�ڵ�

14.2 
������mdev.conf��ʹ��

Сʵ�飺 ����U�̣����Զ����أ���ô�죿
��ʵ�飺 ���������U�̣� ��
	# ls -l /dev/sda* 
		## /dev/sda  ��������U��
		## /dev/sda1 ����U�̵ĵ�һ������
	# mount /dev/sda1 /mnt 
		## cat /proc/mounts
		## ls /mnt
	# umount /mnt 

�� mdev.conf �����ļ�
  # ʹ��˵���μ�  \busybox-1.7.0\docs\�µ�mdev.txt
  # mdev.conf ��ʽ���� <device regex> <uid>:<gid> <octal permissions> [<@|$|*> <command>]
    ## device regex��������ʽ����ʾ��һ���豸
	## uid: ��ʾ�û�
	## git: ��ID
	## octal permissions: �԰˽��Ʊ�ʾ������
	## @ : �����豸�ڵ�֮ǰִ������  $ :ɾ���豸�ڵ�֮��ִ������  *:�����豸�ڵ�֮�� �� ɾ���豸�ڵ�֮ǰ ִ������
	## command :Ҫִ�е�����

  ������ʽ 
	# . ��ʾ�����ַ������з�����
	# * ��ʾ�ظ�0�� �� �����
	# + ��ʾ�ظ�1�� �� �����
	# �� ��ʾ�ظ�0�� �� 1��
	# [-] ��ʾ��Щ�ַ������ĳһ��

�� дmdev.conf�����ļ�

  # ���д������ʽ
    ## leds��led1��led2��led3 :�����һ��������ʽ��ʾ
	  ### leds?[123]?
  # �½�mdev.conf
  	## vi /etc/mdev.conf 
�������led.ko�������޸�Ȩ��Ϊ��

����1. 
leds 0:0 777
led1 0:0 777
led2 0:0 777
led3 0:0 777

����2.��������ʽƥ��
  # leds?[123]? 0:0 777 

����3. �������
  # leds?[123]? 0:0 777 @ echo create /dev/$MDEV(����������ʾ�ĸ��ڵ�) > /dev/console 


����4: ��������ʱ�������豸�� ж������ʱ��ɾ���豸��
  #  leds?[123]? 0:0 777 * if [ $ACTION = "add"];then echo create /dev/$MDEV > /dev/console; else echo remove /dev/$MDEV > /dev/console;fi

����5�� ������д��һ���ű� ��ʵ�鲻�ɹ���

# add_remove_led.sh

 leds?[123]? 0:0 777 *  /bin/add_remove_led.sh (�ű���Ŀ¼)
#�ű���������
#!/bin/sh
if[ $ACTION = "add"]��
then 
	echo create /dev/$MDEV > /dev/console;
else
	echo remove /dev/$MDEV > /dev/console;
fi 

# ���ű����ִ��Ȩ��
  ## chmod +x /bin/add_remove_led.sh

����6�� U���Զ�����
  # ls -l /dev/sda*
  # sda[1-9]+ 0:0 777 * if [ $ACTION = "add"];then mount create /dev/$MDEV > /mnt; else umount /mnt;fi

����7�� �ű���ʽ

# sda[1-9]+ 0:0 777 * /bin/add_remove_udisk.sh

add_remove_udisk.sh
#!/bin/sh
if [ $ACTION = "add"]
then 
		mount /dev/$MDEV /Mmnt;
else 
		umount /mnt;
fi


# ʵ������г��ֵ�����

	# ������д��һ���ű� ��ʵ�鲻�ɹ���