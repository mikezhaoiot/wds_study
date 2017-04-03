
2017/03/20
1.1
描述 : 应用、驱动的大概框架了解 
① 过程
    # 应用调用open()函数，都是C库实现的
    # 执行swi指令进入内核异常处理，调用系统函数sys_open()
    # 内核的异常处理函数根据这些参数找到对应的驱动函数，返回一个文件句柄给库，进而返回给应用程序 

1.2

描述 ：应用程序到驱动的过程  
① 编写驱动函数 : first_drv_open() first_drv_read() first_drv_write()
② 怎么告诉内核 : 先定义一个结构体 static struct file_operations first_drv_fops = {},然后填充
③ 把这个结构体告诉内核 : int register_chrdev();       //注册驱动程序 
④ 谁（驱动的入口函数）来调用 int register_chrdev（）: int first_drv_init(void) 
⑤ 内核如何调用驱动入口函数 : module_init(first_drv_init); (定义一个结构体，里面的函数指针指向入口函数)
    # ls /dev -l 查看设备
⑥ 应用程序如何去调用驱动程序, 视频中13:15 处详细讲解
    # 应用层 open("/dev/xxx" ) 它的属性：主设备号、通过类型、主设备号 和驱动 对应起来； 
    # 进入内核，会分配一个字符型的结构体，通过主设备号找到 file_operations 结构；
    # 入口函数中里面，用register_chrdev（）把file_operations 结构体放入内核中的字符型数组中；
⑦ 编译驱动程序
    #驱动Makefile了解
        ## KERN_DIR 表示内核源码目录，这种方式适用于嵌入式开发的交叉编译，目录包含了内核驱动模块所需要的各种头文件及依赖
        ## obj-m += first_drv.o  //该模块不会编译到zImage,会生成一个独立的first_drv.ko静态编译
        ## obj-y += first_drv.o  //该模块编译到zImage 

  中途 设置: 虚拟机静态IP "ifconfig不显示ip，虚拟机静态IP设置方法"
1.3  

描述 ： 编译驱动程序，设备节点相关知识
① 编译驱动程序
   # 拷贝至  cp first_drv.ko /work/nfs_root/tmp/fs_mini_mdev/
   # 加载驱动程序 insmod first_drv.ko  卸载驱动程序 rmmod first_drv.ko
   # 查看内核支持的字符设备 cat /proc/devices，已经注册
   # 编写测试程序 arm-linux-gcc -o  main main.c
     ##执行./main , 报错，不存在设备节点: cannot open ,需要创建设备节点
     ##创建设备节点 ：mknod /dev/xxx c 111 0 

② 驱动程序中分配主设备号
    # 手动添加
    # 自动分配, 当主设备号为0时，系统自动分配主设备号
      ## rmmod first_drv.ko //删除驱动        
      ## lsmod     //查看驱动
	  ## rm /dev/xxx
③ 应用程序中,打开设备文件，
    # 手动创建 mknod /dev/xxx c(字符设备) 主设备号  次设备号
	# 自动创建 mdev(udev机制) : 根据系统信息创建设备节点
      ## 相关函数 class_create（）、class_device_create（）建立设备
      ## cd /sys/class 查看下面有各种类
      ## echo /sbin/mdev > /proc/sys/kernel/hotplug（热拔插） vi /etc/init.d/rcS 中的  mdev -s

1.4  

描述 ： 点亮一个LED驱动，编写驱动程序，分析myleds的实现方法

① 框架
② 完善硬件操作
  # 看原理图
  # 看芯片手册
  # 编写代码 ：单片机中操作物理地址，Linux驱动中操作虚拟地址，通过映射
③ 主要函数说明
  # ioremap()物理地址映射至虚拟地址
  # copy_from_user() 用户空间拷贝至内核空间
④ 改进程序：点亮指定LED灯 参见myleds
  # ls /dev/led* -l  查看驱动设备

编码过程中遇到的问题：
  # 次设备号都是0
  # 信号量的问题

1.5

描述 ： 按键驱动，通过查询方式获取按键值 

① 框架搭建
② 硬件操作
    # 建立地址映射
    # copy_to_user()
    # top 命令 类似于windows下的任务管理器器

2017/03/23 
1.6 
描述 ： 中断方式获取键值，异常向量处理流程
中断方式获取按键值：
    # 有按键按下，CPU发生中断
    # 强制调转到异常向量入口执行，
        ## 保存被中断的现场
        ## 执行中断处理函数
        ## 恢复被中断的现场
① 中断框架搭建  
    # 异常向量入口:ARM架构CPU的异常向量基址可以是0x00 00 00 00,也可以是0xff ff 00 00 Linux内核使用后者。tarp_init函数将异常向量复制到0xff ff 00 00 处
    # 该地址并不对应实际的内存，需要建立虚拟地址与物理地址之间的映射
    # trap_init() 函数中
    # #define CONFIG_VECTORS_BASE 0xffff0000
    # unsigned long vectors = CONFIG_VECTORS_BASE;
    # memcpy((void *)vectors, __vectors_start, __vectors_end - __vectors_start);
    # __vectors_start: （一些硬件跳转指令）
        ## b	vector_und + stubs_offset
    # vector_stub	und, UND_MODE （vector_stub 宏展开）
