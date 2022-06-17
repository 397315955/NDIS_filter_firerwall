//#ifndef _ICC_
//#define _ICC_
#include <ndis.h>
#include <filteruser.h>
#include<ntddk.h>

#define	ETHER_ADDR_LEN		6
#define IPV4_LENGTH 0x4 //ipv4����
#define IPV6_LENGTH 0x10 //ipv6����
#define TYPE_IPV4  0x0008 //ipv4Э������
#define TYPE_IPV6  0xdd86 //ipv6Э������
#define PROTOCOL_UDP 0x11 //UDPЭ������
#define PROTOCOL_TCP 0x6  //TCPЭ������

#define NEW_FC_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x999,METHOD_BUFFERED,FILE_READ_ACCESS)//���ӹ���
#define DEL_FC_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x99a,METHOD_BUFFERED,FILE_READ_ACCESS)//ɾ������
#define START_FC_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x99b,METHOD_BUFFERED,FILE_READ_ACCESS)//����
#define STOP_FC_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x99c,METHOD_BUFFERED,FILE_READ_ACCESS)//ֹͣ
#define EXIT_FC_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x99d,METHOD_BUFFERED,FILE_READ_ACCESS)//ж��
#define DEL_ALL_FC_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x99e,METHOD_BUFFERED,FILE_READ_ACCESS)//ɾ�����й���
#define GET_B_FC_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x99f,METHOD_NEITHER,FILE_ANY_ACCESS)//��ȡ����Ự�б�
#define GET_B_NUM_CODE CTL_CODE(FILE_DEVICE_UNKNOWN,0x9a0,METHOD_NEITHER,FILE_ANY_ACCESS)//��ȡ����Ự��Ŀ
typedef PUCHAR PPacket;

//--------------------------------------------�����-----------------------------------------------------------------------------------------------------------------------------------------
//��һ����������IO�Թ��˹������ɾ
//�ڶ����������Ƕ��º͵ĻỰ������
//�������������ǶԶ���ĻỰ������
KSPIN_LOCK IC_LOCK, IC_LOCK2, IC_LOCK3;
KIRQL IC_IRQL, IC_IRQL2, IC_IRQL3;

//-------------------------------------------�������˹���----------------------------------------------------------------------------------------------------------------------------------------

int start_flag = 0;//�Ƿ������˹��ܣ�1������0�ر�



//-------------------------------Э��ջ�õĽṹ------------------------------------------------------------------------------------------------------------------------------------------------
//·����·��
typedef struct	ether_header {
	UCHAR	ether_dhost[ETHER_ADDR_LEN];
	UCHAR	ether_shost[ETHER_ADDR_LEN];
	USHORT	ether_type;
}TP_ETHERNET, *PTP_ETHERNET;

//���������
typedef struct ipv4 {
	PUCHAR Src_add;//ԴIP
	PUCHAR Dst_add;//Ŀ��IP
	PUCHAR Protocol;//�ϲ�Э��
	ULONG Length;//ipͷ����

}IP_V4,* PIP_V4;

typedef struct ipv6 {
	PUCHAR Src_add;//ԴIP
	PUCHAR Dst_add;//Ŀ��IP
	PUCHAR Protocol;//�ϲ�Э��
	ULONG Length;//ipͷ����
}IP_V6, *PIP_V6;

typedef union network_layer
{
	//���������������
	IP_V4 ipv4;
	IP_V6 ipv6;

}Network_Layer;

//���������
//UDP �ṹ ���� ���ֽ�������Ҫ �����תС���������ȷֵ
typedef struct udp_header
{
	USHORT srcport;   // Դ�˿�
	USHORT dstport;   // Ŀ�Ķ˿�
	USHORT total_len; // ����UDP��ͷ��UDP���ݵĳ���(��λ:�ֽ�)
	USHORT chksum;    // У���
}TP_UDP, * PTP_UDP;
//TCP �ṹ ���� ���ֽ�������Ҫ �����תС���������ȷֵ
typedef struct tp_tcp {
	unsigned short src_port;    //Դ�˿ں�
	unsigned short dst_port;    //Ŀ�Ķ˿ں�
	unsigned int   seq_no;      //���к�
	unsigned int   ack_no;      //ȷ�Ϻ�

	unsigned char reserved_1 : 4; //����6λ�е�4λ�ײ�����
	unsigned char thl : 4;    //tcpͷ������
	unsigned char flag : 6;  //6λ��־
	unsigned char reseverd_2 : 2; //����6λ�е�2λ

	unsigned short wnd_size;   //16λ���ڴ�С
	unsigned short chk_sum;    //16λTCP�����
	unsigned short urgt_p;     //16Ϊ����ָ��
}TP_TCP, * PTP_TCP;
//��������ͽṹ
typedef union transport_layer
{
	//���������������
	PTP_TCP tcp;
	PTP_UDP udp;

}Transport_Layer;

