
2017/03/20
1.1
���� : Ӧ�á������Ĵ�ſ���˽� 
�� ����
    # Ӧ�õ���open()����������C��ʵ�ֵ�
    # ִ��swiָ������ں��쳣��������ϵͳ����sys_open()
    # �ں˵��쳣������������Щ�����ҵ���Ӧ����������������һ���ļ�������⣬�������ظ�Ӧ�ó��� 

1.2

���� ��Ӧ�ó��������Ĺ���  
�� ��д�������� : first_drv_open() first_drv_read() first_drv_write()
�� ��ô�����ں� : �ȶ���һ���ṹ�� static struct file_operations first_drv_fops = {},Ȼ�����
�� ������ṹ������ں� : int register_chrdev();       //ע���������� 
�� ˭����������ں����������� int register_chrdev����: int first_drv_init(void) 
�� �ں���ε���������ں��� : module_init(first_drv_init); (����һ���ṹ�壬����ĺ���ָ��ָ����ں���)
    # ls /dev -l �鿴�豸
�� Ӧ�ó������ȥ������������, ��Ƶ��13:15 ����ϸ����
    # Ӧ�ò� open("/dev/xxx" ) �������ԣ����豸�š�ͨ�����͡����豸�� ������ ��Ӧ������ 
    # �����ںˣ������һ���ַ��͵Ľṹ�壬ͨ�����豸���ҵ� file_operations �ṹ��
    # ��ں��������棬��register_chrdev������file_operations �ṹ������ں��е��ַ��������У�
�� ������������
    #����Makefile�˽�
        ## KERN_DIR ��ʾ�ں�Դ��Ŀ¼�����ַ�ʽ������Ƕ��ʽ�����Ľ�����룬Ŀ¼�������ں�����ģ������Ҫ�ĸ���ͷ�ļ�������
        ## obj-m += first_drv.o  //��ģ�鲻����뵽zImage,������һ��������first_drv.ko��̬����
        ## obj-y += first_drv.o  //��ģ����뵽zImage 

  ��; ����: �������̬IP "ifconfig����ʾip���������̬IP���÷���"
1.3  

���� �� �������������豸�ڵ����֪ʶ
�� ������������
   # ������  cp first_drv.ko /work/nfs_root/tmp/fs_mini_mdev/
   # ������������ insmod first_drv.ko  ж���������� rmmod first_drv.ko
   # �鿴�ں�֧�ֵ��ַ��豸 cat /proc/devices���Ѿ�ע��
   # ��д���Գ��� arm-linux-gcc -o  main main.c
     ##ִ��./main , �����������豸�ڵ�: cannot open ,��Ҫ�����豸�ڵ�
     ##�����豸�ڵ� ��mknod /dev/xxx c 111 0 

�� ���������з������豸��
    # �ֶ����
    # �Զ�����, �����豸��Ϊ0ʱ��ϵͳ�Զ��������豸��
      ## rmmod first_drv.ko //ɾ������        
      ## lsmod     //�鿴����
	  ## rm /dev/xxx
�� Ӧ�ó�����,���豸�ļ���
    # �ֶ����� mknod /dev/xxx c(�ַ��豸) ���豸��  ���豸��
	# �Զ����� mdev(udev����) : ����ϵͳ��Ϣ�����豸�ڵ�
      ## ��غ��� class_create������class_device_create���������豸
      ## cd /sys/class �鿴�����и�����
      ## echo /sbin/mdev > /proc/sys/kernel/hotplug���Ȱβ壩 vi /etc/init.d/rcS �е�  mdev -s

1.4  

���� �� ����һ��LED��������д�������򣬷���myleds��ʵ�ַ���

�� ���
�� ����Ӳ������
  # ��ԭ��ͼ
  # ��оƬ�ֲ�
  # ��д���� ����Ƭ���в��������ַ��Linux�����в��������ַ��ͨ��ӳ��
�� ��Ҫ����˵��
  # ioremap()�����ַӳ���������ַ
  # copy_from_user() �û��ռ俽�����ں˿ռ�
�� �Ľ����򣺵���ָ��LED�� �μ�myleds
  # ls /dev/led* -l  �鿴�����豸

������������������⣺
  # ���豸�Ŷ���0
  # �ź���������

1.5

���� �� ����������ͨ����ѯ��ʽ��ȡ����ֵ 

�� ��ܴ
�� Ӳ������
    # ������ַӳ��
    # copy_to_user()
    # top ���� ������windows�µ������������

2017/03/23 
1.6 
���� �� �жϷ�ʽ��ȡ��ֵ���쳣������������
�жϷ�ʽ��ȡ����ֵ��
    # �а������£�CPU�����ж�
    # ǿ�Ƶ�ת���쳣�������ִ�У�
        ## ���汻�жϵ��ֳ�
        ## ִ���жϴ�����
        ## �ָ����жϵ��ֳ�
�� �жϿ�ܴ  
    # �쳣�������:ARM�ܹ�CPU���쳣������ַ������0x00 00 00 00,Ҳ������0xff ff 00 00 Linux�ں�ʹ�ú��ߡ�tarp_init�������쳣�������Ƶ�0xff ff 00 00 ��
    # �õ�ַ������Ӧʵ�ʵ��ڴ棬��Ҫ���������ַ�������ַ֮���ӳ��
    # trap_init() ������
    # #define CONFIG_VECTORS_BASE 0xffff0000
    # unsigned long vectors = CONFIG_VECTORS_BASE;
    # memcpy((void *)vectors, __vectors_start, __vectors_end - __vectors_start);
    # __vectors_start: ��һЩӲ����תָ�
        ## b	vector_und + stubs_offset
    # vector_stub	und, UND_MODE ��vector_stub ��չ����
�� �쳣����
    # trap_init()�����쳣����: __vectors_start ��δ��뿽����vectors��0xffff0000)�� ����ʵ�� ��memcpy((void *)vectors, __vectors_start, __vectors_end - __vectors_start); 
    # __vectors_start �� 
        ## b	vector_irq + stubs_offset
        ## vector_irq �ú���ʵ��: vector_stub	irq, IRQ_MODE, 4
            ### .long	__irq_usr			@  0  (USR_26 / USR_32)
            ### __irq_usr: ����ʵ��
                ### irq_handler ����ʵ�֣� ���յ��� asm_do_IRQ

1.7 
����������asm_do_IRQ���˽��ж���ν���
�� ��Ƭ�����ں˵��ж�����

��Ƭ���жϴ������̣�
  # �ֱ�����һ���ж�
  # ���ö�Ӧ�Ĵ�����
  # ���ж�
�ں��жϴ���
  # ��Ƭ���е�������� asm_do_IRQ��ʵ��  
  # desc_handle_irq()�е� desc->handle_irq(irq, desc);
  # __set_irq_handler() �е��� desc->handle_irq = handle;
  # set_irq_handler()
  # s3c24xx_init_irq() �жϳ�ʼ�� (arch\arm\plat-s3c24xx)
    ## handle_edge_irq���� ������ش����ж�
        ### desc->chip->ack(irq); ���ж�
        ### handle_IRQ_event    �����ж�
            #### ȡ�� action  �����еĳ�Ա
            #### ִ��  action->handle
# ���°���
# CPU�����쳣������ 
    ## b	vector_irq + stubs_offset
# asm_do_IRQ

�� ����irq_desc �ṹ��
  # handle_irq ��ָ�� handle_edge_irq����
  # chip : оƬ��صײ����
  # action �� 
  
�� ����request_irq() (kernel\irq\Manage.c) 
  # ���� irqaction �ṹ��������ṹ����irq_desc�������action������ 
  # setup_irq���������ж�
    ## ��action �����м������ж�
    ## dest->chip  
  # ʹ���ж�
  # free_irq 
    ## ������
    ## ��ֹ�ж�

