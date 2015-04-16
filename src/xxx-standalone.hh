#pragma once

#include <list>

#define define_name(k)		const char *k::name() const { return #k; }
#define define_terminal(k)	bool k::is_terminal() const { return true; }
#define define_parse(k)		position k::parse(position begin, position end)

#define parse_start		base *x; position current = begin; do { /* nothing */ } while(false)
#define parse_final		return std::prev(current); fail: children.clear(); return (end)

#define parse_init(k)	do { x = new k(); current = x->parse(current, end); } while(false)
#define parse_push		(current != end) { current++; children.push_back(x); }
#define parse_drop		(current != end) { current++; }

#define parse_push_one(k)		\
	do {						\
		parse_init(k);			\
		if parse_push else {	\
			delete x;			\
			goto fail;			\
		}						\
	} while(false)

#define parse_push_maybe(k)			\
	do {							\
		position rewind = current;	\
		parse_init(k);				\
		if parse_push else {		\
			delete x;				\
			current = rewind;		\
		}							\
	} while(false)

#define parse_drop_one(k)				\
	do {								\
		parse_init(k);					\
		delete x;						\
		if parse_drop else goto fail;	\
	} while(false)

#define declare_composite(k) struct k : base {	\
		const char *name() const;				\
		position parse(position, position);		\
	}

#define declare_terminal(k) struct k : base {	\
		const char *name() const;				\
		bool is_terminal() const;				\
		position parse(position, position);		\
	}

namespace xxx {
	namespace standalone {

		//////////
		// base //
		//////////

		typedef std::string::const_iterator position;

		struct base {

			std::list<base*> children;
			std::string match;

			virtual position parse(position, position) = 0;
			virtual const char *name() const = 0;

			virtual bool is_terminal() const;
			virtual ~base();

			void clear();
		};

		bool base::is_terminal() const {
			return false;
		}

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

////////////////////
// auto-generated //
////////////////////

namespace xxx {
	namespace standalone {

		//////////////////
		// declarations //
		//////////////////

		declare_composite(document);
		declare_composite(blocks);
		declare_composite(block);

		declare_terminal(eof);

		/////////////////////////////
		// document = blocks? !eof //
		/////////////////////////////

		define_name(document)
		define_parse(document) {
			parse_start;
			parse_push_maybe(blocks);
			parse_drop_one(eof);
			parse_final;
		}

		////////////////////////////
		// blocks = block blocks? //
		////////////////////////////

		define_name(blocks)
		define_parse(blocks) {
			parse_start;
			parse_push_one(block);
			parse_push_maybe(blocks);
			parse_final;
		}

		///////////
		// block //
		///////////

		define_name(block)
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

		/////////
		// eof //
		/////////

		define_name(eof)
		define_terminal(eof)
		define_parse(eof) {
			position current = begin;
			// /\A\z/
			return current;
		}

		document d;
		blocks bs;
		block b;
		eof e;
	}
}
