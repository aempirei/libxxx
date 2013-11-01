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
	const quantifier QSTAR(0,INT_MAX);
	const quantifier QPLUS(1,INT_MAX);
	const quantifier QQUESTION(0,1);
}
