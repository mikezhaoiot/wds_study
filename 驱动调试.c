16.1
����:printk��ʵ�ֹ��̷���

ע�⣺
  # uboot��Ҫ���ò���: console= ttySAC0
  # �ں�����printk��ӡ, �϶������Ӳ������
  # �ں˻���������в��� ���ҵ���Ӧ��Ӳ����������
�� printk�������
  # uboot����console=ttySAC0 console=tty1
  # �ں������� console=ttySAC0
    ## __setup("console=",console_setup()) -> printk.c
		//��¼����
		add_preferred_console();	
			console_cmdline ���飬��˭����
		//Ӳ����������ں���
		register_concole(&s3c24xx_serial_console) -> drivers/serial/s3c2410.c
  # printk()��ʵ��
    ## vprintk()
		//�������Ϣ������ʱbuffer
		vscnprintf()
		//����ʱbuff���������������д��log_buff, ����printk("abc")��õ�"<4>abc",��д��log_buff
		//������dmesg�����log_buff������ݴ�ӡ���������ں˵������Ϣ

		//����Ӳ����write���
		release_console_sem()
			call_console_drivers() // ����ͬʱ��PC �� LCD 
				//��log_buf�õ����ݣ��������
				_call_console_drivers()
					// �������������������ӡ
					__call_console_drivers()
						con->write();

16.2 
����: printk���÷� ���ɲμ��鼮��18�� Linux�ں˵��Լ���

���÷�
  ����1 # define DBG_PRINTK  printk
    ## DBG_PRINKT("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
  # �۰��ӡ����
  ����2 # printk(KERN_DEBUG"%s %s %d\n",__FILE__,__FUNCTION__,__LINE__)
  	## ���ֲ���ӡ���������Ҫ�޸Ĵ�ӡ����  
	## cat /proc/sys/kernel/printk   7 4 1 7 
	## �޸Ĵ�ӡ���� echo "8 4 1 7" > /proc/sys/kernel/printk
�� ��ӡ����
  # �رմ�ӡ��Ϣ
    ## �鿴�ں��ĵ� kernel-paramenters.txt
		## loglevel 
	## set bootargs loglevel=0 ������������
	## boot
��

������ ��
# ��������

��ϰ printk ��ӡ�ȼ� ������
��Ϥ printk ������
