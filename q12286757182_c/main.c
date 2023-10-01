//C言語サンプルプログラム (Visual Studio 2013～2022でビルド可)
#define	_CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "user.h"

// このプログラムの特徴
//	入力内容が正しくないときに再入力を受け付ける。
//  CSV ファイルの内容を厳密に走査して読み込む。
//  CSV ファイルの内容に問題があるときは行番号と列名を含むエラーメッセージを表示する。
//
// このプログラムで学習する方へのコメント
//	再入力を受け付ける処理は特殊であり、やり方を覚える必要はない。（そういうやり方もあると見ておくだけで充分）
//	CSV ファイルの走査部分も特殊だが、上級者であれば内容を理解できるようにするのが望ましい。（初心者の場合は難しいと思われる）
//	入門レベルの方は関数の分け方を参考にしていただくと良い。
//	配列とポインタの扱いに慣れていない場合は、これよりも簡単な題材で訓練してから取り組むとやり易くなる筈。
//	このプログラムで配列とポインタに入門するのはかなり大変（あるいは殆ど無理）だと考える。
//	～変数について～
//	変数の宣言・定義はC99で自由な位置に書けるようになった。
//	しかし、今でも関数の冒頭部分に置かれているケースが初心者のコードで多く見られる。
//	変数が増えてくると、遠いところにある変数の挙動が把握しきれなくなってくる。
//	変数はできるだけ狭い範囲に閉じ込めることが大切である。
//	～構造体とメンバ名について～
//	知恵袋内のご質問に書かれてあった構造体名、メンバ名をそのまま使用したが、再考の余地がある。
//	名前フィールドは n、パスワードフィールドは p となっているがそれぞれ name, password あたりが良いだろう。
//	構造体名も member ということで、かなり一般的な名前となっている。
//	プログラムを作る際、構造体名で一般的な名前というのは避けたほうが良い。（できるだけ固有な名前にすることが大切）
//

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
		switch (scanf(format, buf, lf)) {
		case 1:
			//改行が受け取れなかったので入力内容が長すぎたためであると判断できる。
			if (scanf("%*[^\n]%*c") == 0) {
				printf("入力された文字列は長すぎました。%u バイト以内の文字列を入力してください。\n", max_length);
				continue;
			}
			else {
				//1行分の読み捨てが失敗したので入力受付の続行は無理と判断できる。
				fputs("scanfが失敗しました。", stderr);
				exit(EXIT_FAILURE);
			}
		case 2:
			//正常
			return;
		default:
			//EOF発生が想定される。
			fputs("scanfが失敗しました。", stderr);
			exit(EXIT_FAILURE);
		}
	}
}

//番号入力
int input_number()
{
	for (;;) {
		printf("> ");
		int number;
		if (scanf("%d", &number) == 1)
			return number;
		else if (scanf("%*[^\n]") == 0)
			continue;
		else {
			fputs("scanfが失敗しました。", stderr);
			exit(EXIT_FAILURE);
		}
	}
}

//該当する名前のメンバーを探す
member* find_member_by_name(member* first, member* last, const char name[])
{
	for (;;) {
		if (first < last) {
			if (strcmp(first->n, name) == 0)
				return first;
			else
				++first;
		}
		else
			return NULL;
	}
}

//該当するIDのメンバーを探す
member* find_member_by_id(member* first, member* last, const char id[])
{
	for (;;) {
		if (first < last) {
			if (strcmp(first->id, id) == 0)
				return first;
			else
				++first;
		}
		else
			return NULL;
	}
}

//新規メンバー固有名の入力を受け付ける（他のメンバーと同じ名前は不可）
void input_unique_name(member* first, member* last, char name[], unsigned char sizeof_name)
{
	for (;;) {
		input_limited(name, sizeof_name);
		if (find_member_by_name(first, last, name)) {
			printf("登録済みです。\nもう一度入力してください。\n\n");
			continue;
		}
		else
			return;
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
				else {
					fprintf(stderr, "%s(%d): ファイル中の %s は長すぎました。\n", filename, linenumber, pf->name);
					exit(EXIT_FAILURE);
				}
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
				else {
					fprintf(stderr, "%s(%d): ファイル中の %s 列の次にデータがありませんでした。\n", filename, linenumber, pf->name);
					exit(EXIT_FAILURE);
				}
			case ',':
				if (pf - fields == _countof(fields) - 1) {
					fprintf(stderr, "%s(%d): ファイル中の %s 列の次に過剰なコンマがありました。\n", filename, linenumber, pf->name);
					exit(EXIT_FAILURE);
				}
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
		if (e) {
			fputs("ファイルに保存できませんでした。\n", stderr);
			exit(EXIT_FAILURE);
		}
		else
			return;
	}
	else {
		fputs("保存先ファイルを開くことができませんでした。\n", stderr);
		exit(EXIT_FAILURE);
	}
}

//新しいメンバーを追加する
void add_new_member(member* first, member* last, const char* filename)
{
	member* const new_person = find_member_by_name(first, last, "");
	if (new_person) {
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
	else
		printf("これ以上登録できません\n\n");
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
	member* const logged_in = find_member_by_id(first, last, id);
	if (logged_in && strcmp(logged_in->p, password) == 0) {
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
	printf("ログイン失敗です。\n\n");
}

int main()
{
	static const char filename[] = "user_info.csv";
	member member_info[5] = { 0 };
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
