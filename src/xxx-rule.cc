#include <xxx.hh>

namespace xxx {

    static const std::map<rule::builtin_type, const char *> builtin_to_c_str = {
        { isalnum , "isalnum"  },
        { isalpha , "isalpha"  },
        { iscntrl , "iscntrl"  },
        { isdigit , "isdigit"  },
        { isgraph , "isgraph"  },
        { islower , "islower"  },
        { isprint , "isprint"  },
        { ispunct , "ispunct"  },
        { isspace , "isspace"  },
        { isupper , "isupper"  },
        { isxdigit, "isxdigit" },
        { isascii , "isascii"  },
        { isblank , "isblank"  }
    };

    static const std::map<std::string, rule::builtin_type> string_to_builtin = {
        { "isalnum" , isalnum  },
        { "isalpha" , isalpha  },
        { "iscntrl" , iscntrl  },
        { "isdigit" , isdigit  },
        { "isgraph" , isgraph  },
        { "islower" , islower  },
        { "isprint" , isprint  },
        { "ispunct" , ispunct  },
        { "isspace" , isspace  },
        { "isupper" , isupper  },
        { "isxdigit", isxdigit },
        { "isascii" , isascii  },
        { "isblank" , isblank  }
    };

	std::string rule::str() const {

        std::stringstream ss;

        switch(type) {

            case rule_type::recursive:

                if(not recursive.empty()) {

                    auto iter = recursive.begin();

                    ss << iter->str();

                    while(++iter != recursive.end())
                        ss << ' ' << iter->str();
                }

                break;

            case rule_type::literal:

                ss << '\'' << literal << '\'';
                break;

            case rule_type::builtin:

                ss << builtin_to_c_str.at(builtin) << "()";
                break;

            case rule_type::regex:

                ss << '/' << regex.str().substr(2,std::string::npos) << '/';
                break;
        }

        return ss.str();
	}

	rules rule::singletons(const hints& xs) {

        rules y;

		for(const auto& x : xs)

			y.push_back(rule(x));

		return y;
	}


    //
    // rule::rule
    //

	rule::rule() : type(rule_type::recursive) {
	}

    rule::rule(const hint& h) : type(h.first) {

        switch(type) {

            case rule_type::literal:

                literal = h.second;
                break;

            case rule_type::builtin:

                builtin = string_to_builtin.at(h.second);
                break;

            case rule_type::regex:

                regex.assign("\\A" + h.second, boost::regex::perl);
                break;

            case rule_type::recursive:

                recursive = { predicate(h.second) };
                break;
        }
    }

	rule::rule(const   literal_type& x) : type(rule_type::literal  ) { literal   = x; }
	rule::rule(const   builtin_type& x) : type(rule_type::builtin  ) { builtin   = x; }
	rule::rule(const     regex_type& x) : type(rule_type::regex    ) { regex     = x; }
	rule::rule(const recursive_type& x) : type(rule_type::recursive) { recursive = x; }

    //
    // rule::operator<<
    //

    rule& rule::operator<<(const var& x) {
        return operator<<(predicate(x));
    }

    rule& rule::operator<<(const predicate& x) {

        if(type != rule_type::recursive)
            throw std::runtime_error("rule::type is not rule_type::recursive in rule::operator<<");

        recursive.push_back(predicate(x));

        return *this;
    }

    rule& rule::operator<<(predicate_modifier x) {

        if(type != rule_type::recursive)
            throw std::runtime_error("rule::type is not rule_type::recursive in rule::operator<<");

        if(recursive.empty())
            throw std::runtime_error("cannot assign quantifier to last predicate--recursive rule is empty");

		recursive.back().modifier = x;

		return *this;
	}

	rule& rule::operator<<(const predicate_quantifier& x) {

		if(type != rule_type::recursive)
			throw std::runtime_error("rule::type is not rule_type::recursive in rule::operator<<");

		if(recursive.empty())
			throw std::runtime_error("cannot assign quantifier to last predicate--recursive rule is empty");

		recursive.back().quantifier = x;

		return *this;
	}

}
