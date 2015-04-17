#pragma once

#include <list>

#define nothing				do { /* nothing */ } while(false)

#define define_parse(k)		position k::parse(position begin, position end)
#define	define_rule(k,n)	static position \
							rule_##k##n(position begin, position end, base::children_type& children, base::match_type& match)

#define parse_rule(k,n)		rule_##k##n(begin,end,children,match)							

#define parse_start		base *x; position current = begin; nothing
#define parse_final		return std::prev(current); fail: children.clear(); return (end)

#define parse_init(k)	do { x = new k(); current = x->parse(current, end); } while(false)
#define parse_ok(f)		(current != end) { current++; f; }
#define parse_ok_push	parse_ok(children.push_back(x))
#define parse_ok_drop	parse_ok(if(false))
#define else_delete(f)	else { delete x; f; } nothing

#define parse_push_one(k)							\
	do {											\
		parse_init(k);								\
		if parse_ok_push else_delete(goto fail);	\
	} while(false)

#define parse_push_maybe(k)								\
	do {												\
		position rewind = current;						\
		parse_init(k);									\
		if parse_ok_push else_delete(current = rewind);	\
	} while(false)

#define parse_drop_one(k)					\
	do {									\
		parse_init(k);						\
		delete x;							\
		if parse_ok_drop else goto fail;	\
	} while(false)

#define parse_drop_maybe(k)						\
	do {										\
		position rewind = current;				\
		parse_init(k);							\
		delete x;								\
		if parse_ok_drop else current = rewind;	\
	} while(false)

#define declare_rule(k) struct k : base {			\
		const char *name() const { return #k; };	\
		position parse(position, position);			\
	}

namespace xxx {
	namespace standalone {

		////////
		// base

		typedef std::string::const_iterator position;

		struct base {

			typedef std::list<base*> children_type;
			typedef std::string match_type;

			children_type children;
			match_type match;

			virtual position parse(position, position) = 0;
			virtual const char *name() const = 0;

			virtual ~base();

			void clear();
		};

		base::~base() {
			clear();
		}

		void base::clear() {
			while(not children.empty()) {
				delete children.back();
				children.pop_back();
			}
		}
	}
}

//////////////////
// auto-generated

namespace xxx {
	namespace standalone {

		////////////////
		// declarations

		declare_rule(document);
		declare_rule(blocks);
		declare_rule(block);
		declare_rule(eop);

		declare_rule(eof);
		declare_rule(eol);
		declare_rule(_);

		///////////////////////////
		// document = blocks? !eof

		define_parse(document) {
			parse_start;
			parse_push_maybe(blocks);
			parse_drop_one(eof);
			parse_final;
		}

		//////////////////////////
		// blocks = block blocks?

		define_parse(blocks) {
			parse_start;
			parse_push_one(block);
			parse_push_maybe(blocks);
			parse_final;
		}

		///////////////////////
		// eop = !_ !eol !eop? 

		define_parse(eop) {
			parse_start;
			parse_drop_one(_);
			parse_drop_one(eol);
			parse_drop_maybe(eop);
			parse_final;
		}

		///////////////////////////////////////////////////////
		// block = blockquote / codeblock / header / paragraph

		define_parse(block) {
			// parse_start;
			// blockquote
			// /
			// codeblock
			// / 
			// header
			// /
			// paragraph
			// parse_final;
			return begin == end ? end : begin;
		}

		/////
		// _ 

		define_parse(_) {
			position current = begin;
			// /\A[ \t]*/
			return current;
		}

		///////
		// eol

		define_parse(eol) {
			position current = begin;
			// /\A\r?\n/
			return current;
		}

		///////
		// eof

		define_parse(eof) {
			position current = begin;
			// /\A\z/
			return current;
		}
	}
}