//-----------------------------������ؽṹ-----------------------------------------------------------------------------------------------------------------------------------------------

//���������İ�������Ϣ�����ڹ���
typedef struct packet_information {
	//�˽ṹ�����Ź��ܵ������Զ���

	UCHAR V4_Or_V6;//�ǹ���ipv4����ipv6  0:ipv4  1:ipv6
	union src_ip
	{
		PUCHAR ipv4;
		PUCHAR ipv6;
	}SRC_IP;
	union dst_ip
	{
		PUCHAR ipv4;
		PUCHAR ipv6;
	}DST_IP;
	USHORT src_port; //2�ֽ�С��8�ֽ���ֵ
	USHORT dst_port;
}Packet_Information, * PPacket_Information;
//��ȡ�Ĺ��˹���RING 3 ���룩
typedef struct filter_condition {
	//�˽ṹ�����Ź��ܵ������Զ��壬����������ͬ�ṹһ���

	UCHAR S_Or_R;//�ڷ���ʱ���˻����յ�ʱ���� 0������ ��1���յ�
	UCHAR S_Or_D;//�ǹ���Դ����Ŀ�� 0��Դ��1��Ŀ��
	UCHAR V4_Or_V6;//�ǹ���ipv4����ipv6  0:ipv4  1:ipv6
	union
	{
		UCHAR ipv4[4];
		UCHAR ipv6[16];
	}IP;
	USHORT port;
	int flag;//��ʶ�ţ�ɾ��ʱҪ��
	struct filter_condition* prior;
	struct filter_condition* next;
	//UCHAR flag[20];//��Ӧʹ�õĹ��˺�����1ʹ�ã�0��ʹ�ã�Ԥ��20�����˹�������λ��
}Filter_Condition, * PFilter_Condition;
//���˹�������
typedef struct fc_list {
	PFilter_Condition head;//�����һ��
	PFilter_Condition last;//�������һ��
	int num;
}FC_List;

FC_List FC_list = { 0 };//�����Ҫȫ�ֱ��������˹�������

//һ�����������ĵ�һ��������ʱ��Ϣ�����ڹ��˶Ա�
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//��¼���лỰ������¼�Է���ip
typedef struct record {
	UCHAR v4_or_v6;
	union 
	{
		UCHAR ipv4[4];
		UCHAR ipv6[16];
	}IP;
	struct record* next;
}Record,*PRecord;

typedef struct record_list {
	PRecord head;
	PRecord last;
	int num;
}Rocord_List,*PRocord_List;
//benign rocord list
Rocord_List B_Rrd_List = { 0 };
//malware rocord list
Rocord_List M_Rrd_List = { 0 };

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Network_Layer�Ƿ�Ϊ�գ����������δ����ʱ�����ؿ�

int Network_Layer_Is_Space(Network_Layer LS) {
	PUCHAR AD = (PUCHAR)&LS;
	for (int i = 0; i < sizeof(Network_Layer); i++)
	{
		if (AD[i] != 0x00)
		{
			return 0;
		}
	}
	return 1;
}
//Network_Layer�Ƿ�Ϊ�գ����ô����δ����ʱ�����ؿ�
int Transport_Layer_Is_Space(Transport_Layer LS) {
	PUCHAR AD = (PUCHAR)&LS;
	for (int i = 0; i < sizeof(Transport_Layer); i++)
	{
		if (AD[i] != 0x00)
		{
			return 0;
		}
	}
	return 1;
}

