6.1 6.2 
描述：回顾字符设备驱动程序

① 简介
  # 引入硬盘读数据。Flash的例子，
  # 块设备中把读写放入队列
  # 优化后在执行

② 框架

 app   open,read,write, "1.txt"
 ------------------------------------ 文件的读写
 文件系统:vfat ext2 ext3 yaffs2 jffs2 （把文件的读写转换为扇区的读写）
 -------------ll_rw_block------------ 扇区的读写
 					1. 把“读写”放入队列
					2. 调用队列的处理函数（优化/调顺序/合并）
 			块设备驱动程序
 -------------------------------------
 硬件： 	硬盘 flash

 参考书籍 <LINUX内核源代码情景分析>

③ 分析 ll_rw_block
  # for (i = 0; i < nr; i++) {
		struct buffer_head *bh = bhs[i];
		submit_bh(rw, bh);
			struct bio *bio;		//使用bh来构造bio (block input/output)
			submit_bio(rw, bio);
				//通用的构造请求: 使用bio来构造请求(request)
				generic_make_request(bio);	
					__generic_make_request(bio);
						q = bdev_get_queue(bio->bi_bdev);	//请求队列
						//调用队列的"构造请求函数"
						ret = q->make_request_fn(q, bio); -> q->make_request_fn = mfn; -> blk_queue_make_request(q, __make_request);
							// 默认的函数是__make_request
							blk_init_queue_node（）
								__make_request（）
									//先尝试合并， 采用电梯调度算法
									elv_merge(q, &req, bio)
									//如果合并不成，使用bio构造请求
									init_request_from_bio(req, bio);
									//把请求放入队列
									add_request(q, req);
									//执行队列
									__generic_unplug_device(q);
										// 调用队列的“处理函数”
										q->request_fn(q);
④ 如何写块设备驱动程序
  # 分配gendisk结构体 : alloc_disk
  # 设置
  	## 分配/设置队列： request_queue_t //它提供读写能力
	## 设置gendisk其他信息			// 它提供属性：比如容量
  # 注册: add_disk

6.3 
描述: 内存模拟块设备驱动程序 


① 参考相关文件
  # xd.c 
  # z2ram.c 
② 步骤
  # 入口函数
  	## static int ramblock_init(void)
	  ### 分配一个gendisk结构体
	  ### 设置
	  	#### 设置队列

	  ### 注册
  # 出口函数
    ## static int ramblock_exit(void)

# 待写 ：搭建一个块设备驱动框架