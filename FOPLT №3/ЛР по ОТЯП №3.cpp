// ЛР по ОТЯП №3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

#pragma comment(lib,"Winmm.lib")	
using namespace std;

const char BIG_EN_BEG = 65;			//Код буквы А в ASCII
const char BIG_EN_END = 90;			//Код буквы Z в ASCII
const char SMALL_EN_BEG = 97;		//Код буквы a в ASCII
const char SMALL_EN_END = 122;		//Код буквы z в ASCII
const int END_CONST = 32767;		//Правый диапазон константы
const int STR_LENGTH = 255;			//Допустимое количество символов в идентификаторе	
bool voice;

//ЛЕКСИЧЕСКИЙ АНАЛИЗАТОР
enum EState { S, Ai, Ac, As, Bs, Cs, Ds, Gs, E, F, Fe };	//Список состояний
enum ELexType { lFor, lTo, lNext, lAnd, lOr, lRel, lAs, lAo, lVar, lConst, lError };		//Список типов лексем

int lex_matrix[8][9]							/*			0	1	2	3	4	5	6	7	8*/
{												/*			S	Ai	Ac	As	Bs	Cs	Ds	Gs	E*/
	1,	1,	8,	10,	10,	10,	10,	10,	8,			//буквы		Ai	Ai	E	Fe	Fe	Fe	Fe	Fe	E
	2,	1,	2,	10,	10,	10,	10,	10,	8,			//цифры		Ac	Ai	Ac	Fe	Fe	Fe	Fe	Fe	E
	3,	10,	10,	10,	10,	10,	3,	10,	8,			//>			As	Fe	Fe	Fe	Fe	Fe	As	Fe	E
	6,	10,	10,	10,	10,	10, 10,	10,	8,			//<			Ds	Fe	Fe	Fe	Fe	Fe	Fe	Fe	E
	4,	10,	10,	10,	7,	10,	10,	10,	8,			//=			Bs	Fe	Fe	Fe	Gs	Fe	Fe	Fe	E
	5,	10,	10,	10,	10,	10,	10,	10,	8,			//-,+,*,/	Cs	Fe	Fe	Fe	Fe	Fe	Fe	Fe	E
	9,	9,	9,	9,	9,	9,	9,	9,	9,			//' ', '\n' F	F	F	F	F	F	F	F	F
	8,	8,	8,	8,	8,	8,	8,	8,	8			//остальные E	E	E	E	E	E	E	E	E
};

//Проверка на соответствие пробелу, переводу строки или каретки и т.д.
bool isempty(char currentChar)
{
	return currentChar == ' ' || currentChar == '\n' || currentChar == '\r' || currentChar == '\t';
}

//Проверка на соответствие арифметическому знаку
bool isarif(char currentChar)
{
	return currentChar == '-' || currentChar == '+' || currentChar == '*' || currentChar == '/';
}

//Функция преобразования символа в соответствующий номер строки матрицы переходов GetMatrixCol - ловля столбца матрицы
int LexGMC(char currentChar)
{
	if (currentChar >= BIG_EN_BEG && currentChar <= BIG_EN_END
		|| currentChar >= SMALL_EN_BEG && currentChar <= SMALL_EN_END) return 0;	//Возвращение строки букв
	else if (currentChar >= '0' && currentChar <= '9') return 1;					//Возвращение строки цифр
	else if (currentChar == '>')	  return 2;				//Возвращение строки знака >
	else if (currentChar == '<')	  return 3;				//Возвращение строки знака <
	else if (currentChar == '=')	  return 4;				//Возвращение строки знака =
	else if (isarif(currentChar))  return 5;				//Возвращение строки знака -,+,*,/
	else if (isempty(currentChar)) return 6;				//Возвращение строки "пустых" символов
	return 7;										//Возвращение строки остальных символов
}

//Контейнер лексемы
struct Lex {
	ELexType type;	//Тип лексемы
	string str;		//текст лексемы
};