//----------------------------------Э��ջ���-----------------------------------------------------------------------------------------------------------------------------
//ͨ��NET_BUFFER��ȡһ������������,��PPacket��Ҫ�ͷţ������������
PPacket IC_Get_Packer(PNET_BUFFER nb) {
	PMDL Mdl = nb->CurrentMdl;//ȡ��ǰ
	ULONG packet_size = nb->DataLength;//�˰��Ĵ�С
	PPacket packet = (PPacket)ExAllocatePool(NonPagedPool, packet_size);//�����ŵĿռ�
	//ʧ��
	if (packet == NULL)
	{
		DbgPrint("packet�����ڴ�ʧ��\n");
		return NULL;
	}
	PPacket packetoffset = packet;
	ULONG offset = nb->CurrentMdlOffset;//ƫ��
	ULONG i = 0;
	do {
		ULONG cinnum = 0;
		PUCHAR OEP;
		ULONG thisMDLsize = Mdl->ByteCount;
		if (i == 0 && packet_size > (thisMDLsize - offset))//������offset>thismdlsize�����
		{
			//��һ��MDL���Ҵ�MDLװ���������İ���ʱ��
			cinnum = thisMDLsize - offset;
			OEP = (PUCHAR)Mdl->MappedSystemVa + offset;
			i++;
		}
		else if (i == 0)
		{
			//��һ��MDL����MDL����װ��һ�������İ�
			cinnum = packet_size;
			OEP = (PUCHAR)Mdl->MappedSystemVa + offset;
		}
		else {
			//���ڶ�����������������MDL������
			cinnum = thisMDLsize;
			OEP = (PUCHAR)Mdl->MappedSystemVa;
		}
		//cinnum = Mdl->ByteCount;
		memcpy(packetoffset, OEP, cinnum);
		packetoffset += cinnum;
		Mdl = Mdl->Next;
	} while (Mdl != NULL);//�������packet����Ϊ��������
	return packet;
}
//����������·�㲿������
PTP_ETHERNET IC_Get_Data_Link_Layer(PPacket packet)
{
	PTP_ETHERNET LS =  (PTP_ETHERNET)packet;
	/*if (packet == NULL)
		return LS;*/
	return LS;
}
//��������㲿������
Network_Layer IC_Get_Network_Layer(PPacket packet, PTP_ETHERNET data_link, PPacket_Information LSPI)
{
	Network_Layer LLS = { 0 };
	switch (data_link->ether_type)
	{
	case TYPE_IPV4:
	{
		IP_V4 LS;
		LS.Src_add = packet + 26;//ԴIP
		LS.Dst_add = packet + 30;//Ŀ��IP
		LS.Protocol = packet + 23;//�ϲ�Э������
		UCHAR LENG;
		memcpy(&LENG, packet + 14, 0x1);
		LENG &= 0x0f;
		ULONG L = 0;
		memcpy(&L, &LENG, 1);
		LS.Length = L * 0x4;
		LLS.ipv4 = LS;

	
		LSPI->V4_Or_V6 = 0;
		LSPI->SRC_IP.ipv4 = LS.Src_add;
		LSPI->DST_IP.ipv4 = LS.Dst_add;

		


		return LLS;
	}
	case TYPE_IPV6:
	{
		IP_V6 LS;
		LS.Length = 40;//ipv6ͷ�̶�40�ֽ�
		LS.Protocol = packet + 20;//�ϲ�Э������
		LS.Src_add = packet + 22;//ԴIP
		LS.Dst_add = packet + 26 + 16;//Ŀ��IP
		LLS.ipv6 = LS;


		LSPI->V4_Or_V6 = 1;
		LSPI->SRC_IP.ipv6 = LS.Src_add;
		LSPI->DST_IP.ipv6 = LS.Dst_add;

		
		return LLS;
	}
	default:
		return LLS;
		break;
	}


}
//���ش���㲿������
Transport_Layer IC_Get_Transport_Layer(PPacket packet, PTP_ETHERNET data_link, Network_Layer network_data, PPacket_Information LSPI) {
	Transport_Layer LLS = { 0 };
	PUCHAR lpacket = packet;
	lpacket += 14;//����������·��
	if (Network_Layer_Is_Space(network_data))//�Ƿ�Ϊδ����������
		return LLS;
	switch (data_link->ether_type)//��ͬ�������Э�飬Ҫ���ϵ�ƫ�Ʋ�ͬ
	{
	case TYPE_IPV4://ȷ����ipv4Э��
	{
		lpacket += network_data.ipv4.Length;//���������
		switch (*network_data.ipv4.Protocol)
		{
		case PROTOCOL_TCP:
		{
			LLS.tcp = (PTP_TCP)lpacket;

			LSPI->src_port = LLS.tcp->src_port;
			LSPI->dst_port = LLS.tcp->dst_port;


			break;
		}
		case PROTOCOL_UDP:
		{
			LLS.udp = (PTP_UDP)lpacket;

			LSPI->src_port = LLS.udp->srcport;
			LSPI->dst_port = LLS.udp->dstport;


			break;
		}
		default://δ����Ĵ���㣬���ؿ�
			break;
		}

		break;
	}
	case TYPE_IPV6://ȷ����ipv6Э��
	{
		lpacket += network_data.ipv6.Length;//���������
		switch (*network_data.ipv6.Protocol)
		{
		case PROTOCOL_TCP:
		{
			LLS.tcp = (PTP_TCP)lpacket;

			LSPI->src_port = LLS.tcp->src_port;
			LSPI->dst_port = LLS.tcp->dst_port;


			break;
		}
		case PROTOCOL_UDP:
		{
			LLS.udp = (PTP_UDP)lpacket;

			LSPI->src_port = LLS.udp->srcport;
			LSPI->dst_port = LLS.udp->dstport;


			break;
		}
		default://δ����Ĵ���㣬���ؿ�
			break;
		}
		break;
	}
	default://δ���������㣬���ؿ�
		break;
	}


	return LLS;
}

