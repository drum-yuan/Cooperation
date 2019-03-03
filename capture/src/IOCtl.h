#pragma once

#define CHANGE_QUEUE_SIZE          50000

// op_type
#define  OP_TEXTOUT        1
#define  OP_BITBLT         2
#define  OP_COPYBITS       3
#define  OP_STROKEPATH     4
#define  OP_LINETO         5
#define  OP_FILLPATH       6
#define  OP_STROKEANDFILLPATH  7
#define  OP_STRETCHBLT     8
#define  OP_ALPHABLEND     9
#define  OP_TRANSPARENTBLT 10
#define  OP_GRADIENTFILL   11
#define  OP_PLGBLT         12
#define  OP_STRETCHBLTROP  13
#define  OP_RENDERHINT     14

#define ESCAPE_CODE_MAP_USERBUFFER     0x0DAACC00
#define ESCAPE_CODE_UNMAP_USERBUFFER   0x0DAACC10

struct draw_change_t
{
	unsigned int    op_type; /// ����GDI����
	RECT            rect;    /// �����仯������
};

struct draw_queue_t
{
	unsigned int       next_index;                 /// ��һ����Ҫ���Ƶ�λ��, �� 0�� ( CHANGE_QUEUE_SIZE - 1 ),ѭ������ָʾ��
	draw_change_t      draw_queue[CHANGE_QUEUE_SIZE];    
};

struct draw_buffer_t
{
	char                magic[16];   
	unsigned int        cx;          ///��Ļ����
	unsigned int        cy;          ///��Ļ�߶�
	unsigned int        line_bytes;  ///ÿ��ɨ���е�ʵ�����ݳ���
	unsigned int        line_stride; ///ÿ��ɨ���е�4�ֽڶ�������ݳ���
	unsigned int        bitcount;    ///8,16,24,32 
	draw_queue_t        changes;     ///�����仯�ľ��ο򼯺�
	unsigned int        data_length; //data_bufferָ�����ĻRGB���ݳ��ȣ����� line_stride*cy
	char                data_buffer[1]; //�����Ļ��������
};