//Функция определения типа лексемы и её запись 
void AddType(const string& str, vector<Lex>& lex, Lex& lexema, const EState& prevState, const int& firstPos, const int& position)
{
	lexema.str = str.substr(firstPos, position - firstPos);	//вычленение подстроки и запись в лексему
	if (prevState == E || prevState == Ai && lexema.str.length() > STR_LENGTH) {
		lexema.type = lError;
		lex.push_back(lexema);
		return;
	}
	switch (prevState) {
	case Ai: {
		if (lexema.str == "for") { lexema.type = lFor; cout << lexema.str << "           " << "Ключевое слово - for\n"; lex.push_back(lexema); return; }
		if (lexema.str == "to") { lexema.type = lTo;  cout << lexema.str << "           " << "Ключевое слово - to\n"; lex.push_back(lexema); return; }
		if (lexema.str == "next") { lexema.type = lNext; cout << lexema.str << "           " << "Ключевое слово - next\n"; lex.push_back(lexema); return; }
		if (lexema.str == "and") { lexema.type = lAnd; cout << lexema.str << "           " << "Ключевое слово - and\n"; lex.push_back(lexema); return; }
		if (lexema.str == "or") { lexema.type = lOr; cout << lexema.str << "           " << "Ключевое слово - or\n"; lex.push_back(lexema); return; }
		lexema.type = lVar;
		cout << lexema.str << "           " << "Идентификатор\n";
		lex.push_back(lexema);
		return;
	}
	case Ac: { lexema.type = lConst; cout << lexema.str << "           " << "Константа\n"; lex.push_back(lexema); return; }
	case As: case Gs: case Ds: { lexema.type = lRel; cout << lexema.str << "           " << "Операция сравнения\n"; lex.push_back(lexema); return; }
	case Bs: { lexema.type = lAs; cout << lexema.str << "           " << "Операция присваивания\n"; lex.push_back(lexema); return; }
	case Cs: { lexema.type = lAo; cout << lexema.str << "           " << "Арифметическая операция\n"; lex.push_back(lexema); return; }
	}
	//if (prevState == Ai) {
	//	if (lexema.str == "for") { lexema.type = lFor; cout << lexema.str << "           " << "Ключевое слово - for\n"; lex.push_back(lexema); return; }
	//	if (lexema.str == "to") { lexema.type = lTo;  cout << lexema.str << "           " << "Ключевое слово - to\n"; lex.push_back(lexema); return; }
	//	if (lexema.str == "next") { lexema.type = lNext; cout << lexema.str << "           " << "Ключевое слово - next\n"; lex.push_back(lexema); return; }
	//	if (lexema.str == "and") { lexema.type = lAnd; cout << lexema.str << "           " << "Ключевое слово - and\n"; lex.push_back(lexema); return; }
	//	if (lexema.str == "or") { lexema.type = lOr; cout << lexema.str << "           " << "Ключевое слово - or\n"; lex.push_back(lexema); return; }
	//	lexema.type = lVar;
	//	cout << lexema.str << "           " << "Идентификатор\n";
	//	lex.push_back(lexema);
	//	return;
	//}
	//else if (prevState == Ac) { lexema.type = lConst; cout << lexema.str << "           " << "Константа\n"; lex.push_back(lexema); return; }
	/*else if (prevState == As || prevState == Gs || prevState == Ds) { 
		lexema.type = lRel; 
		cout << lexema.str << "           " << "Операция сравнения\n"; 
		lex.push_back(lexema); 
		return; 
	}*/
	/*else if (prevState == Bs) { lexema.type = lAs; cout << lexema.str << "           " << "Операция присваивания\n"; lex.push_back(lexema); return; }
	else if (prevState == Cs) { lexema.type = lAo; cout << lexema.str << "           " << "Арифметическая операция\n"; lex.push_back(lexema); return; }*/
}