//----------------------------------�������----------------------------------------------------------------------------------------------------------------------------
//����ר��CALL
void TEST(PNET_BUFFER_LIST pnbl) {
	PNET_BUFFER pnb = NET_BUFFER_LIST_FIRST_NB(pnbl);
	/*if (pnb->Next == NULL)
		return;*/
	Packet_Information LSPI = { 0 };//��ȡ�˰�����ʱ��Ϣ
	PPacket packet = IC_Get_Packer(pnb);
	PTP_ETHERNET PET = IC_Get_Data_Link_Layer(packet);
	Network_Layer NL = IC_Get_Network_Layer(packet, PET,&LSPI);
	Transport_Layer TL = IC_Get_Transport_Layer(packet, PET, NL,&LSPI);
	DbgBreakPoint();
	ExFreePool(packet);
}
//����ʱ���˺���������0��ʱ�Ź�������1��ʱ����
int Send_Filtering_Function(IN PNET_BUFFER_LIST pnbl,OUT PPacket* Need_Free,IN OUT PPacket_Information LSPI) {
	if (FC_list.num == 0)//�ж��Ƿ���ڹ���
		return 0;
	//Packet_Information LSPI = { 0 };//��ȡ�˰�����ʱ��Ϣ
	//DbgBreakPoint();
	PNET_BUFFER pnb = NET_BUFFER_LIST_FIRST_NB(pnbl);

	PPacket packet = IC_Get_Packer(pnb);
	if (packet == NULL)
		return 0;
	*Need_Free = packet;
	PTP_ETHERNET PET = IC_Get_Data_Link_Layer(packet);
	Network_Layer NL = IC_Get_Network_Layer(packet, PET,LSPI);
	Transport_Layer TL = IC_Get_Transport_Layer(packet, PET, NL,LSPI);
	int bj = 0;//bj=1:�˰����Ϲ��˹����е�ip��ַ����һ����Ҫ�ж϶˿ں��Ƿ����
	PFilter_Condition FCLS = NULL;
	if (LSPI->DST_IP.ipv4 == NULL || LSPI->SRC_IP.ipv4 == NULL )
		return 0;//����㲻��ipЭ�飬����
	for (FCLS = FC_list.head; FCLS != NULL; FCLS = FCLS->next)
	{
		if (FCLS->S_Or_R == 1)
			continue;//˵��������򲻹��ҹ�,ȡ��һ������

		//if (FCLS->S_Or_R == 2)//���̶˿ڹ��� 
		//{
		//	PUCHAR PP1 = (PUCHAR) & (FCLS->port) + 1;
		//	PUCHAR PP2 = (PUCHAR) & (LSPI->src_port);
		//	if (*PP1 == *PP2)
		//		if (*(PP1 - 1) == *(PP2 + 1))
		//			return 1;//˵���˿��������
		//}
		switch (FCLS->S_Or_D) {
		case 0://����Դ
		{
			//����ip��bj=1˵��ip���ţ���һ����Ҫ�ж϶˿�
			switch (FCLS->V4_Or_V6)
			{
			case 0://IPV4
			{
				int i = 0;
				for (i = 0; i < IPV4_LENGTH; i++)
					if (FCLS->IP.ipv4[i] != LSPI->SRC_IP.ipv4[i])
						break;//��ƥ��
				if (i < IPV4_LENGTH)
					continue;//��ƥ��
				bj = 1;//˵�������ipƥ�䣬��һ����֤�˿��Ƿ�ƥ��
				break;

			}
			case 1://IPV6
			{
				int i = 0;
				for (i = 0; i < IPV6_LENGTH; i++)
					if (FCLS->IP.ipv6[i] != LSPI->SRC_IP.ipv6[i])
						break;
				if (i < IPV6_LENGTH)
					continue;//��ƥ��
				bj = 1;
				break;//˵�������ƥ����Ҫ����
			}
			default: // ���۲������������
				return 0;
			}
			//���˶˿�(���ﲻ��ֱ�ӶԱ�USHORT�ͣ���Ϊ����һ��)
			if (bj = 1 && FCLS->port == 0)
				return 1;//�������ip�����ж˿�
			if (bj = 1 && FCLS->port != 0 && LSPI->src_port !=0x00)
			{
				PUCHAR PP1 = (PUCHAR) & (FCLS->port) + 1;
				PUCHAR PP2 = (PUCHAR) & (LSPI->src_port);
				if (*PP1 == *PP2)
					if (*(PP1 - 1) == *(PP2 + 1))
						return 1;//˵���˿��������
			}
			bj = 0;

			continue;
		}
		case 1://����Ŀ��
		{
			//����ip
			switch (FCLS->V4_Or_V6)
			{
			case 0://IPV4
			{
				int i = 0;
				for (i = 0; i < IPV4_LENGTH; i++)
					if (FCLS->IP.ipv4[i] != LSPI->DST_IP.ipv4[i])
						break;
				if (i < IPV4_LENGTH)
					continue;//��ƥ��
				bj = 1;
				break;

			}
			case 1://IPV6
			{
				int i = 0;
				for (i = 0; i < IPV6_LENGTH; i++)
					if (FCLS->IP.ipv6[i] != LSPI->DST_IP.ipv6[i])
						break;
				if (i < IPV6_LENGTH)
					continue;//��ƥ��
				bj = 1;//˵�������ƥ����Ҫ����
				break;
			}
			default: // ���۲������������,���ǿ���չ���������Э��
				return 0;
			}
			//���˶˿�
			if (bj = 1 && FCLS->port == 0)
				return 1;//�������ip�����ж˿�
			if (bj = 1 && FCLS->port != 0 && LSPI->dst_port != 0x00)
			{
				PUCHAR PP1 = (PUCHAR) & (FCLS->port) + 1;
				PUCHAR PP2 = (PUCHAR) & (LSPI->dst_port);
				if (*PP1 == *PP2)
					if (*(PP1 - 1) == *(PP2 + 1))
						return 1;//˵���˿��������
			}
			bj = 0;
			continue;
		}
		default://���̹��� 
			if (bj == 0 && LSPI->src_port != 0 && FCLS->port != 0)
			{
				PUCHAR PP1 = (PUCHAR) & (FCLS->port) + 1;
				PUCHAR PP2 = (PUCHAR) & (LSPI->src_port);//�����ط���ʱ���������̶˿�һ��������Դ�˿�
				if (*PP1 == *PP2)
					if (*(PP1 - 1) == *(PP2 + 1))
						return 1;//˵���˿��������
			}
			continue;
		}
	}


	return 0;



}
//����ʱ���˺���������0��ʱ�Ź�������1��ʱ����
int Rec_Filtering_Function(IN PNET_BUFFER_LIST pnbl,OUT PPacket* Need_Free,IN OUT PPacket_Information RLSPI) {
	if (FC_list.num == 0)//�ж��Ƿ���ڹ���
		return 0;
	//Packet_Information RLSPI = { 0 };
	PNET_BUFFER pnb = NET_BUFFER_LIST_FIRST_NB(pnbl);
	/*if (pnb->Next == NULL)
		return;*/
	PPacket packet = IC_Get_Packer(pnb);
	if (packet == NULL)
		return 0;
	*Need_Free = packet;
	PTP_ETHERNET PET = IC_Get_Data_Link_Layer(packet);
	Network_Layer NL = IC_Get_Network_Layer(packet, PET,RLSPI);
	Transport_Layer TL = IC_Get_Transport_Layer(packet, PET, NL, RLSPI);
	int bj = 0;
	PFilter_Condition FCLS = NULL;
	if (RLSPI->DST_IP.ipv4 == NULL || RLSPI->SRC_IP.ipv4 == NULL )
		return 0;//����㲻��ipЭ�飬����
	for (FCLS = FC_list.head; FCLS != NULL; FCLS = FCLS->next)
	{
		if (FCLS->S_Or_R == 0)
			continue;//˵��������򲻹��ҹ�,ȡ��һ������

		//if (FCLS->S_Or_R == 2)//���̶˿ڹ��� 
		//{
		//	PUCHAR PP1 = (PUCHAR) & (FCLS->port) + 1;
		//	PUCHAR PP2 = (PUCHAR) & (RLSPI->dst_port);//���յ��İ���˵��Ŀ�Ķ˿ھ������Ǳ������̶˿�
		//	if (*PP1 == *PP2)
		//		if (*(PP1 - 1) == *(PP2 + 1))
		//			return 1;//˵���˿��������
		//}
		switch (FCLS->S_Or_D) {
		case 0://����Դ
		{
			//����ip��bj=1˵��ip���ţ���һ����Ҫ�ж϶˿�
			switch (FCLS->V4_Or_V6)
			{
			case 0://IPV4
			{
				int i = 0;
				for (i = 0; i < IPV4_LENGTH; i++)
					if (FCLS->IP.ipv4[i] != RLSPI->SRC_IP.ipv4[i])
						break;
				if (i < IPV4_LENGTH)
					continue;//��ƥ��
				bj = 1;//˵�������ipƥ�䣬��һ����֤�˿��Ƿ�ƥ��
				break;

			}
			case 1://IPV6
			{
				int i = 0;
				for (i = 0; i < IPV6_LENGTH; i++)
					if (FCLS->IP.ipv6[i] != *(RLSPI->SRC_IP.ipv6 + i))
						break;
				if (i < IPV6_LENGTH)
					continue;//��ƥ��
				bj = 1;
				break;//˵�������ƥ����Ҫ����
			}
			default: // ���۲������������
				return 0;
			}
			//���˶˿�(���ﲻ��ֱ�ӶԱ�USHORT�ͣ���Ϊ����һ��)
			if (bj = 1 && FCLS->port == 0)
				return 1;//�������ip�����ж˿�
			if (bj = 1 && FCLS->port != 0 && RLSPI->src_port != 0)
			{
				PUCHAR PP1 = (PUCHAR) & (FCLS->port) + 1;
				PUCHAR PP2 = (PUCHAR) & (RLSPI->src_port);
				if (*PP1 == *PP2)
					if (*(PP1 - 1) == *(PP2 + 1))
						return 1;//˵���˿��������
			}
			bj = 0;

			continue;
		}
		case 1://����Ŀ��
		{
			//����ip
			switch (FCLS->V4_Or_V6)
			{
			case 0://IPV4
			{
				int i = 0;
				for (i = 0; i < IPV4_LENGTH; i++)
					if (FCLS->IP.ipv4[i] != RLSPI->DST_IP.ipv4[i])
						break;
				if (i < IPV4_LENGTH)
					continue;//��ƥ��
				bj = 1;
				break;

			}
			case 1://IPV6
			{
				int i = 0;
				for (i = 0; i < IPV6_LENGTH; i++)
					if (FCLS->IP.ipv6[i] != RLSPI->DST_IP.ipv6[i])
						break;
				if (i < IPV6_LENGTH)
					continue;//��ƥ��
				bj = 1;//˵�������ƥ����Ҫ����
				break;
			}
			default: // ���۲������������
				return 0;
			}
			//���˶˿�
			if (bj = 1 && FCLS->port == 0)
				return 1;//�������ip�����ж˿�
			if (bj = 1 && FCLS->port != 0 && RLSPI->dst_port != 0)
			{
				PUCHAR PP1 = (PUCHAR) & (FCLS->port) + 1;
				PUCHAR PP2 = (PUCHAR) & (RLSPI->dst_port);
				if (*PP1 == *PP2)
					if (*(PP1 - 1) == *(PP2 + 1))
						return 1;//˵���˿��������
			}
			bj = 0;
			continue;
		}
		default://���̹��� 
			if (bj == 0 && FCLS->port != 0 && RLSPI->dst_port != 0)//�հ�ʱ���أ����̶˿�һ����Ŀ�Ķ˿�
			{
				//DbgBreakPoint();
				PUCHAR PP1 = ((PUCHAR) & (FCLS->port)) + 1;
				PUCHAR PP2 = (PUCHAR) & (RLSPI->dst_port);
				if (*PP1 == *PP2)
					if (*(PP1 - 1) == *(PP2 + 1))
						return 1;//˵���˿��������
			}
			continue;
		}
	}


	return 0;



}

