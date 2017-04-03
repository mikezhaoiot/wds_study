16.1
描述:printk的实现过程分析

注意：
  # uboot需要设置参数: console= ttySAC0
  # 内核里用printk打印, 肯定会调用硬件函数
  # 内核会根据命令行参数 来找到对应的硬件操作函数
① printk处理过程
  # uboot传入console=ttySAC0 console=tty1
  # 内核中搜索 console=ttySAC0
    ## __setup("console=",console_setup()) -> printk.c
		//记录下来
		add_preferred_console();	
			console_cmdline 数组，由谁调用
		//硬件驱动的入口函数
		register_concole(&s3c24xx_serial_console) -> drivers/serial/s3c2410.c
  # printk()的实现
    ## vprintk()
		//把输出信息放入临时buffer
		vscnprintf()
		//把临时buff里的数据稍作处理，写入log_buff, 比如printk("abc")会得到"<4>abc",再写入log_buff
		//可以用dmesg命令吧log_buff里的数据打印出来重现内核的输出信息

		//调用硬件的write输出
		release_console_sem()
			call_console_drivers() // 可以同时在PC 或 LCD 
				//从log_buf得到数据，算出级别
				_call_console_drivers()
					// 如果级别满足条件，打印
					__call_console_drivers()
						con->write();

16.2 
描述: printk的用法 ，可参见书籍第18章 Linux内核调试技术

①用法
  方法1 # define DBG_PRINTK  printk
    ## DBG_PRINKT("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
  # 折半打印调试
  方法2 # printk(KERN_DEBUG"%s %s %d\n",__FILE__,__FUNCTION__,__LINE__)
  	## 出现不打印的情况，需要修改打印级别  
	## cat /proc/sys/kernel/printk   7 4 1 7 
	## 修改打印机别 echo "8 4 1 7" > /proc/sys/kernel/printk
② 打印设置
  # 关闭打印信息
    ## 查看内核文档 kernel-paramenters.txt
		## loglevel 
	## set bootargs loglevel=0 其他参数不变
	## boot
③

待处理 ：
# 待办任务

练习 printk 打印等级 及设置
熟悉 printk 的流程
