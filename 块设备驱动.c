6.1 6.2 
�������ع��ַ��豸��������

�� ���
  # ����Ӳ�̶����ݡ�Flash�����ӣ�
  # ���豸�аѶ�д�������
  # �Ż�����ִ��

�� ���

 app   open,read,write, "1.txt"
 ------------------------------------ �ļ��Ķ�д
 �ļ�ϵͳ:vfat ext2 ext3 yaffs2 jffs2 �����ļ��Ķ�дת��Ϊ�����Ķ�д��
 -------------ll_rw_block------------ �����Ķ�д
 					1. �ѡ���д���������
					2. ���ö��еĴ��������Ż�/��˳��/�ϲ���
 			���豸��������
 -------------------------------------
 Ӳ���� 	Ӳ�� flash

 �ο��鼮 <LINUX�ں�Դ�����龰����>

�� ���� ll_rw_block
  # for (i = 0; i < nr; i++) {
		struct buffer_head *bh = bhs[i];
		submit_bh(rw, bh);
			struct bio *bio;		//ʹ��bh������bio (block input/output)
			submit_bio(rw, bio);
				//ͨ�õĹ�������: ʹ��bio����������(request)
				generic_make_request(bio);	
					__generic_make_request(bio);
						q = bdev_get_queue(bio->bi_bdev);	//�������
						//���ö��е�"����������"
						ret = q->make_request_fn(q, bio); -> q->make_request_fn = mfn; -> blk_queue_make_request(q, __make_request);
							// Ĭ�ϵĺ�����__make_request
							blk_init_queue_node����
								__make_request����
									//�ȳ��Ժϲ��� ���õ��ݵ����㷨
									elv_merge(q, &req, bio)
									//����ϲ����ɣ�ʹ��bio��������
									init_request_from_bio(req, bio);
									//������������
									add_request(q, req);
									//ִ�ж���
									__generic_unplug_device(q);
										// ���ö��еġ���������
										q->request_fn(q);
�� ���д���豸��������
  # ����gendisk�ṹ�� : alloc_disk
  # ����
  	## ����/���ö��У� request_queue_t //���ṩ��д����
	## ����gendisk������Ϣ			// ���ṩ���ԣ���������
  # ע��: add_disk

6.3 
����: �ڴ�ģ����豸�������� 


�� �ο�����ļ�
  # xd.c 
  # z2ram.c 
�� ����
  # ��ں���
  	## static int ramblock_init(void)
	  ### ����һ��gendisk�ṹ��
	  ### ����
	  	#### ���ö���

	  ### ע��
  # ���ں���
    ## static int ramblock_exit(void)

# ��д ���һ�����豸�������