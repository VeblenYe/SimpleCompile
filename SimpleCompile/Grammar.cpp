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


// 将left -> right 保存到生成式P 中
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
		// 从N的生成式向下查找
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


// 尝试将表达式s转换到给定非终结符N，能成功转换的条件是s在文法内直接出现过
bool Grammar::transform(const string &s, const string &N) {
	return transform_aux(s, N) || transform_aux(N, s);
}


// 对字符串s进行归约
string Grammar::reduction(const string &s) {
	return "";
}


void Grammar::OPG_analysis(const Lexer &l) {
	vector<string> svec;
	svec.push_back("#");

	string sym;		// 当前读入字符
	size_t k = 0;	// 当前归约栈中的最后一个元素位置
	size_t j;		// 当前归约栈中的最后一个非终结符的位置
	bool correct = true;
	stack<int> si;
	vector<stack<string>> srvec{ {} };	// 该栈中保存查询生成式的右部，作为下一个符号的归约对象
	int curStack = 0;	// 每进入一个左括号，则切换到新的栈，该值指向当前符号串所对应的栈
	for (int i = 0; i < l.seq_vec.size();) {
		if (l.seq_vec[i].first == ";") {
			if (correct)
				cout << "正确，计算结果为" << si.top() << endl;
			svec.clear();
			svec.push_back("#");
			while (!si.empty())
				si.pop();
			srvec.clear();
			srvec.push_back({});
			curStack = 0;
			k = 0;
			correct = true;
			++i;
			continue;
		}
		// 将字母与数字替换为i，其余符号不替换
		if (isalnum(l.seq_vec[i].first[0])) {
			sym = "i";
			si.push(stoi(l.seq_vec[i].second));
		}
		else
			sym = l.seq_vec[i].first;

		// 将j设置为归约栈中最后一个非终结符的位置
		if (!isupper(svec[k][0]))
			j = k;
		else
			j = k - 1;

		// 进入一个括号内就创建新的栈作为当前栈
		if (sym == "(") {
			++curStack;
			srvec.push_back({});
		}

		// 当归约栈最后一个非终结符算符优先于即将读入的字符，则进行归约
		while ((*OPG_table)[{svec[j], sym}] == ">") {
			string Q;
			int count = 0;	// 计数算符比较的次数
			
			// 这里不得已而为之，因为当要归约(E)时，通过下面循环
			// 找到的串错误，因为( = )，而循环条件为 <，无法维持
			// 循环，而算法保证在读入字符sym为")"时，就会将括号
			// 内的元素归约到E，所以这里直接归约(E)
			if (svec.back() == ")")
				j -= 3;
			else {
				do {
					Q = svec[j];
					if (!isupper(svec[j - 1][0])) {
						// 这里不得已而为之，主要问题是引入了"#"和"("，
						// 其造成的影响是在某些情况下，会造成转换串出错
						// 这里仅仅是为了防止这些情况造成的错误
						if (svec[j - 1] == "#" || svec[j - 1] == "(") {
							if (count <= 1)
								j -= 1;
							else
								break;
						} else {
							j -= 1;
						}
						++count;
					} else if (j - 2 >= 0) {
						// 同上
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
					// 需要注意的是，这里比所给算法多出了一个判断条件，也是为了防止转换串出错
				} while ((*OPG_table)[{svec[j], Q}] == "<" && (*OPG_table)[{svec[j], sym}] == ">");
			}

			string res;	// 转换串
			for (auto it = svec.begin() + j + 1; it < svec.begin() + k + 1; ++it)
				res += *it;
			// 从归约栈中删除转换串
			svec.erase(svec.begin() + j + 1, svec.begin() + k + 1);
			k = j + 1;

			// 从文法中找出含有sym的生成式
			string left, right, nextRight;
			if (sym != "#") {
				for (auto i : *P) {
					for (auto x : i.second) {
						auto y = x.find(sym);
						if (y != string::npos) {
							// 以下判断仅仅针对当前文法
							if (x[0] == '(')
								left = "E";
							else
								left = x.substr(0, y);

							if (y < x.size() - 1)
								// nextRight为当前查找的生成式的右部（如果有）
								// 该右部需要在归约转换串完成后入栈
								nextRight = x.substr(y + 1, x.size());
							break;
						}
					}
					if (!left.empty())
						break;
				}
			} else {
				left = "E";
				nextRight = "";
			}

			// 先尝试直接归约res到left
			if (transform(res, left)) {
				// 成功则压栈，并计算出结果
				svec.push_back(left);
				if (res.find('+') != string::npos) {
					auto x = si.top();
					si.pop();
					auto y = si.top();
					si.pop();
					si.push(x + y);
				} else if (res.find('*') != string::npos) {
					auto x = si.top();
					si.pop();
					auto y = si.top();
					si.pop();
					si.push(x * y);
				}
				// 若转换串为(E)，则删除当前归约辅助栈，回到上一个归约辅助栈
				if (res == "(E)") {
					srvec.erase(srvec.begin() + curStack);
					--curStack;
				}
				// 将当前生成式的右部入栈
				if(!nextRight.empty())
					srvec[curStack].push(nextRight);
			} else {
				// 失败，则去查找右部栈，找到上一个生成式的右部
				if (!srvec[curStack].empty()) {
					right = srvec[curStack].top();
					srvec[curStack].pop();
					// 尝试将转换串的最后一个符号转换成上一个生成式的右部
					if (transform({ *(--res.end()) }, right)) {
						// 成功则替换该符号，并再次尝试转换为left
						res.erase(--res.end()); 
						res.append(right);
						if (transform(res, left)) {
							svec.push_back(left);
							if (res.find('+') != string::npos) {
								auto x = si.top();
								si.pop();
								auto y = si.top();
								si.pop();
								si.push(x + y);
							} else if (res.find('*') != string::npos) {
								auto x = si.top();
								si.pop();
								auto y = si.top();
								si.pop();
								si.push(x * y);
							}
							if (res == "(E)") {
								srvec.erase(srvec.begin() + curStack);
								--curStack;
							}
							if (!nextRight.empty())
								srvec[curStack].push(nextRight);
						}
						else {
							cout << "错误：转换后之后依旧失败" << endl;
							correct = false;
						}
					} else {
						cout << "错误：转换失败" << endl;
						correct = false;
					}
				} else {
					cout << "错误：无法转换" << endl;
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
			cout << "错误：输入匹配失败" << endl;
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
			// left为产生式左部，right为产生式右部
			string left, right;

			// 读入左部，插入非终结符集合，若开始符号S为空，则令该左部为开始符号
			while (isupper(peek) || peek == '\'') {
				left.push_back(peek);
				peek = in.get();
			}

			Vn->insert(left);
			if (S.empty())
				S = left;

			// 跳过空格和->
			while (peek == ' ')
				peek = in.get();

			if (peek == '-' && in.peek() == '>') {
				in.get();
				peek = in.get();
			}

			// 读入该行的所有生成式，并保存
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
				cout << "正确" << endl;
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
				cout << "错误：没有匹配的生成式" << endl;
				while (i < l.seq_vec.size() && l.seq_vec[i].first != ";")
					++i;
			}
		} else {
			cout << "错误：输入既不匹配也不是非终结符" << endl;
			while (i < l.seq_vec.size() && l.seq_vec[i].first != ";")
				++i;
		}
	}
}


// 判断s是否以prefix开头
static inline bool starts_with(const string &prefix, const string &s) {
	return prefix == s.substr(0, prefix.size());
}


// 将s中除except中的每个元素加上后缀suffix
static inline set<string> append_into_set_e(const set<string> &s, const string &suffix, const set<string> &except) {
	set<string> tmp;
	for (string i : s)
		if(except.find(i) == except.end())
			tmp.insert(i + suffix);
	return tmp;
}


// 消除给定生成式的直接左递归
static inline void remove_direct_r(const set<string> &old, map<string, set<string>> &m, const string &left) {
	set<string> s;
	// 先找出所有存在直接左递归的生成式
	for(auto p : old)
		if (starts_with(left, p))
			s.insert(p);
	// 去除这些生成式
	m[left] = append_into_set_e(old, "", s);
	string attach = "'";
	for (auto rb = s.rbegin(); rb != s.rend(); ++rb, attach += "'") {
		// 消除左递归
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
		// 每次迭代，先消除i的直接左递归，然后对[j, i)之间的生成式不断替换i
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
	// 对消除左递归新加入的非终结符添加到Vn中
	for (auto it : m)
		Vn->insert(it.first);
	*P = std::move(old_P);
}


// 太过极端，还没法掌握
void Grammar::_remove_left_recursion() {
	
	auto old_P = *P;
	while (true) {
		// 消除左递归直到无变化
		auto old_nP = *nP;
		remove_left_recursion();
		if (old_nP == *nP) break;
	}
	*P = std::move(old_P);

	// 消除开始符号无法到达的生成式
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


// 提取公因子
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


// 用模板只是因为迭代器类型太长了
template <typename It>
void Grammar::generate_first_aux(const It i) {
	for (auto p : i->second) {
		auto first = front_split(p);
		for (auto f : (*FIRST)[first])
			(*FIRST)[i->first].insert(f);
	}
}


void Grammar::generate_first() {
	// 对于终结符，其FIRST集为自身
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


// 第一个string是生成式中的非终结符，第二个string是每个非终结符的后一个符号
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
		// 对于Ai的每一个生成式进行拆分
		auto follow = back_split(p);
		for (auto fo : follow) {
			// 对于拆分的每个组合构造FOLLOW集
			if (fo.second.empty() || (*FIRST)[fo.second].find("@") != (*FIRST)[fo.second].end()) {
				for (auto f : (*FOLLOW)[i->first])
					(*FOLLOW)[fo.first].insert(f);
				(*FOLLOW)[fo.first].erase("@");
			}
			// 注意这里还要判断，原因是上一个判断的第二个条件
			if (!fo.second.empty()) {
				for (auto f : (*FIRST)[fo.second])
					(*FOLLOW)[fo.first].insert(f);
				(*FOLLOW)[fo.first].erase("@");
			}
		}
	}
}


void Grammar::generate_follow() {
	// 先将#置入开始符号的FOLLOW集
	(*FOLLOW)[S].insert("#");

	for (auto i = nP->begin(); i != nP->end(); ++i) {
		for (auto j = nP->begin(); j != i; ++j)
			// 暴力更新
			generate_follow_aux(j);
		generate_follow_aux(i);
	}
}


// 生成预测分析表
void Grammar::generate_analysis_table() {
	// p : left -> right
	for (auto p : *nP) {
		auto left = p.first;
		for (auto right : p.second) {
			// 对每个非终结符的每个生成式
				for (auto vt : (*FIRST)[right.substr(0,1)]) {
					// 判断其FIRST集中的元素是否为@
					if(vt != "@")
						// 元素为终结符vt
						(*analysis_table)[{left, vt}] = right;
					else
						// 元素为空集@
						for(auto follow : (*FOLLOW)[left])
							(*analysis_table)[{left, follow}] = right;
				}
		}
	}
}


void Grammar::print() const {

	cout << "非终结符号集：" << flush;
	for (auto vn : *Vn) {
		cout << vn << ends;
	}
	cout << endl;
	cout << "终结符号集：" << flush;
	for (auto vt : *Vt) {
		cout << vt << ends;
	}
	cout << std::endl;
	cout << "开始符号" << S << endl;
	cout << "生成式" << endl;
	for (auto p : *P) {
		cout << p.first << " -> " << flush;
		for (auto i : p.second)
			cout << i << "|" << flush;
		cout << endl;
	}

	cout << "消除左递归生成式" <<endl;
	for (auto p : *nP) {
		cout << p.first << " -> " << flush;
		for (auto i : p.second)
			cout << i << "|" << flush;
		cout << endl;
	}

	cout << "FIRST集" << endl;
	for (auto f : *FIRST) {
		cout << f.first << " { " << flush;
		for (auto i : f.second)
			cout << i << ", " << flush;
		cout << "}" << endl;
	}

	cout << "FOLLOW集" << endl;
	for (auto f : *FOLLOW) {
		cout << f.first << " { " << flush;
		for (auto i : f.second)
			cout << i << ", " << flush;
		cout << "}" << endl;
	}

	cout << "预测分析表" << endl;
	for (auto it : *analysis_table) {
		cout << "<" << it.first.first << ", " << it.first.second << "> : " << it.second << endl;
	}

	cout << "FIRSTVT集" << endl;
	for (auto f : *FIRSTVT) {
		cout << f.first << " { " << flush;
		for (auto i : f.second)
			cout << i << ", " << flush;
		cout << "}" << endl;
	}

	cout << "LASTVT集" << endl;
	for (auto f : *LASTVT) {
		cout << f.first << " { " << flush;
		for (auto i : f.second)
			cout << i << ", " << flush;
		cout << "}" << endl;
	}

	cout << "算符优先表" << endl;
	for (auto it : *OPG_table) {
		cout << "<" << it.first.first << ", " << it.first.second << "> : " << it.second << endl;
	}
}

