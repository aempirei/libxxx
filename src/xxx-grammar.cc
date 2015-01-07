#include <xxx.hh>

namespace xxx {

	static std::string grammar_rules_str(const rules&);

	std::string grammar_str(const grammar& g) {

        std::stringstream ss;

		for(const auto& x : g)
            ss << x.first << " := " << grammar_rules_str(x.second) << std::endl;


		return ss.str();
	}

	static std::string grammar_rules_str(const rules& rs) {

		std::stringstream ss;

        if(not rs.empty()) {

            auto iter = rs.begin();

            ss << iter->str();

            while(++iter != rs.end())
                ss << " / " << iter->str();
		}

		return ss.str();
	}
}