//Функция лексического анализа
void LexAnalysis(string str, vector<Lex>& lex)
{
	int position = 0;				//текущая позиция в строке
	EState state = S, prevState;	//текущее состояние
	int firstPos;					//позиция начала лексемы
	Lex lexema;						//текущая лексема
	cout << "Проверка лексическим анализатором:\n";

	for (; isempty(str[position]); position++);		//Пропуск начальных пустых символов
	for (; str[position] != '\0'; position++)		//Проход до конца текста
	{
		char currentChar = str[position];
		prevState = state;			//Предыдущее состояние

		if (state == S) {
			for (; isempty(str[position]); position++);		//Пропуск пустых символов при считывании новой лексемы 
			currentChar = str[position];
			firstPos = position;
		}

		int i = LexGMC(currentChar);	//Переход по матрице состояний лексического анализатора
		state = (EState)lex_matrix[i][state];

		if (state == F) {		//Запись текущей лексемы после пустых символов
			AddType(str, lex, lexema, prevState, firstPos, position);
			state = S;
		}
		if (state == Fe) {		//Запись текущей лексемы после специальных символов
			AddType(str, lex, lexema, prevState, firstPos, position);
			state = S;
			--position;
		}
	}

	return;
}
//_________________________________________________________________________________________________________________________________________
//СИНТАКСИЧЕСКИЙ АНАЛИЗАТОР
enum SState { Start, Stat, Stat1, AE, AE1, WT, WT1, WS,	WS1, WS2, err };	//Состояния для матрицы синтаксического анализатора

int syn_matrix[8][11]
{												//			Start Stat Stat1 AE	  AE1	WT	 WT1	WS	 WS1	WS2  err
	1,	10,	10,	10,	10, 10,	10, 10,	10, 10,	10,	//lFor		Stat  err  err	 err  err	err	 err	err	 err	err  Stat
	10,	10, 10,	10,	5,	10,	10, 10,	10, 10,	5,	//lTo		err	  err  err	 err  WT	err	 err	err	 err	err	 WT
	10,	10, 10,	10,	10,	10, 10,	10, 10,	0,	0,	//lNext		err	  err  err	 err  err	err	 err	err	 err	Start Start
	10,	10, 3,	10, 10,	10,	10,	8,	10,	10, 10,	//lAs (=)	err	  err  АВ	 err  err	err	 err	WS1	 err	err  err
	10,	10, 10,	10,	3,	10, 5,	10,	10,	8,	10,	//lAo(+-*/) err	  err  err	 err  АE	err	 WT	    err	 err	WS1  err
	10,	2,	10,	4,	10,	6,	7,	10,	9,	10,	10,	//lVar		err	  Stat1 err	 АE1  err	WT1	 WS	    err	 WS2	err  err
	10,	10,	10,	4,	10,	6,	10,	10,	9,	10,	10,	//lConst	err	  err  err	 АE1  err	WT1	 err	err	 WS2	err  err
	10,	10,	10,	10, 10,	10, 10,	10, 10,	10,	10	//Error		err	  err  err	 err  err	err	 err	err	 err	err	 err
};

//Функция вкл/выкл звукового сопровождения
void On_Off_voice()		
{
	string temp;
	cout << "Включить звуковое сопровождение? (Y-да, остальное-нет): ";
	getline(cin, temp);
	if (temp == "y" || temp == "Y") {
		voice = true;
		cout << "Звуковое сопровождение включено.\n";
	}
	else {
		voice = false;
		cout << "Звуковое сопровождение отключено.\n";
	}
	return;
}

//Функция преобразования типа лексемы в номер строки
int SynGMC(Lex& lexema)
{
	switch (lexema.type) {
	case lFor: { return 0; }	//Возвращение строки lFor
	case lTo: { return 1; }		//Возвращение строки lTo
	case lNext: { return 2; }	//Возвращение строки lNext
	case lAs: { return 3; }		//Возвращение строки lAs
	case lAo: { return 4; }		//Возвращение строки lAo
	case lVar: { return 5; }	//Возвращение строки lVar
	case lConst: { return 6; }	//Возвращение строки lConst
	}
	return 7;		//Возвращение строки Error (сюда входят все остальные типы, включая lError)
}

