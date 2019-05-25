#include "Grammar.h"
#include <iostream>
#include <stack>


using namespace std;


Grammar::Grammar() : 
	Vn(make_unique<set<string>>()),
	Vt(make_unique<set<string>>()),
	S(""), P(make_unique<map<string, set<string>>>()), peek(' '),
	nP(make_unique<map<string, set<string>>>()),
	FIRST(make_unique<map<string, set<string>>>()),
	FOLLOW(make_unique<map<string, set<string>>>()),
	analysis_table(make_unique<map<pair<string, string>, string>>()),
	FIRSTVT(make_unique<map<string, set<string>>>()),
	LASTVT(make_unique<map<string, set<string>>>()),
	OPG_table(make_unique<map<pair<string, string>, string>>()) {}


Grammar::~Grammar() {}


// ��left -> right ���浽����ʽP ��
static inline void save_into_P(const string &left, const string &right, map<string, set<string>> &P) {
	if (!left.empty() && !right.empty()) {
		auto it = P.find(left);
		if (it != P.end())
			it->second.emplace(right);
		else
			P.emplace(left, set<string>{ right });
	}
}


template <typename It>
void Grammar::generate_firstvt_aux(const It i) {
	for (auto j : i->second) {
		auto c = j.cbegin();
		if (isupper(*c)) {
			(*FIRSTVT)[i->first].insert((*FIRSTVT)[{ *c }].cbegin(), (*FIRSTVT)[{ *c }].cend());
			if (1 < j.size())
				(*FIRSTVT)[i->first].insert({ *(++c) });
		} else
			(*FIRSTVT)[i->first].insert({ *c });
	}
}


void Grammar::generate_firstvt() {
	for (auto i = P->cbegin(); i != P->cend(); ++i) {
		generate_firstvt_aux(i);
		for (auto j = P->cbegin(); j != i; ++j)
			generate_firstvt_aux(j);
	}
}


template <typename It>
void Grammar::generate_lastvt_aux(const It i) {
	for (auto j : i->second) {
		auto c = j.crbegin();
		if (isupper(*c)) {
			(*LASTVT)[i->first].insert((*LASTVT)[{ *c }].cbegin(), (*LASTVT)[{ *c }].cend()); 
			if (1 < j.size())
				(*LASTVT)[i->first].insert({ *(++c) });
		} else
			(*LASTVT)[i->first].insert({ *c });	
	}
}


void Grammar::generate_lastvt() {
	for (auto i = P->cbegin(); i != P->cend(); ++i) {
		generate_lastvt_aux(i);
		for (auto j = P->cbegin(); j != i; ++j)
			generate_lastvt_aux(j);
	}
}


void Grammar::generate_OPG_table_aux(const char x, const char y) {
	if (!isupper(x) && !isupper(y))
		(*OPG_table)[{ { x }, { y }}] = "=";
	else if (isupper(x))
		for (auto k : (*LASTVT)[{ x }])
			(*OPG_table)[{ k, { y }}] = ">";
	else
		for (auto k : (*FIRSTVT)[{ y }])
			(*OPG_table)[{ { x }, k }] = "<";
}


void Grammar::generate_OPG_table() {
	for (auto i : *P) {
		for (auto j : i.second) {
			if (j.size() == 1) 
				continue;
			auto prev = j.cbegin();
			auto cur = prev + 1;
			auto next = cur + 1;
			while (cur != j.cend()) {
				generate_OPG_table_aux(*prev, *cur);
				if (next == j.cend())
					break;
				if (isupper(*cur))
					(*OPG_table)[{ { *prev }, { *next }}] = "=";
				++prev;
				++cur;
				++next;
			}
		}
	}
}


bool Grammar::transform_aux(const string &s, const string &N){
	bool res = false;

	if (Vn->find(N) != Vn->end()) {
		// ��N������ʽ���²���
		for (auto i : (*P)[N]) {
			if (i == s)
				return true;
			else if (Vn->find(i) != Vn->end())
				res = transform_aux(s, i);
			if (res)
				return res;
		}
	}

	return false;
}


// ���Խ����ʽsת�����������ս��N���ܳɹ�ת����������s���ķ���ֱ�ӳ��ֹ�
bool Grammar::transform(const string &s, const string &N) {
	return transform_aux(s, N) || transform_aux(N, s);
}


