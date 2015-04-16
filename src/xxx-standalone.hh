#pragma once

#include <list>

namespace xxx {
	namespace standalone {

		//////////
		// base //
		//////////

		typedef std::string::const_iterator position;

		struct base {

			std::list<base*> children;

			virtual position parse(position, position) = 0;
			virtual const std::string name() const = 0;

			void clear();

			virtual ~base();
		};

		void base::clear() {
			while(not children.empty()) {
				delete children.back();
				children.pop_back();
			}
		}

		base::~base() {
			clear();
		}

		//////////////////
		// declarations //
		//////////////////

		struct document : base {
			const std::string name() const;
			position parse(position begin, position end);
		};

		struct blocks : base {
			const std::string name() const;
			position parse(position begin, position end);
		};

		struct block : base {
			const std::string name() const;
			position parse(position begin, position end);
		};

		struct eof : base {
			const std::string name() const;
			position parse(position begin, position end);
		};

		//////////////
		// document //
		//////////////

		const std::string document::name() const {
			return "document";
		}

		position document::parse(position begin, position end) {

			base *x;

			position current = begin;
			position rewind;

			// blocks?

			rewind = current;
			x = new blocks();
			current = x->parse(current, end);
			if(current != end) {
				current++;
				children.push_back(x);
			} else {
				delete x;
				current = rewind;
			}

			// !eof

			x = new eof();
			current = x->parse(current, end);
			delete x;
			if(current != end) {
				current++;
			} else {
				goto fail;
			}

			//

			return std::prev(current);

			//

fail:

			children.clear();
			return end;
		}

		////////////
		// blocks //
		////////////

		const std::string blocks::name() const {
			return "blocks";
		}

		position blocks::parse(position begin, position end) {

			base *x;

			position current = begin;
			position rewind;

			// block

			x = new block();
			current = x->parse(current, end);
			if(current != end) {
				current++;
				children.push_back(x);
			} else {
				delete x;
				goto fail;
			}

			// blocks?

			rewind = current;
			x = new blocks();
			current = x->parse(current, end);
			if(current != end) {
				current++;
				children.push_back(x);
			} else {
				delete x;
				current = rewind;
			}

			//

			return std::prev(current);

			//

fail:

			children.clear();
			return end;
		}

		///////////
		// block //
		///////////

		const std::string block::name() const {
			return "block";
		}

		position block::parse(position begin, position end) {
			position current = begin;
			// blockquote
			// /
			// codeblock
			// / 
			// header
			// /
			// paragraph
			return current;
		}

		/////////
		// eof //
		/////////

		const std::string eof::name() const {
			return "eof";
		}

		position eof::parse(position begin, position end) {
			position current = begin;
			// /\A\z/
			return current;
		}

		document d;
	}
}



