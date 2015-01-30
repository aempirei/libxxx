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
        throw std::runtime_error("grammar::to_s_cc() unimplemented");
        return "";
    }

	std::string grammar::to_s_xxx() const {

        std::stringstream ss;

        for(const auto& x : *this)
            for(const auto& r : x.second)
                ss << x.first << ' ' << (char)r.type << "= " << r.str() << std::endl;

		return ss.str();
	}
}
