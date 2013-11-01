#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cassert>

#include <unistd.h>

#include <sstream>
#include <iostream>
#include <iomanip>

#include <list>
#include <stack>
#include <thread>
#include <mutex>

#include <liblang.hh>

namespace lang {
	const quantifier q::star      = quantifier(0,INT_MAX);
	const quantifier q::plus      = quantifier(1,INT_MAX);
	const quantifier q::question  = quantifier(0,1);
	const quantifier q::singleton = quantifier(1,1);
}