// ���ַ���s���й�Լ
string Grammar::reduction(const string &s) {
	return "";
}


void Grammar::OPG_analysis(const Lexer &l) {
	vector<string> svec;
	svec.push_back("#");

	string sym;
	int k = 0;
	int j;
	bool correct = true;
	vector<stack<string>> slvec{ {} };
	vector<stack<string>> srvec{ {} };
	int curStack = 0;
	for (int i = 0; i < l.seq_vec.size();) {
		if (l.seq_vec[i].first == ";") {
			if (correct)
				cout << "��ȷ" << endl;
			svec.clear();
			svec.push_back("#");
			slvec.clear();
			slvec.push_back({});
			srvec.clear();
			srvec.push_back({});
			curStack = 0;
			k = 0;
			correct = true;
			++i;
			continue;
		}
		if (isalnum(l.seq_vec[i].first[0]))
			sym = "i";
		else
			sym = l.seq_vec[i].first;
		if (!isupper(svec[k][0]))
			j = k;
		else
			j = k - 1;

		if (sym == "(") {
			++curStack;
			srvec.push_back({});
			slvec.push_back({});
		}

		while ((*OPG_table)[{svec[j], sym}] == ">") {
			string Q;
			string res;
			int count = 0;
			if (svec.back() == ")")
				j -= 3;
			else {
				do {
					Q = svec[j];
					if (!isupper(svec[j - 1][0])) {
						if (svec[j - 1] == "#" || svec[j-1] == "(") {
							if (count <= 1)
								j -= 1;
							else
								break;
						} else {
							j -= 1;
						}
						++count;
					} else if (j - 2 >= 0) {
						if (svec[j - 2] == "#" || svec[j - 2] == "(") {
							if (count <= 1)
								j -= 2;
							else
								break;
						} else {
							j -= 2;
						}	
						++count;
					}
				} while ((*OPG_table)[{svec[j], Q}] == "<" && (*OPG_table)[{svec[j], sym}] == ">");
			}
			for (auto it = svec.begin() + j + 1; it < svec.begin() + k + 1; ++it)
				res += *it;
			svec.erase(svec.begin() + j + 1, svec.begin() + k + 1);
			k = j + 1;

			string left, right, nextRight;
			if (sym != "#") {
				for (auto i : *P) {
					for (auto x : i.second) {
						auto y = x.find(sym);
						if (y != string::npos) {
							if (!slvec[curStack].empty())
								slvec[curStack].pop();
							slvec[curStack].push(i.first);
							if (x[0] == '(')
								left = "E";
							else
								left = x.substr(0, y);
							if (y < x.size() - 1)
								nextRight = x.substr(y + 1, x.size());
							break;
						}
					}
					if (!left.empty())
						break;
				}
			} else if (!slvec[curStack].empty()) {
				left = slvec[curStack].top();
				slvec[curStack].pop();
			} else {
				left = "E";
				nextRight = "";
			}

			if (left.empty() && !slvec[curStack].empty()) {
				left = slvec[curStack].top();
				slvec[curStack].pop();
			}

			if (transform(res, left)) {
				svec.push_back(left);
				if (res[0] == '(') {
					slvec.erase(slvec.begin() + curStack);
					srvec.erase(srvec.begin() + curStack);
					--curStack;
				}
				if(!nextRight.empty())
					srvec[curStack].push(nextRight);
			} else {
				if (!srvec[curStack].empty()) {
					right = srvec[curStack].top();
					srvec[curStack].pop();
					if (transform({ *(--res.end()) }, right)) {
						res.erase(--res.end()); 
						res.append(right);
						if (transform(res, left)) {
							svec.push_back(left);
							if (res[0] == '(') {
								slvec.erase(slvec.begin() + curStack);
								srvec.erase(srvec.begin() + curStack);
								--curStack;
							}
							if (!nextRight.empty())
								srvec[curStack].push(nextRight);
						}
						else {
							cout << "��������֮������ʧ��" << endl;
							correct = false;
						}
					} else {
						cout << "��������ʧ��" << endl;
						correct = false;
					}
				} else {
					cout << "�����޷�����" << endl;
					correct = false;
				}
			}
		}

		if (!correct) {
			while (k < l.seq_vec.size() && l.seq_vec[i].first != ";")
				++i;
			continue;
		}

		if ((*OPG_table)[{svec[j], sym}] == "<" || (*OPG_table)[{svec[j], sym}] == "=") {
			++i;
			++k;
			svec.push_back(sym);
		} else {
			cout << "��������ƥ��ʧ��" << endl;
			correct = false;
		}
	}
}


