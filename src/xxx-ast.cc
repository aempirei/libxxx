#include <xxx.hh>

namespace xxx {

	static range<ssize_t> parse_recursive(const grammar&, std::string, const std::string&, ast&, ssize_t);
	static std::string ast_str_recursive(const ast&, int, bool);
	static std::string ast_xml_recursive(const ast&, int);

	ast::ast() {
	}

	ast::ast(const grammar& g, FILE *fp) {
		parse(g, fp);
	}

	ast::ast(const grammar& g, const std::string& s) {
		parse(g, s);
	}

	void ast::parse(const grammar& g, FILE *fp) {

		std::string s;

		char buf[512];

		while(!feof(fp)) {

			int n = fread(buf, 1, sizeof(buf), fp);

			if(n != sizeof(buf) && ferror(fp)) {
				std::string s("fread(): ");
				s += strerror(errno);
				throw new std::runtime_error(s);
			}

			s.append(buf, n);
		}

		parse(g, s);
	}

	void ast::parse(const grammar& g, const std::string& s) {

		range<ssize_t> ab = parse_recursive(g, "document", s, *this, 0);

		if(ab.first == -1) {

			std::stringstream ss;
			ss << "ast::parse failed at offset " << ab.second;

			std::cerr  << s.substr(ab.second, std::string::npos);

			throw std::runtime_error(ss.str());
		}
	}

	static range<ssize_t> parse_recursive(const grammar& g, std::string name, const std::string& s, ast& q, ssize_t offset) {

		const auto iter = g.find(name);

		if(iter == g.end()) {
			std::stringstream ss;
			ss << "grammar rule not found -- \"" << name << '"';
			throw std::runtime_error(ss.str());
		}

		const auto& rules = iter->second;

		ssize_t current = offset;

		q.offset = offset;
		q.name = name;

		for(const auto& rule : rules) {

			bool success = true;

			boost::smatch matches;
			std::string ms;

			current = offset;

			q.children.clear();
			q.type = rule.type;

			switch(rule.type) {

				case rule_type::terminal:

					ms = s.substr(offset, std::string::npos);

					if(!boost::regex_search(ms, matches, rule.terminal)) {
						success = false;
						break;
					}

					q.matches.resize(matches.size());

					for(size_t n = 0; n < matches.size(); n++)
						q.matches[n] = matches[n];

					current += q.matches[0].size();

					break;

				case rule_type::recursive:

					for(const auto& predicate : rule.recursive) {

						size_t n;

						for(n = 0; n < predicate.quantifier.second; n++) {

							ast qq;

							auto next = parse_recursive(g, predicate.name, s, qq, current);

							if(next.first == -1)
								break;

							current = next.second;

							if(predicate.modifier == predicate_modifier::push) {

								q.children.push_back(qq);

							} else if(predicate.modifier == predicate_modifier::lift) {

								if(qq.type != rule_type::recursive)
									throw new std::runtime_error("attempting to lift non-recursive ast node");

								for(const auto& qqq : qq.children)
									q.children.push_back(qqq);

							} else if(predicate.modifier == predicate_modifier::discard) {
								// discard
							}
						}

						if(n < predicate.quantifier.first) {
							success = false;
							break;
						}
					}

					break;

				default:

					throw std::runtime_error("undefined rule type");
			}

			if(success)
				return range<ssize_t>(offset,current);
		}

		return range<ssize_t>(-1,current);
	}

	static std::string ast_str_recursive(const ast& q, int depth=0, bool basic=false) {

		std::stringstream ss;

		if(basic) {
			ss << q.name;
		} else {
			ss << std::setw(4) << q.offset << " " << std::setw(depth) << "" << q.name;
		}
		switch(q.type) {
			case rule_type::terminal:
				ss << " =~ " << '"' << q.matches[0] << '"' << std::endl;
				break;
			case rule_type::recursive:

				if(q.children.size() == 1) {
					ss << ' ' << ast_str_recursive(q.children.back(), depth + 2, true);
				} else {

					ss << std::endl;

					for(const auto& qq : q.children)
						ss << ast_str_recursive(qq, depth + 2);
				}

				break;
		}

		return ss.str();
	}

	std::string ast::str() const {
		return ast_str_recursive(*this);
	}

	static std::string ast_xml_recursive(const ast& q, int depth=0) {

		std::stringstream ss;

		switch(q.type) {

			case rule_type::terminal:

				ss << std::string(depth, ' ') << '<' << q.name << '>';
				ss << q.matches[0];
				ss << "</" << q.name << '>' << std::endl;

				break;

			case rule_type::recursive:

				ss << std::string(depth, ' ') << '<' << q.name << '>' << std::endl;

				for(const auto& qq : q.children)
					ss << ast_xml_recursive(qq, depth + 1);

				ss << std::string(depth, ' ') << "</" << q.name << '>' << std::endl;

				break;
		}

		return ss.str();
	}


	std::string ast::xml() const {
		return ast_xml_recursive(*this);
	}


}
