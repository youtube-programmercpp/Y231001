//C++サンプルプログラム
#define	_CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "../q12286757182_c/user.h"
#include <algorithm>
#include <stdexcept>
#include <string>
#include <iostream>


//文字数制限付きの入力
void input_limited(char buf[], unsigned char sizeof_buf)
{
	assert(sizeof_buf > sizeof "");//sizeof_buf は必ず 1 より大きい
	const unsigned char max_length = sizeof_buf - 1;//従って max_length は必ず 0 より大きい
	//scanf用の書式文字列を作る
	// "%123s%1[\n]" という形式で 123 の部分に max_length が入る
	// これでmax_length以内の文字列入力を受け付けることが出来る。
	// 長すぎる文字列が入力されると%123sのところで余剰文字列が残留し、
	// それが妨げとなって %1[\n] の入力が受け付けられない状況となる。
	// →結果として scanf は 1 を返す
	// 正常に入力されれば scanf は 2 を返す。
	char format[sizeof "%255s%1[\n]"];
	sprintf(format, "%%%us%%1[\n]", max_length);
	for (;;) {
		printf("> ");
		char lf[sizeof "\n"];
		switch (const auto retval1 = scanf(format, buf, lf)) {
		case 1:
			//改行が受け取れなかったので入力内容が長すぎたためであると判断できる。
			if (const auto retval2 = scanf("%*[^\n]%*c") == 0) {
				printf("入力された文字列は長すぎました。%u バイト以内の文字列を入力してください。\n", max_length);
				continue;
			}
			else {
				//1行分の読み捨てが失敗したので入力受付の続行は無理と判断できる。
				throw std::runtime_error(__FILE__ "(" _CRT_STRINGIZE(__LINE__) "): In " __FUNCTION__ ", scanf returned "+std::to_string(retval2));
			}
		case 2:
			//正常
			return;
		default:
			//EOF発生が想定される。
			throw std::runtime_error(__FILE__ "(" _CRT_STRINGIZE(__LINE__) "): In " __FUNCTION__ ", scanf returned " + std::to_string(retval1));
		}
	}
}

//番号入力
int input_number()
{
	for (;;) {
		printf("> ");
		int number;
		if (const auto retval1 = scanf("%d", &number) == 1)
			return number;
		else if (const auto retval2 = scanf("%*[^\n]") == 0)
			continue;
		else
			throw std::runtime_error(__FILE__ "(" _CRT_STRINGIZE(__LINE__) "): In " __FUNCTION__ ", scanf returned " + std::to_string(retval2));
	}
}

//新規メンバー固有名の入力を受け付ける（他のメンバーと同じ名前は不可）
void input_unique_name(member* first, member* last, char name[], unsigned char sizeof_name)
{
	for (;;) {
		input_limited(name, sizeof_name);
		if (std::find_if(first, last, [name](const member& r) noexcept {return strcmp(r.n, name) == 0; }) == last)
			return;
		else
		{
			printf("登録済みです。\nもう一度入力してください。\n\n");
			continue;
		}
	}
}
//ファイルから読み込む
void csv_load(member* first, member* last, const char* filename)
{
	assert(first < last);
	FILE* const fp = fopen(filename, "r");
	if (fp) {
		//列構造体
		struct field {
			const char*   name      ;//列の名前
			unsigned char offset    ;//列のオフセット
			unsigned char max_length;//列の最大長
		};
#define	Macro(name)	{#name, offsetof(member, name), sizeof ((member*)0)->name - 1},
		const struct field fields[] =
		{
			Macro(id)
			Macro(n )
			Macro(p )
		};
#undef	Macro
		const struct field* pf = fields;
		char* pch = (char*)first + pf->offset;
		for (int linenumber = 1;;) {
			const int ch = getc(fp);
			switch (ch) {
			default:
				if (pch < (char*)first + pf->offset + pf->max_length) {
					//次の文字に進む
					*pch++ = ch;
					continue;
				}
				else
					throw std::runtime_error(__FILE__ "(" _CRT_STRINGIZE(__LINE__) "): In " __FUNCTION__ ", too long field: " + std::string(pf->name) );
			case '\n':
				if (pf - fields == _countof(fields) - 1) {
					*pch = '\0';
					if (++first == last) {
						fclose(fp);
						return;
					}
					else {
						//次の行に進む
						++linenumber;
						pf = fields;
						pch = (char*)first + pf->offset;
						continue;
					}
				}
				else
					throw std::runtime_error(__FILE__ "(" _CRT_STRINGIZE(__LINE__) "): In " __FUNCTION__ ", no field next to: " + std::string(pf->name));
			case ',':
				if (pf - fields == _countof(fields) - 1)
					throw std::runtime_error(__FILE__ "(" _CRT_STRINGIZE(__LINE__) "): In " __FUNCTION__ ", extra comma next to: " + std::string(pf->name));
				else {
					//次の列に進む
					++pf;
					*pch = '\0';
					pch = (char*)first + pf->offset;
					continue;
				}
			}
		}
	}
}

