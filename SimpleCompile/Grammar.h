#pragma once


#include <map>
#include <set>
#include <fstream>
#include <memory>
#include <string>
#include "Lexer.h"


class Grammar {
private:
	std::unique_ptr<std::set<std::string>> Vn;	// 非终结符号集
	std::unique_ptr<std::set<std::string>> Vt;	// 终结符号集
	std::string S;								// 开始符号
	std::unique_ptr<std::map<std::string, std::set<std::string>>> P;	// 生成式
	std::unique_ptr<std::map<std::string, std::set<std::string>>> nP;	// 消除左递归的生成式
	std::unique_ptr<std::map<std::string, std::set<std::string>>> FIRST;	// FIRST集
	std::unique_ptr<std::map<std::string, std::set<std::string>>> FOLLOW;	// FOLLOW集
	std::unique_ptr<std::map<std::pair<std::string, std::string>, std::string>> analysis_table;	// 预测分析表

	std::unique_ptr<std::map<std::string, std::set<std::string>>> FIRSTVT;	// FIRSTVT集
	std::unique_ptr<std::map<std::string, std::set<std::string>>> LASTVT;	// LASTVT集
	std::unique_ptr<std::map<std::pair<std::string, std::string>, std::string>> OPG_table;	// 算符优先表

	template <typename It>
	void generate_follow_aux(const It i);

	template <typename It>
	void generate_first_aux(const It i);

	template <typename It>
	void generate_firstvt_aux(const It i);

	template <typename It>
	void generate_lastvt_aux(const It i);

	void generate_OPG_table_aux(const char x, const char y);

	bool transform_aux(const std::string &s, const std::string &N);
	
	bool transform(const std::string &s, const std::string &N);

	std::string reduction(const std::string &s);

	char peek;
public:
	Grammar();
	~Grammar();
	
	void convert_to_LL1() {
		remove_left_recursion();
		generate_first();
		generate_follow();
		generate_analysis_table();
	}

	void convert_to_OPG() {
		generate_firstvt();
		generate_lastvt();
		generate_OPG_table();
	}

	void generate_firstvt();

	void generate_lastvt();

	void generate_OPG_table();

	void OPG_analysis(const Lexer &l);

	void scan(std::ifstream &in);

	void analysis(const Lexer &l);

	void _remove_left_recursion();

	void remove_left_recursion();

	void extract_left_gene();

	void generate_first();

	void generate_follow();

	void generate_analysis_table();

	void print() const;
};

