#pragma once


#include <map>
#include <set>
#include <fstream>
#include <memory>
#include <string>
#include "Lexer.h"


class Grammar {
private:
	std::unique_ptr<std::set<std::string>> Vn;	// ���ս���ż�
	std::unique_ptr<std::set<std::string>> Vt;	// �ս���ż�
	std::string S;								// ��ʼ����
	std::unique_ptr<std::map<std::string, std::set<std::string>>> P;	// ����ʽ
	std::unique_ptr<std::map<std::string, std::set<std::string>>> nP;	// ������ݹ������ʽ
	std::unique_ptr<std::map<std::string, std::set<std::string>>> FIRST;	// FIRST��
	std::unique_ptr<std::map<std::string, std::set<std::string>>> FOLLOW;	// FOLLOW��
	std::unique_ptr<std::map<std::pair<std::string, std::string>, std::string>> analysis_table;	// Ԥ�������

	std::unique_ptr<std::map<std::string, std::set<std::string>>> FIRSTVT;	// FIRSTVT��
	std::unique_ptr<std::map<std::string, std::set<std::string>>> LASTVT;	// LASTVT��
	std::unique_ptr<std::map<std::pair<std::string, std::string>, std::string>> OPG_table;	// ������ȱ�

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

