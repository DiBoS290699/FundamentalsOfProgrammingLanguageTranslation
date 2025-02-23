//Лаба по ОТЯП №2.cpp
//Вариант 5. Конструкция: 5. Уровень: 2

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

const char BIG_EN_BEG = 65;			//Код буквы А в ASCII
const char BIG_EN_END = 90;			//Код буквы Z в ASCII
const char SMALL_EN_BEG = 97;		//Код буквы a в ASCII
const char SMALL_EN_END = 122;		//Код буквы z в ASCII
const int END_CONST = 32767;		//Правый диапазон константы
const int STR_LENGTH = 255;			//Допустимое количество символов в идентификаторе
enum EState { S, Ai, Ac, As, Bs, Cs, Ds, Gs, E, F, Fe };	//Список состояний
enum ELexType { lFor, lTo, lNext, lAnd, lOr, lRel, lAs, lAo, lVar, lConst, LError };		//Список типов лексем

int matrix[8][9]								/*			0	1	2	3	4	5	6	7	8*/		
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

//Функция преобразования символа в соответствующий номер строки матрицы переходов
int GetMatrixCol(char currentChar)
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
void AddType(const EState& prevState, Lex& lexema, vector<Lex>& var, vector<Lex>& con)
{
	if (prevState == Ai)	{ 
		if (lexema.str == "for") { lexema.type = lFor; cout << lexema.str << "           " << "Ключевое слово - for\n"; return; }
		if (lexema.str == "to") { lexema.type = lTo;  cout << lexema.str << "           " << "Ключевое слово - to\n"; return; }
		if (lexema.str == "next") { lexema.type = lNext; cout << lexema.str << "           " << "Ключевое слово - next\n"; return; }
		if (lexema.str == "and") { lexema.type = lAnd; cout << lexema.str << "           " << "Ключевое слово - and\n"; return; }
		if (lexema.str == "or") { lexema.type = lOr; cout << lexema.str << "           " << "Ключевое слово - or\n"; return; }
		lexema.type = lVar;
		cout << lexema.str << "           " << "Идентификатор\n"; 
		var.push_back(lexema);
		return;
	}
	else if (prevState == Ac)	{ 
		lexema.type = lConst;
		cout << lexema.str << "           " << "Константа\n"; 
		con.push_back(lexema);
		return;
	}
	else if (prevState == As || prevState == Gs || prevState == Ds) { lexema.type = lRel; cout << lexema.str << "           " << "Операция сравнения\n"; return; }
	else if (prevState == Bs)	{ lexema.type = lAs; cout << lexema.str << "           " << "Операция присваивания\n";  return; }
	else if (prevState == Cs)	{ lexema.type = lAo; cout << lexema.str << "           " << "Арифметическая операция\n"; return; }
}

//Вывод результата
void entry(const string& str, vector<Lex>& var, vector<Lex>& con, Lex& lexema, const EState& prevState, const int& firstPos, const int& position)
{
	lexema.str = str.substr(firstPos, position - firstPos);	//вычленение подстроки и запись в лексему
	if (prevState == E || prevState == Ai && lexema.str.length() > STR_LENGTH) {
		lexema.type = LError;
		cout << lexema.str << "           " << "Некорректная лексема\n"; 
		return; 
	}
	AddType(prevState, lexema, var, con);						//Функция определения типа лексемы и её запись
	return;
}
//Функция лексического анализа
void LexAnalysis(string str, vector<Lex>& var, vector<Lex>& con)
{
	int position = 0;				//текущая позиция в строке
	EState state = S, prevState;	//текущее состояние
	int firstPos;					//позиция начала лексемы
	Lex lexema;						//текущая лексема

	for (; isempty(str[position]); position++);		//Пропуск начальных пустых символов
	for (; str[position] != '\0'; position++)		//Проход до конца текста
	{
		char currentChar = str[position];
		prevState = state;			//Предыдущее состояние

		//Инициализация лексемы при обнаружении непробельного символа
		if (state == S) {
			for (; isempty(str[position]); position++);		//Пропуск пустых символов при считывании новой лексемы 
			currentChar = str[position];
			firstPos = position;
		}
		//Переход по матрице состояний
		int i = GetMatrixCol(currentChar);
		state = (EState)matrix[i][state];

		//Запись текущей лексемы после пустых символов
		if (state == F) {
			entry(str, var, con, lexema, prevState, firstPos, position);
			state = S;
		}

		//Запись текущей лексемы после специальных символов
		if (state == Fe) {
			entry(str, var, con, lexema, prevState, firstPos, position);
			state = S;
			--position;
		}
	}
	return;
}

//Функция вывода на экран таблицы ИДЕНТИФИКАТОРОВ И КОНСТАНТ
void tablica(const vector<Lex>& var, const vector<Lex>& con) {
	cout << "----Идентификаторы----\n";
	for (Lex i : var)				//использование foreach для вектора идентификаторов
		cout << i.str << endl;
	cout << "----Константы----\n";
	for (Lex i : con)				//использование foreach для вектора констант
		cout << i.str << endl;
	return;
}

//Вывод на экран анекдота
void anecdote()
{
	cout << "Хочешь анекдот напоследок? :)\n"
		<< "Введите номер анекдота (0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9) либо (-1), если не хочешь: ";
	vector<string> file = { "anecdote/0.txt", "anecdote/1.txt" , "anecdote/2.txt" , "anecdote/3.txt" , "anecdote/4.txt" , 
		"anecdote/5.txt", "anecdote/6.txt", "anecdote/7.txt", "anecdote/8.txt", "anecdote/9.txt" };
	long int num;
	cin >> num;		//Ввод номера анекдота
	if (num == -1) return;
	do {
		if (num < 0 || num > 9) {		//Если вышли за пределы номеров, 
			cout << "Попробуй ещё раз: ";		//то пользователь должен попробовать ещё раз
		}
		else {
			ifstream fin(file[num]);
			if (!fin.is_open() || fin.eof()) {		//Проверка на открытие и на пустоту файла
				cout << "Файл не открывается или он пустой!\n";
				return;
			}
			cout << "----------------------------------------\n";
			string text;
			for (; getline(fin, text); text += '\n', cout << text);
			cout << "----------------------------------------\n";
			fin.close();
			cout << "Хочешь ещё анекдот? =) Правила те же: ";
		}
		cin >> num;
	} while (num != -1);
	return;
}

int main()
{
	setlocale(LC_ALL, "rus");
	cout << "Введите имя файла: ";
	string input, text;
	getline(cin, input);							//Предлагается ввести имя открываемого файла
	ifstream fin(input, ios::binary);
	if (!fin.is_open() || fin.peek() == EOF) {		//Проверка на открытие и на пустоту файла
		cout << "Файл не открывается или он пустой!\n";
		system("pause");
		return 1;
	}
	for (string temp; getline(fin, temp); temp += '\n', text += temp);	//в переменную text заносится в каждой итерации строка из файла
	text += "\0";									//Необходимо для работы лексического анализатора
	cout << "Текст файла:\n" << text << endl;
	vector<Lex> var;			//Хранит все идентификаторы
	vector<Lex> con;			//Хранит все константы
	LexAnalysis(text, var, con);	//Запуск лексического анализатора
	tablica(var, con);				//Вывод идентификаторов и констант
	anecdote();						//Функция для оригинальности
    return 0;
}