#pragma once
#define KAI 1 //会員登録      
#define LOG 2 //ログイン      
#define OUT 3 //ログアウト    
#define TAI 4 //退会          
#define YOY 5 //予約          
#define CAN 6 //予約削除      
#define KAK 7 //予約確認      

struct member {
	char id[ 8]; //会員ID    
	char n [31]; //氏名      
	char p [16]; //パスワード
};
#ifndef __cplusplus
typedef struct member member;
#endif