void Grammar::scan(ifstream &in) {
	while (!in.eof()) {
		for (;; peek = in.get()) {
			if (peek == ' ' || peek == '\t' || peek == '\n')
				continue;
			break;
		}
		if (isalpha(peek)) {
			// leftΪ����ʽ�󲿣�rightΪ����ʽ�Ҳ�
			string left, right;

			// �����󲿣�������ս�����ϣ�����ʼ����SΪ�գ��������Ϊ��ʼ����
			while (isupper(peek) || peek == '\'') {
				left.push_back(peek);
				peek = in.get();
			}

			Vn->insert(left);
			if (S.empty())
				S = left;

			// �����ո��->
			while (peek == ' ')
				peek = in.get();

			if (peek == '-' && in.peek() == '>') {
				in.get();
				peek = in.get();
			}

			// ������е���������ʽ��������
			for (; peek != '\n' && !in.eof(); peek = in.get()) {
				if (peek == ' ' || peek == '\t')
					continue;
				else if (peek == '|') {
					save_into_P(left, right, *P);
					right.clear();
				} else if (isupper(peek)) {
					Vn->insert(string{ peek });
					right.push_back(peek);
				} else {
					Vt->insert(string{ peek });
					right.push_back(peek);
				}
			}
			save_into_P(left, right, *P);
		}
	}
	peek = ' ';
}


void Grammar::analysis(const Lexer &l) {
	stack<string> ss;
	ss.push("#");
	ss.push(S);

	string x, a;
	for (auto i = 0; i < l.seq_vec.size(); ) {
		if (l.seq_vec[i].first == ";") {
			if (ss.empty()) {
				cout << "��ȷ" << endl;
				ss.push("#");
				ss.push(S);
				++i;
				continue;
			} else {
				stack<string> tmp{};
				ss.swap(tmp);
				ss.push("#");
				ss.push(S);
				++i;
				continue;
			}
		}
		x = ss.top();
		if (isalnum(l.seq_vec[i].first[0]))
			a = "i";
		else
			a = l.seq_vec[i].first;
		if (x == a) {
			ss.pop();
			++i;
		} else if (isupper(x[0])) {
			auto it = analysis_table->find({ x, a });
			if (it != analysis_table->end()) {
				ss.pop();
				if (it->second != "@") {
					for (auto rb = it->second.rbegin(); rb != it->second.rend(); ++rb) {
						if (*rb == '\'') {
							string s{ "'" };
							++rb;
							while (rb != it->second.rend() && *rb == '\'') {
								s.push_back('\'');
								++it;
							}
							s.push_back(*rb);
							reverse(s.begin(), s.end());
							ss.push(s);
						} else
							ss.push(string{ *rb });
					}
				}
			} else {
				cout << "����û��ƥ�������ʽ" << endl;
				while (i < l.seq_vec.size() && l.seq_vec[i].first != ";")
					++i;
			}
		} else {
			cout << "��������Ȳ�ƥ��Ҳ���Ƿ��ս��" << endl;
			while (i < l.seq_vec.size() && l.seq_vec[i].first != ";")
				++i;
		}
	}
}


// �ж�s�Ƿ���prefix��ͷ
static inline bool starts_with(const string &prefix, const string &s) {
	return prefix == s.substr(0, prefix.size());
}


// ��s�г�except�е�ÿ��Ԫ�ؼ��Ϻ�׺suffix
static inline set<string> append_into_set_e(const set<string> &s, const string &suffix, const set<string> &except) {
	set<string> tmp;
	for (string i : s)
		if(except.find(i) == except.end())
			tmp.insert(i + suffix);
	return tmp;
}