//ファイルに書き出す
void csv_save(const member* first, const member* last, const char* filename)
{
	FILE* const fp = fopen(filename, "w");
	if (fp) {
		for (; first < last; ++first)
			fprintf(fp, "%s,%s,%s\n", first->id, first->n, first->p);
		const auto e = fflush(fp);
		fclose(fp);
		if (e)
			throw std::runtime_error(__FILE__ "(" _CRT_STRINGIZE(__LINE__) "): In " __FUNCTION__ ", fflush failed");
		else
			return;
	}
	else
		throw std::runtime_error(__FILE__ "(" _CRT_STRINGIZE(__LINE__) "): In " __FUNCTION__ ", fopen(\"" + std::string(filename) + "\") failed");
}

//新しいメンバーを追加する
void add_new_member(member* first, member* last, const char* filename)
{
	auto* const new_person = std::find_if(first, last, [](const member& r) noexcept {return *r.n == '\0'; });
	if (new_person == last) 
		printf("これ以上登録できません\n\n");
	else {
		char name    [sizeof ((member*)0)->n];
		char password[sizeof ((member*)0)->p];
		/*氏名を入力*/
		printf("登録したい氏名を入力してください。\n");
		input_unique_name(first, last, name, sizeof name);
		/*パスワード入力*/
		printf("次に、登録したいパスワードを入力してください。\n");
		input_limited(password, sizeof password);
		sprintf(new_person->id, "%d", (int)(new_person - first) + 1001);//ここでidを入れます。
		strcpy(new_person->n, name    );
		strcpy(new_person->p, password);
		printf("あなたのidは %s です\n", new_person->id);
		csv_save(first, last, filename);
		printf("登録が完了しました。\n");
		printf("\n");
	}
}

//ログイン処理とログイン中のコマンド受け付け＆実行処理
void login_session(member* first, member* last, const char* filename)
{
	char id      [sizeof((member*)0)->id];
	char password[sizeof((member*)0)->p ];
	printf("idを入力してください。\n");
	input_limited(id      , sizeof id      );
	printf("パスワードを入力してください。\n");
	input_limited(password, sizeof password);

	auto* const logged_in = std::find_if(first, last, [id](const member& r) noexcept {return strcmp(r.id, id) == 0; });
	if (logged_in != last && strcmp(logged_in->p, password) == 0) {
		printf("ログイン成功です。\n\n");
		for (;;) {
			printf("以下のいずれかを選択してください。\n");
			printf("3:ログアウト, 4:退会, 5:予約, 6:予約削除, 7:予約確認\n");
			switch (input_number()) {
			case OUT://ログアウト    
				printf("ログアウトしました。\n\n");
				return;
			case TAI://退会          
				printf("退会手続きをしてもよろしいでしょうか。\n");
				printf("1:yes, 2:no\n");
				if (input_number() == 1) {
					printf("退会しました。\n");
					//会員情報を削除
					*logged_in->id = *logged_in->n = *logged_in->p = '\0';
					csv_save(first, last, filename);
					return;
				}
				else
					continue;
			case YOY://予約          
			case CAN://予約削除
			case KAK://予約確認      
				printf("対応する機能はまだ実装されていません。\n\n");
				continue;
			default:
				continue;
			}
		}
	}
	else
		printf("ログイン失敗です。\n\n");
}

int main()
{
	static const char filename[] = "user_info.csv";
	member member_info[5] = {};
	try {
		csv_load(member_info, *((&member_info) + 1), filename);
		for (;;) {
			printf("以下のいずれかを選択してください。\n");
			printf("1:会員登録, 2:ログイン, 0: 終了\n");
			switch (input_number()) {
			case 0  : //終了
				return EXIT_SUCCESS;
			case KAI: //会員登録
				add_new_member(member_info, *((&member_info) + 1), filename);
				continue;
			case LOG: //ログイン
				login_session(member_info, *((&member_info) + 1), filename);
				continue;
			default:
				continue;
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
/*
https://detail.chiebukuro.yahoo.co.jp/qa/question_detail/q12286757182

1052126005さん
2023/9/29 23:51

構造体の中身を削除するプログラムを書きたいのですが、
できずに詰まっております。

例)
1001, NAME, PASS
1002, NAME, PASS
・・・
というように構造体に入っていたとします。
1001の会員IDでログインした場合、
[1001, NAME, PASS]のデータが構造体から削除するようなプログラムを作成したいです。どなたかよろしくお願いいたします。

https://ideone.com/q1XRaN
*/
