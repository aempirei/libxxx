#include <xxx.hh>

namespace xxx {

    std::string grammar::to_s(string_format_type type) const {
        switch(type) {
            default:
            case string_format_type::xxx: return to_s_xxx(); break;
            case string_format_type::js : return to_s_js (); break;
            case string_format_type::cc : return to_s_cc (); break;
        }
    }

	std::string grammar::to_s_js() const {
        throw std::runtime_error("grammar::to_s_js() unimplemented");
        return "";
    }

	std::string grammar::to_s_cc() const {

        std::stringstream ss;

        ss << "namespace xxx {" << std::endl;
        ss << "\tstatic grammar define_grammar() {" << std::endl;
        ss << "\t\tgrammar g;" << std::endl;
        ss << "\t\tusing M = predicate_modifier;" << std::endl;

        for(const auto& x : *this) {
            switch(x.second.size()) {

                case 0:

                    ss << "\t\tg[\"" << x.first << "\"] = { };" << std::endl;
                    break;

                case 1:

                    ss << "\t\tg[\"" << x.first << "\"] = { " << x.second.front().to_cc() << " };" << std::endl;
                    break;

                default:

                    ss << "\t\tg[\"" << x.first << "\"] = {" << std::endl;
                    for(const auto& r : x.second)
                        ss << "\t\t\t" << r.to_cc() << ',' << std::endl;
                    ss << "\t\t};" << std::endl;
                    break;
            }
        }


        ss << "\t\treturn g;" << std::endl;
        ss << "\t}" << std::endl;
        ss << "}" << std::endl;

        return ss.str();
    }

	std::string grammar::to_s_xxx() const {

        std::stringstream ss;

        for(const auto& x : *this)
            for(const auto& r : x.second)
                ss << x.first << ' ' << (char)r.type << "= " << r.str() << std::endl;

		return ss.str();
	}
}
