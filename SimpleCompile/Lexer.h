#pragma once


#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>


const int ID = 0;
const int DECIMAL = 1;
const int OCTAL = 2;
const int HEX = 3;
const int IF = 256;
const int ELSE = 257;
const int WHILE = 258;
const int DO = 259;
const int THEN = 260;


class Token {
public:
	int tag; 
	Token(int t) : tag(t) {}
	virtual ~Token() {}
};


class Num : public Token {
public:
	int value;
	Num(int t, int v) : Token(t), value(v) {}
};


class Word : public Token {
public:
	std::string lexeme;
	Word(int t, const std::string& s) : Token(t), lexeme(s) {}
};


class Lexer {
public:
	Lexer();
	~Lexer();

	void reserve(std::unique_ptr<Word> w) {
		words.emplace(w->lexeme, std::move(w));
	}

	void scan(std::ifstream &);

	void show() const {
		for (auto i : seq_vec)
			std::cout << "<" << i.first << ", " << i.second << ">" << std::endl;
	}

	std::unordered_map<std::string, std::unique_ptr<Word>> words;
	std::vector<std::pair<std::string, std::string>> seq_vec;
	int line;
	char peek;
};

