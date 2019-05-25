#include "Lexer.h"
#include <cctype>


Lexer::Lexer() : line(0), peek(' ') {
	reserve(std::make_unique<Word>(IF, "if"));
	reserve(std::make_unique<Word>(ELSE, "else"));
	reserve(std::make_unique<Word>(DO, "do"));
	reserve(std::make_unique<Word>(WHILE, "while"));
	reserve(std::make_unique<Word>(THEN, "then"));
}


Lexer::~Lexer() { words.clear(); seq_vec.clear(); }


void Lexer::scan(std::ifstream &in) {
		for (;; peek = in.get()) {
			if (peek == ' ' || peek == '\t') continue;
			else if (peek == '\n') ++line;
			else if (in.eof()) return;
			else break;
		}

		std::string s;
		if (std::isdigit(peek)) {
			int v = 0;
			if (peek == '0') {
				peek = in.get();
				if (peek == 'x') {
					peek = in.get();
					while (std::isxdigit(peek)) {
						s.push_back(peek);
						if (peek > '9')
							v = 16 * v + std::tolower(peek) - 'a' + 9;
						else
							v = 16 * v + peek - '0';
						peek = in.get();
					}
					seq_vec.emplace_back(std::to_string(HEX), s);
				} else {
					if (peek != 0) {
						do {
							s.push_back(peek);
							v = 8 * v + peek - '0';
							peek = in.get();
						} while ('0' <= peek && peek < '8');
						seq_vec.emplace_back(std::to_string(OCTAL), s);
					}
				}
			} else {
				do {
					s.push_back(peek);
					v = 10 * v + peek - '0';
					peek = in.get();
				} while (std::isdigit(peek));
				seq_vec.emplace_back(std::to_string(DECIMAL), s);

			}

		} else if (std::isalpha(peek)) {
			do {
				s.push_back(peek);
				peek = in.get();
			} while (std::isalpha(peek));

			auto it = words.find(s);
			if (it != words.end()) {
				seq_vec.emplace_back(s, "-");

			} else {
				seq_vec.emplace_back(std::to_string(ID), s);
				//reserve(std::move(w));
			}
			return;
		} else {
			s.push_back(peek);
			switch (peek) {
			case '+':
				peek = in.peek();
				if (peek == '+') {
					s.push_back(peek);
					in.get();
				} else if (peek == '=') {
					s.push_back(peek);
					in.get();
				}
				break;
			case '-':
				peek = in.peek();
				if (peek == '-') {
					s.push_back(peek);
					in.get();
				} else if (peek == '=') {
					s.push_back(peek);
					in.get();
				}
				break;
			case '>':
			case '<':
				peek = in.peek();
				if (peek == '=') {
					s.push_back(peek);
					in.get();
				}
			}
			peek = ' ';
			seq_vec.emplace_back(s, "-");
		}
	
}

