#pragma once
#define KAI 1 //����o�^      
#define LOG 2 //���O�C��      
#define OUT 3 //���O�A�E�g    
#define TAI 4 //�މ�          
#define YOY 5 //�\��          
#define CAN 6 //�\��폜      
#define KAK 7 //�\��m�F      

struct member {
	char id[ 8]; //���ID    
	char n [31]; //����      
	char p [16]; //�p�X���[�h
};
#ifndef __cplusplus
typedef struct member member;
#endif