//----------------------------------�Ự���----------------------------------------------------------------------------------------------------------------------------
//����ºͻỰ�б�
VOID Clear_B_Rrd_List()
{
	PRecord free = B_Rrd_List.head;
	PRecord next_free = NULL;
	KeAcquireSpinLock(&IC_LOCK2, &IC_IRQL2);//����
	B_Rrd_List.num = 0;
	B_Rrd_List.head = B_Rrd_List.last = NULL;
	KeReleaseSpinLock(&IC_LOCK2, IC_IRQL2);//����
	for (; free != NULL; free = next_free)
	{
		next_free = free->next;
		ExFreePool(free);
	}
}
//��ն���Ự�б�
VOID Clear_M_Rrd_List()
{
	PRecord free = M_Rrd_List.head;
	PRecord next_free = NULL;
	KeAcquireSpinLock(&IC_LOCK3, &IC_IRQL3);//����
	M_Rrd_List.num = 0;
	M_Rrd_List.head = M_Rrd_List.last = NULL;
	KeReleaseSpinLock(&IC_LOCK3, IC_IRQL3);//����
	for (; free != NULL; free = next_free)
	{
		next_free = free->next;
		ExFreePool(free);
	}
}

//���Ӷ���Ự
int M_NEW_Record(PPacket_Information RLSPI) {
	
	PRecord PNew_record = ExAllocatePool(NonPagedPool, sizeof(Record));
	if (PNew_record == NULL)
	{
		DbgBreakPoint();
		return 0;
	}
	memset(PNew_record, 0, sizeof(Record));
	PNew_record->v4_or_v6 = RLSPI->V4_Or_V6;
	//DbgBreakPoint();
	switch (RLSPI->V4_Or_V6)
	{
	case 0:
	{
		for (int i = 0; i < IPV4_LENGTH; i++)
			PNew_record->IP.ipv4[i] = RLSPI->SRC_IP.ipv4[i];//��¼ip
	}
	case 1:
	{
		for (int i = 0; i < IPV6_LENGTH; i++)
			PNew_record->IP.ipv6[i] = RLSPI->SRC_IP.ipv6[i]; //��¼ip
	}
	default:
		break;
	}
	KeAcquireSpinLock(&IC_LOCK3, &IC_IRQL3);//����
	if (M_Rrd_List.last != NULL)
	{
		M_Rrd_List.last->next = PNew_record;
		M_Rrd_List.last = PNew_record;
	}
	else
		M_Rrd_List.head = M_Rrd_List.last = PNew_record;
	M_Rrd_List.num++;
	KeReleaseSpinLock(&IC_LOCK3, IC_IRQL3);//����
	return 1;
}
//��¼�Ự���հ���
int Rec_Record_IP(PPacket_Information RLSPI) {
	if (RLSPI->DST_IP.ipv4 ==NULL )
		return 0;
	PRecord LS = NULL;
	//�ж��Ƿ����� �ºͻỰ�б�
	for (LS = B_Rrd_List.head; LS != NULL; LS = LS->next)
	{
		if (LS->v4_or_v6 != RLSPI->V4_Or_V6)
			continue;
		switch (RLSPI->V4_Or_V6)
		{
		case 0:
		{
			int i = 0;
			for (i = 0; i < IPV4_LENGTH; i++)
				if (LS->IP.ipv4[i] != RLSPI->SRC_IP.ipv4[i])
					break;//��ƥ��
			if (i == IPV4_LENGTH)
				return 0;
			break;
		}
		case 1:
		{
			int i = 0;
			for (i = 0; i < IPV6_LENGTH; i++)
				if (LS->IP.ipv6[i] != RLSPI->SRC_IP.ipv6[i])
					break;//��ƥ��
			if (i == IPV6_LENGTH)
				return 0;
			break;
		}
		default:
			break;
		
		}
	}
	//�ж��Ƿ����� ����Ự�б�
	for (LS =  M_Rrd_List.head; LS != NULL; LS = LS->next)
	{
		if (LS->v4_or_v6 != RLSPI->V4_Or_V6)
			continue;
		switch (RLSPI->V4_Or_V6)
		{
		case 0:
		{
			int i = 0;
			for (i = 0; i < IPV4_LENGTH; i++)
				if (LS->IP.ipv4[i] != RLSPI->SRC_IP.ipv4[i])
					break;//��ƥ��
			if (i == IPV4_LENGTH)
				return 0;
			break;
		}
		case 1:
		{
			int i = 0;
			for (i = 0; i < IPV6_LENGTH; i++)
				if (LS->IP.ipv6[i] != RLSPI->SRC_IP.ipv6[i])
					break;//��ƥ��
			if (i == IPV6_LENGTH)
				return 0;
			break;
		}
		default:
			break;

		}
	}
	M_NEW_Record(RLSPI);
	return 1;
}
//��¼�ºͻỰ
int B_New_Record(PPacket_Information LSPI)
{
	
	PRecord PNew_record = ExAllocatePool(NonPagedPool, sizeof(Record));
	if (PNew_record == NULL)
	{
		DbgBreakPoint();
		return 0;
	}
	memset(PNew_record, 0, sizeof(Record));
	PNew_record->v4_or_v6 = LSPI->V4_Or_V6;
	//DbgBreakPoint();
	switch (LSPI->V4_Or_V6)
	{
	case 0:
	{
		for (int i = 0; i < IPV4_LENGTH; i++)
			PNew_record->IP.ipv4[i] = LSPI->DST_IP.ipv4[i]; 
	}
	case 1:
	{
		for (int i = 0; i < IPV6_LENGTH; i++)
			PNew_record->IP.ipv6[i] = LSPI->DST_IP.ipv4[i];
	}
	default:
		break;
	}
	KeAcquireSpinLock(&IC_LOCK2, &IC_IRQL2);//����
	if (B_Rrd_List.last != NULL)
	{
		B_Rrd_List.last->next= PNew_record;
		B_Rrd_List.last = PNew_record;
	}
	else
		B_Rrd_List.head = B_Rrd_List.last = PNew_record;
	B_Rrd_List.num++;
	KeReleaseSpinLock(&IC_LOCK2, IC_IRQL2);//����
	return 1;
	
}
//��¼�Ự��������
int Send_Record_IP(PPacket_Information LSPI) {
	if (LSPI->DST_IP.ipv4 == NULL)
		return 0;
	PRecord LS = NULL;
	//�ж��Ƿ����� �ºͻỰ�б�
	for (LS = B_Rrd_List.head; LS != NULL; LS = LS->next)
	{
		if (LS->v4_or_v6 != LSPI->V4_Or_V6)
			continue;
		switch (LSPI->V4_Or_V6)
		{
		case 0:
		{
			int i = 0;
			for (i = 0; i < IPV4_LENGTH; i++)
				if (LS->IP.ipv4[i] != LSPI->DST_IP.ipv4[i])
					break;//��ƥ��
			if (i == IPV4_LENGTH)
				return 0;
			break;
		}
		case 1:
		{
			int i = 0;
			for (i = 0; i < IPV6_LENGTH; i++)
				if (LS->IP.ipv6[i] != LSPI->DST_IP.ipv6[i])
					break;//��ƥ��
			if (i == IPV6_LENGTH)
				return 0;
			break;
		}
		default:
			break;

		}
	}
	//�ж��Ƿ����� ����Ự�б�
	for (LS = M_Rrd_List.head; LS != NULL; LS = LS->next)
	{
		if (LS->v4_or_v6 != LSPI->V4_Or_V6)
			continue;
		switch (LSPI->V4_Or_V6)
		{
		case 0:
		{
			int i = 0;
			for (i = 0; i < IPV4_LENGTH; i++)
				if (LS->IP.ipv4[i] != LSPI->DST_IP.ipv4[i])
					break;//��ƥ��
			if (i == IPV4_LENGTH)
				return 0;
			break;
		}
		case 1:
		{
			int i = 0;
			for (i = 0; i < IPV6_LENGTH; i++)
				if (LS->IP.ipv4[i] != LSPI->DST_IP.ipv4[i])
					break;//��ƥ��
			if (i == IPV6_LENGTH)
				return 0;
			break;
		}
		default:
			break;

		}
	}
	if (B_Rrd_List.num > 100)//�ºͼ�¼ֻ��¼100����Ȼ�����¼���
		Clear_B_Rrd_List();
	B_New_Record(LSPI);
	return 1;
}





//#endif