//Функция оповещения об ошибке
void CheckError(const SState& prevstate, const Lex& lexema)	
{
	if (lexema.type == lError) { cout << "\n(ОШИБКА!!! Найдена некорректная лексема, необходимо исправить)\n"; }
	switch (prevstate) {
	case Start: 
		{ cout << "\n(ОШИБКА!!! Ожидалось слово for.)\n"; return; }
	case Stat: 
		{ cout << "\n(ОШИБКА!!! Ожидался идентификатор.)\n"; return; }
	case Stat1: case WS: 
		{ cout << "\n(ОШИБКА!!! Ожидался оператор присванивания <<=>>)\n"; return; }
	case AE: case WT: case WS1:
		{ cout << "\n(ОШИБКА!!! Ожидался операнд)\n"; return; }
	case AE1:
		{ cout << "\n(ОШИБКА!!! Ожидалось слово to или арифметическая операция)\n"; return; }
	case WT1:
		{ cout << "\n(ОШИБКА!!! Ожидался идентификатор или арифметическая операция)\n"; return; }
	case WS2:
		{ cout << "\n(ОШИБКА!!! Ожидалось слово next или арифметическая операция)\n"; return; }
	}
}

//Функция синтаксического анализа
void SynAnalysis(vector<Lex>& lex)
{
	int lex_num = 0, size = lex.size();	//Номер лексемы в векторе
	SState state = Start, prevstate;
	bool error = false;			//Булева переменная, обозначающая наличие хоть одной ошибки
	Lex lexema;
	cout << "\nПроверка синтаксическим анализатором:\n";
	for (; lex_num < size; ++lex_num) {		//Поочередно проходим по всем лексемам
		prevstate = state;					
		lexema = lex[lex_num];
		cout << lexema.str;
		(lexema.type == lNext) ? cout << "\n\n" : cout << ' ';		//Разделение конструкций двумя переводами строк
		int i = SynGMC(lexema);					//Переход по матрице состояний 
		state = (SState)syn_matrix[i][state];	//синт. анализатора
		if ((lex_num == size - 1 || lex[lex_num + 1].type == lFor) && state != Start) {
			error = true;
			(state == err) ? CheckError(prevstate, lexema) : CheckError(state, lexema);		//Обнаружение ошибки
			state = Start;
			continue;
		}

		if (state == err) {
			error = true;
			CheckError(prevstate, lexema);		//Обнаружение ошибки
		}
		if (state == Start && lex_num == size - 1 && !error)
			cout << "Ошибок не найдено.\n";
	}
	if (error && voice) {				//Если есть хоть одна ошибка и звуковое сопровождение вкл.,
		PlaySound(TEXT("Error.wav"),0, SND_ASYNC);		//то производится звук ошибки Windows XP
	}
	return;
}

int main()
{
	setlocale(LC_ALL, "rus");
	string input, text;			//Строковые переменные, обозначающие имя файла и его содержимое соответственно
	On_Off_voice();						//Функция вкл/выкл звукового сопровождения
	cout << "Введите имя файла: ";
	getline(cin, input);							//Предлагается ввести имя открываемого файла
	ifstream fin(input, ios::binary);
	if (!fin.is_open() || fin.peek() == EOF) {		//Проверка на открытие и на пустоту файла
		cout << "Файл не открывается или он пустой!\n";
		system("pause");
		return 1;
	}
	for (string temp; getline(fin, temp);) {	//в переменную text заносится в каждой итерации строка из файла
		for (int length = temp.length(); temp[length - 1] == ' '; length -= 1, temp.erase(length));	//Удаление пробелов в конце строки
		temp += '\n';
		text += temp;
	}
	text += "\0";									//Необходимо для работы лексического анализатора
	cout << "\nСодержимое файла:\n" << text << endl;
	fin.close();									//Закрытие файла с текстом
	vector<Lex> lex;			//Хранит все лексемы
	LexAnalysis(text, lex);	//Запуск лексического анализатора
	SynAnalysis(lex);		//Запуск синтаксического анализатора
	cout << endl;
	system("pause");
	return 0;
}