// ������������ʽ��ֱ����ݹ�
static inline void remove_direct_r(const set<string> &old, map<string, set<string>> &m, const string &left) {
	set<string> s;
	// ���ҳ����д���ֱ����ݹ������ʽ
	for(auto p : old)
		if (starts_with(left, p))
			s.insert(p);
	// ȥ����Щ����ʽ
	m[left] = append_into_set_e(old, "", s);
	string attach = "'";
	for (auto rb = s.rbegin(); rb != s.rend(); ++rb, attach += "'") {
		// ������ݹ�
		string _S = left + attach;
		string a = rb->substr(left.size(), rb->size());
		m[left] = append_into_set_e(m[left], _S, {});
		m[_S] = { a + _S, "@" };
	}
}


void Grammar::remove_left_recursion() {
	auto &m = *nP;
	auto old_P = *P;
	for (auto i = P->rbegin(); i != P->rend(); ++i) {
		// ÿ�ε�����������i��ֱ����ݹ飬Ȼ���[j, i)֮�������ʽ�����滻i
		for (auto j = P->rbegin(); j != i; ++j) {
			for (string jP : j->second)
				if (starts_with(i->first, jP)) {
					string suffix = jP.substr(i->first.size(), jP.size());
					m[j->first].erase(jP);
					for (auto i : i->second)
						m[j->first].insert(i + suffix);
				}
			 j->second = m[j->first];
		}
		remove_direct_r(i->second, m, i->first);
		i->second = m[i->first];
	}
	// ��������ݹ��¼���ķ��ս����ӵ�Vn��
	for (auto it : m)
		Vn->insert(it.first);
	*P = std::move(old_P);
}


// ̫�����ˣ���û������
void Grammar::_remove_left_recursion() {
	
	auto old_P = *P;
	while (true) {
		// ������ݹ�ֱ���ޱ仯
		auto old_nP = *nP;
		remove_left_recursion();
		if (old_nP == *nP) break;
	}
	*P = std::move(old_P);

	// ������ʼ�����޷����������ʽ
	set<string> s = *Vn;
	for (auto item1 : (*nP)[S]) {
		for (auto item2 = item1.begin(); item2 != item1.end();) {
			if (isupper(*item2)) {
				string tmp;
				tmp.push_back(*item2);
				++item2;
				while (item2 != item1.end() && *item2 == '\'') {
					tmp.push_back(*item2);
					++item2;
				}
				s.erase(tmp);
			} else
				++item2;
		}
	}
	for (auto it : s){
		Vn->erase(it);
		nP->erase(it);
	}
}


// ��ȡ������
void Grammar::extract_left_gene() {
	
}


static inline string front_split(const string &s) {
	string res;
	for (auto it = s.begin(); it != s.end();) {
		if (isupper(*it)) {
			res.push_back(*it);
			++it;
			while (it != s.end() && *it == '\'') {
				res.push_back(*it);
				++it;
			}
			break;
		} else {
			res.push_back(*it);
			break;
		}
	}
	return res;
}


// ��ģ��ֻ����Ϊ����������̫����
template <typename It>
void Grammar::generate_first_aux(const It i) {
	for (auto p : i->second) {
		auto first = front_split(p);
		for (auto f : (*FIRST)[first])
			(*FIRST)[i->first].insert(f);
	}
}


void Grammar::generate_first() {
	// �����ս������FIRST��Ϊ����
	(*FIRST)["@"] = { "@" };
	for (auto it : *Vt) {
		(*FIRST)[it] = { it };
	}
	
	for (auto i = nP->begin(); i != nP->end(); ++i) {
		for (auto j = nP->begin(); j != i; ++j)
			generate_first_aux(j);
		generate_first_aux(i);
	}
}


// ��һ��string������ʽ�еķ��ս�����ڶ���string��ÿ�����ս���ĺ�һ������
static inline set<pair<string, string>> back_split(const string &s) {
	set<pair<string, string>> res;
	string cur, prev;
	for (auto it = s.rbegin(); it != s.rend(); ++it) {
		if (isupper(*it)) {
			cur.push_back(*it);
			res.insert({ cur, prev });
			prev = cur;
			cur.clear();
		} else if (*it == '\'') {
			cur.push_back(*it);
			++it;
			while (it != s.rend() && *it == '\'') {
				cur.push_back(*it);
				++it;
			}
			cur.push_back(*it);
			reverse(cur.begin(), cur.end());
			res.insert({ cur, prev });
			prev = cur;
			cur.clear();
		} else {
			prev.clear();
			prev.push_back(*it);
		}
	}
	return res;
}


