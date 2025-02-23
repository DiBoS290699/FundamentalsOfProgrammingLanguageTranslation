// ЛР по ОТЯП №1.cpp : Defines the entry point for the console application.
//Вариант 15

#include "stdafx.h"
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

//Матрица переходов
int matrix[4][4] =
{					 //					 S	X	Y	E
	1,  3,  1,  3,   //(гласные)		 X	E	X	E	
	2,  2,  3,  3,   //(согласные)		 Y	Y	E	E	
	3,  3,  3,  3,   //(остальные, ь, ъ) E	E	E	E	
	4,  4,  4,  4,	 //('\n','_','\r')	 F	F	F	F	
};

//Состояния автомата
enum AState { S, X, Y, E, F };	//0, 1, 2, 3, 4

//Функция, определяющая гласную букву
bool isvowel(char c)
{
	return c == 'а' || c == 'о' || c == 'и' || c == 'е' || c == 'ё' ||
		c == 'э' || c == 'ы' || c == 'у' || c == 'ю' || c == 'я' ||  c == 'А' || c == 'О' || c == 'И' || c == 'Е' || c == 'Ё' ||
		c == 'Э' || c == 'Ы' || c == 'У' || c == 'Ю' || c == 'Я';
}

//Функция, определяющая согласную букву
bool isconsonant(char c)
{
	return c == 'б' || c == 'в' || c == 'г' || c == 'д' || c == 'ж' || c == 'з' || c == 'й' || c == 'к' ||
		c == 'л' || c == 'м' || c == 'н' || c == 'п' || c == 'р' || c == 'с' || c == 'т' || c == 'ф' ||
		c == 'х' || c == 'ц' || c == 'ч' || c == 'ш' || c == 'щ' || c == 'Б' || c == 'В' || c == 'Г' ||
		c == 'Д' || c == 'Ж' || c == 'З' || c == 'Й' || c == 'К' || c == 'Л' || c == 'М' || c == 'Н' || 
		c == 'П' || c == 'Р' || c == 'С' || c == 'Т' || c == 'Ф' || c == 'Х' || c == 'Ц' || c == 'Ч' || c == 'Ш' || c == 'Щ';
}

//Функция преобразования символа в соответствующий номер строки матрицы переходов
int GetMatrixCol(char currentChar)
{
	if (isvowel(currentChar)) return 0;				//Возвращение строки гласных букв
	if (isconsonant(currentChar)) return 1;			//Возвращение строки согласных букв
	if (currentChar == ' ' || currentChar == '\n' || currentChar == '\r' || currentChar == '\t') return 3;
	return 2;										//Возвращение строки остальных символов и ь, ъ
}

//Контейнер лексемы
struct Lex {
	bool valid;		//флаг соответствия заданию
	bool kiril;		//флаг на соответствие слову
	char* str;		//текст лексемы
};


//Функция лексического анализа
void LexAnalysis(vector<Lex>& result, char* str)
{
	int position = 0;				//текущая позиция в строке
	AState state = S;				//текущее состояние
	Lex lexema;						//текущая лексема
	int firstPos;					//позиция начала лексемы

	for (; str[position] == ' ' || str[position] == '\t'; position++);
	for (; str[position] != '\0'; position++)
{
		char currentChar = str[position];

		//Инициализация лексемы при обнаружении непробельного символа
		if (state == S && currentChar != ' ') 
		{
			firstPos = position;
			lexema.kiril = true;
			lexema.valid = true;
		}

		//Переход по матрице состояний
		int i = GetMatrixCol(currentChar);
		state = (AState)matrix[i][state];

		//Определение является ли последовательность словом
		if (state == E) {
			lexema.valid = false;
			if (i == 2 && currentChar != 'ь' && currentChar != 'ъ' && currentChar != 'Ь' && currentChar != 'Ъ') {
				lexema.kiril = false;		//Если символ не кириллица, то это не слово
			}
		}

		//Запись текущей лексемы в выходной список и инициализация новой лексемы
		if (state == F && str[position - 1] != ' ')
		{
			int length = position - firstPos;
			lexema.str = new char[length + 1];
			strncpy_s(&lexema.str[0], length + 1, &str[0] + firstPos, length);	//вычленение подстроки и запись в лексему
			result.push_back(lexema);											//запись лексемы в список
			state = S;
		}
	}
	if (str[position - 1] != ' ' && str[position] == '\0')
	{
		int length = position - firstPos;
		lexema.str = new char[length + 1];
		strncpy_s(&lexema.str[0], length + 1, &str[0] + firstPos, length);	//вычленение подстроки и запись в лексему
		result.push_back(lexema);											//запись лексемы в список
	}
	return;
}

//Функция занесения результатов в файл и вывод на консоль
void entry(ofstream& fout, const vector<Lex>& result)
{
	cout << "\nУдовлетворяющие условию лексемы:\n";
	fout << "Удовлетворяющие условию лексемы:\n";
	for (Lex i : result) {
		if (i.valid && i.kiril) {
			cout << i.str << ' ';
			fout << i.str << ' ';
		}
	}
	cout << "\n---------------------------\n" << "Неудовлетворяющие условию лексемы:\n";
	fout << "\n---------------------------\n" << "Неудовлетворяющие условию лексемы:\n";
	for (Lex i : result) {
		if (!i.valid && i.kiril) {
			cout << i.str << ' ';
			fout << i.str << ' ';
		}
		delete[] i.str;
	}
}

int main()
{
	setlocale(LC_ALL, "rus");
	cout << "Введите имя файла: ";
	char input[50];
	cin.getline(input, 50);
	ifstream fin(input, ios::binary);
	if (!fin.is_open() || fin.peek() == EOF) {		//Проверка на открытие и на пустоту файла
		cout << "Файл не открывается или он пустой!\n";
		system("pause");
		return 1;
	}
	
	//Универсальный вариант чтения всего файла в одну строку (произвольной длины)
	fin.seekg(0, ios::end);			//Перемещение указателя в конец
	int length = fin.tellg();		//Определение длины потока по номеру указателя
	char* str = new char[length + 1];
	fin.seekg(0, ios::beg);			//Перемещение указателя в начало
	fin.read(&str[0], length);		//Занесение в массив str файла длины length
	str[length] = '\0';				
	cout << "Текст файла:\n" << str << endl;
	vector<Lex> result;
	LexAnalysis(result, str);

	ofstream fout("output.txt");
	entry(fout, result);

	//Очистка памяти для универсального варианта чтения
	delete[] str;

	fin.close();
	fout.close();
	cout << endl;
	system("pause");
    return 0;
}