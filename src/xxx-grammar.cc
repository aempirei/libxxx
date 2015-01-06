#include <xxx.hh>

namespace xxx {

	static std::string grammar_mapped_type_str(const grammar::mapped_type&);

	std::string grammar_str(const grammar& g) {

		std::string s;

		for(const auto& x : g) {
			s += x.first;
			s += ((x.second.front().type == rule_type::terminal) ? " ~= " : " := ");
			s += grammar_mapped_type_str(x.second);
		}


		return s;
	}

	static std::string grammar_mapped_type_str(const grammar::mapped_type& rs) {

		std::stringstream ss;

        if(not rs.empty()) {

            auto iter = rs.begin();

            ss << iter->str();

            while(++iter != rs.end())
                ss << " / " << iter->str();
		}

		ss << std::endl;

		return ss.str();
	}
}