② 异常向量
    # trap_init()构造异常向量: __vectors_start 这段代码拷贝至vectors（0xffff0000)， 代码实现 ：memcpy((void *)vectors, __vectors_start, __vectors_end - __vectors_start); 
    # __vectors_start 宏 
        ## b	vector_irq + stubs_offset
        ## vector_irq 用宏来实现: vector_stub	irq, IRQ_MODE, 4
            ### .long	__irq_usr			@  0  (USR_26 / USR_32)
            ### __irq_usr: 宏来实现
                ### irq_handler 宏来实现， 最终调用 asm_do_IRQ

1.7 
描述：解析asm_do_IRQ、了解中断如何进行
① 单片机和内核的中断流程

单片机中断处理流程：
  # 分辨是哪一个中断
  # 调用对应的处理函数
  # 清中断
内核中断处理：
  # 单片机中的三项都是由 asm_do_IRQ来实现  
  # desc_handle_irq()中的 desc->handle_irq(irq, desc);
  # __set_irq_handler() 中调用 desc->handle_irq = handle;
  # set_irq_handler()
  # s3c24xx_init_irq() 中断初始化 (arch\arm\plat-s3c24xx)
    ## handle_edge_irq（） 处理边沿触发中断
        ### desc->chip->ack(irq); 清中断
        ### handle_IRQ_event    处理中断
            #### 取出 action  链表中的成员
            #### 执行  action->handle
# 按下按键
# CPU进入异常处理函数 
    ## b	vector_irq + stubs_offset
# asm_do_IRQ

② 分析irq_desc 结构体
  # handle_irq ：指向 handle_edge_irq（）
  # chip : 芯片相关底层操作
  # action ： 
  
③ 分析request_irq() (kernel\irq\Manage.c) 
  # 分配 irqaction 结构，把这个结构放入irq_desc数组项的action链表中 
  # setup_irq（）设置中断
    ## 在action 链表中加入新中断
    ## dest->chip  
  # 使能中断
  # free_irq 
    ## 出链表
    ## 禁止中断

#############################################################################################
1.8 
描述: 初步测试中断按键驱动程序 及 优化驱动程序

① 初始化系列函数：视频中13:21 
  # 引脚初始化
  # request_irq(IRQ_EINT0,buttons_irq,IRQ_BOTHEDGE,"S2",1);
  # free_irq(IRQ_EINT0,1);
  # 验证中断
    ## cat /proc/interrupts
    ## exec命令：打开设备 exec 5</dev/buttons    关闭exec 5<&-
      ### 打开设备，定位到5，
      ### ps 查看当前进程，为771
      ### ls -l /proc/771/fd 
    ## cat /proc/interrupts 

② 改进中断处理函数
  # 确定按键值 
③ 查看性能
  # ps 

1.9 
描述: poll机制  参见poll机制分析

① 增加poll机制
  # 驱动中添加:poll_wait()
  # poll机制分析
    ## app: poll
    ## kernel: sys_poll
        ### do_sys_poll（）
                poll_initwait(&table)
                    poll_initwait(&table) > init_poll_funcptr(&pwq->pt, __pollwait) > pt->qproc = qproc
                    
                    table->pt->qproc = __pollwait
            do_poll(nfds, head, &table, timeout)
                for(;;)
                {
                   	if (do_pollfd(pfd, pt)) {  > mask = file->f_op->poll(file, pwait); return mask 
                                                //驱动程序poll
					    count++;
					    pt = NULL;
                    }
                    //count 非0、超时、有信号等待处理
                    if (count || !*timeout || signal_pending(current))
			            break;
                    //休眠__timeout
                    __timeout = schedule_timeout(__timeout);
                }
② 驱动中poll_wait()
  # p->qproc(filp, wait_address, p) 替换 __pollwait(filp, wait_address, p)
  # 当前进程挂到wait_address队列里去

③ 编写驱动程序 

1.9.1 

描述: 异步通知

① 读取按键的方式
 # 查询方式
 # 中断方式
 # poll机制 
 # 异步通知

② 小程序测试: 进程间通信 - 信号量
  # 相关函数：signal(SIGUSR1,my)
    ## ./signal & // 后台执行
    ## ps   //查看
    ## kill -USR1 833(进程号)// 发送信号
    ## kill -9 833 // 杀掉进程
  # 步骤
    ## 注册信号处理函数
    ## 谁发
    ## 发给谁
    ## 怎么发
③ 按键驱动程序
  # 步骤
    ## 应用程序 注册信号处理函数
    ## 驱动程序发送
    ## 发给应用程序，应用程序要告诉驱动PID
    ## 调用驱动函数kill_fasync()发送
  # 程序相关函数
    ## kill_fasync()    //发出信号
    ## fasync_helper()  // 初始化
    ## 应用程序中
        ### signal(SIGIO,my)　
        ### fcntl()  //告诉驱动

1.9.2 
描述:原子操作、信号量、阻塞初步了解
相关笔记：同步互斥阻塞.txt

目的：同一时刻，只能有一个应用程序打开驱动程序 
① 引入原子操作
  # 传统通过标志位方式存在漏洞，自加操作在汇编中实际分为三步实行的，可能出现某一步出现切换操作
② 信号量
  # 初始化信号量
  # 获取信号量 
  # 释放信号量
③ 阻塞与非阻塞
 
1.9.3 
描述：引入定时器防抖原理,优化之前的按键中断程序

① 概念
② 相关函数
  # init_timer();
  # add_timer();
  # mod_timer();





 


                     






	