#############################################################################################
1.8 
����: ���������жϰ����������� �� �Ż���������

�� ��ʼ��ϵ�к�������Ƶ��13:21 
  # ���ų�ʼ��
  # request_irq(IRQ_EINT0,buttons_irq,IRQ_BOTHEDGE,"S2",1);
  # free_irq(IRQ_EINT0,1);
  # ��֤�ж�
    ## cat /proc/interrupts
    ## exec������豸 exec 5</dev/buttons    �ر�exec 5<&-
      ### ���豸����λ��5��
      ### ps �鿴��ǰ���̣�Ϊ771
      ### ls -l /proc/771/fd 
    ## cat /proc/interrupts 

�� �Ľ��жϴ�����
  # ȷ������ֵ 
�� �鿴����
  # ps 

1.9 
����: poll����  �μ�poll���Ʒ���

�� ����poll����
  # ���������:poll_wait()
  # poll���Ʒ���
    ## app: poll
    ## kernel: sys_poll
        ### do_sys_poll����
                poll_initwait(&table)
                    poll_initwait(&table) > init_poll_funcptr(&pwq->pt, __pollwait) > pt->qproc = qproc
                    
                    table->pt->qproc = __pollwait
            do_poll(nfds, head, &table, timeout)
                for(;;)
                {
                   	if (do_pollfd(pfd, pt)) {  > mask = file->f_op->poll(file, pwait); return mask 
                                                //��������poll
					    count++;
					    pt = NULL;
                    }
                    //count ��0����ʱ�����źŵȴ�����
                    if (count || !*timeout || signal_pending(current))
			            break;
                    //����__timeout
                    __timeout = schedule_timeout(__timeout);
                }
�� ������poll_wait()
  # p->qproc(filp, wait_address, p) �滻 __pollwait(filp, wait_address, p)
  # ��ǰ���̹ҵ�wait_address������ȥ

�� ��д�������� 

1.9.1 

����: �첽֪ͨ

�� ��ȡ�����ķ�ʽ
 # ��ѯ��ʽ
 # �жϷ�ʽ
 # poll���� 
 # �첽֪ͨ

�� С�������: ���̼�ͨ�� - �ź���
  # ��غ�����signal(SIGUSR1,my)
    ## ./signal & // ��ִ̨��
    ## ps   //�鿴
    ## kill -USR1 833(���̺�)// �����ź�
    ## kill -9 833 // ɱ������
  # ����
    ## ע���źŴ�����
    ## ˭��
    ## ����˭
    ## ��ô��
�� ������������
  # ����
    ## Ӧ�ó��� ע���źŴ�����
    ## ����������
    ## ����Ӧ�ó���Ӧ�ó���Ҫ��������PID
    ## ������������kill_fasync()����
  # ������غ���
    ## kill_fasync()    //�����ź�
    ## fasync_helper()  // ��ʼ��
    ## Ӧ�ó�����
        ### signal(SIGIO,my)��
        ### fcntl()  //��������

1.9.2 
����:ԭ�Ӳ������ź��������������˽�
��رʼǣ�ͬ����������.txt

Ŀ�ģ�ͬһʱ�̣�ֻ����һ��Ӧ�ó������������ 
�� ����ԭ�Ӳ���
  # ��ͳͨ����־λ��ʽ����©�����ԼӲ����ڻ����ʵ�ʷ�Ϊ����ʵ�еģ����ܳ���ĳһ�������л�����
�� �ź���
  # ��ʼ���ź���
  # ��ȡ�ź��� 
  # �ͷ��ź���
�� �����������
 
1.9.3 
���������붨ʱ������ԭ��,�Ż�֮ǰ�İ����жϳ���

�� ����
�� ��غ���
  # init_timer();
  # add_timer();
  # mod_timer();





 


                     






	