template <typename It>
void Grammar::generate_follow_aux(const It i) {
	for (auto p : i->second) {
		// ����Ai��ÿһ������ʽ���в��
		auto follow = back_split(p);
		for (auto fo : follow) {
			// ���ڲ�ֵ�ÿ����Ϲ���FOLLOW��
			if (fo.second.empty() || (*FIRST)[fo.second].find("@") != (*FIRST)[fo.second].end()) {
				for (auto f : (*FOLLOW)[i->first])
					(*FOLLOW)[fo.first].insert(f);
				(*FOLLOW)[fo.first].erase("@");
			}
			// ע�����ﻹҪ�жϣ�ԭ������һ���жϵĵڶ�������
			if (!fo.second.empty()) {
				for (auto f : (*FIRST)[fo.second])
					(*FOLLOW)[fo.first].insert(f);
				(*FOLLOW)[fo.first].erase("@");
			}
		}
	}
}


void Grammar::generate_follow() {
	// �Ƚ�#���뿪ʼ���ŵ�FOLLOW��
	(*FOLLOW)[S].insert("#");

	for (auto i = nP->begin(); i != nP->end(); ++i) {
		for (auto j = nP->begin(); j != i; ++j)
			// ��������
			generate_follow_aux(j);
		generate_follow_aux(i);
	}
}


// ����Ԥ�������
void Grammar::generate_analysis_table() {
	// p : left -> right
	for (auto p : *nP) {
		auto left = p.first;
		for (auto right : p.second) {
			// ��ÿ�����ս����ÿ������ʽ
				for (auto vt : (*FIRST)[right.substr(0,1)]) {
					// �ж���FIRST���е�Ԫ���Ƿ�Ϊ@
					if(vt != "@")
						// Ԫ��Ϊ�ս��vt
						(*analysis_table)[{left, vt}] = right;
					else
						// Ԫ��Ϊ�ռ�@
						for(auto follow : (*FOLLOW)[left])
							(*analysis_table)[{left, follow}] = right;
				}
		}
	}
}


void Grammar::print() const {

	cout << "���ս���ż���" << flush;
	for (auto vn : *Vn) {
		cout << vn << ends;
	}
	cout << endl;
	cout << "�ս���ż���" << flush;
	for (auto vt : *Vt) {
		cout << vt << ends;
	}
	cout << std::endl;
	cout << "��ʼ����" << S << endl;
	cout << "����ʽ" << endl;
	for (auto p : *P) {
		cout << p.first << " -> " << flush;
		for (auto i : p.second)
			cout << i << "|" << flush;
		cout << endl;
	}

	cout << "������ݹ�����ʽ" <<endl;
	for (auto p : *nP) {
		cout << p.first << " -> " << flush;
		for (auto i : p.second)
			cout << i << "|" << flush;
		cout << endl;
	}

	cout << "FIRST��" << endl;
	for (auto f : *FIRST) {
		cout << f.first << " { " << flush;
		for (auto i : f.second)
			cout << i << ", " << flush;
		cout << "}" << endl;
	}

	cout << "FOLLOW��" << endl;
	for (auto f : *FOLLOW) {
		cout << f.first << " { " << flush;
		for (auto i : f.second)
			cout << i << ", " << flush;
		cout << "}" << endl;
	}

	cout << "Ԥ�������" << endl;
	for (auto it : *analysis_table) {
		cout << "<" << it.first.first << ", " << it.first.second << "> : " << it.second << endl;
	}

	cout << "FIRSTVT��" << endl;
	for (auto f : *FIRSTVT) {
		cout << f.first << " { " << flush;
		for (auto i : f.second)
			cout << i << ", " << flush;
		cout << "}" << endl;
	}

	cout << "LASTVT��" << endl;
	for (auto f : *LASTVT) {
		cout << f.first << " { " << flush;
		for (auto i : f.second)
			cout << i << ", " << flush;
		cout << "}" << endl;
	}

	cout << "������ȱ�" << endl;
	for (auto it : *OPG_table) {
		cout << "<" << it.first.first << ", " << it.first.second << "> : " << it.second << endl;
	}
}

