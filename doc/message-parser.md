# deepstream Message Parser

## Problem Statement

### Requirements

- Can handle UTF-8, e.g., in usernames, event/RPC/record names
- Can parse all deepstream messages (sent to clients)

### Information

The deepstream message format is
```
TOPIC|ACTION|DATA+
```
where `|` is ASCII character 31 (unit separator), `+` is ASCII character 30
(record separator). `TOPIC` and `ACTION` are tokens from a fixed set of
alphanumeric strings. `DATA` does not contain `+` because UTF-8 encodes
multi-byte characters by setting the highest bit the storage locations occupied
by this character so ASCII character 30 and 31 cannot accidentally occur in
message because of the UTF-8 encoding of a non-ASCII character. Thus, deepstream
messages are LL(1); in layman's terms, having an LL(1) grammar means messages
can be parsed by examining the characters sequentially and one at a time.
Furthermore, there is no to use a stack for parsing so messages can be parsed
with regular expressions.


## Parser Generator vs Handwritten Parser

### Why Use Handwritten Parsers?

- no additional (build time) dependencies
- more simple build process
- no additional knowledge required
- parser generators are unsuited for practical applications
  ([link](http://blog.reverberate.org/2013/09/ll-and-lr-in-context-why-parsing-tools.html))

[Relevant quote](https://www.reddit.com/r/golang/comments/46bd5h/ama_we_are_the_go_contributors_ask_us_anything/d03zx6f/?st=ixa8rnfx&sh=dcabfaa2):
> Yacc is a great tool, but it complicates the build process, requires
> maintenance, and means people who wish to work on the compiler must understand
> another tool and its input language. Translating the compiler to Go from C
> meantprogrammers didn't have to know C to contribute to the compiler; now they
> don't have to know yacc either.
>
> There is also strong anecdotal evidence that error messages are easier to make
> high quality in a recursive descent parser than in a LALR(1) grammar.
>
> And yes, it is a little faster. Yacc's output isn't especially well handled in
> Go because of all the indexing operations, although I suppose as compiler
> technology improves that might help.
>
> Overall, it's nice to have one less tool to maintain and a cleaner, all-Go
> compiler.

### List of Potential Compiler-Compilers for Deepstream

A compiler-compiler takes a language grammar and generates code that can parse
the given language. Every regular expression scanner is an example of a
compiler-compiler.

- Boost::Spirit
	- C++ with templates
	- slow compilation because of heavy template use
	- C++ templates => long-winded C++ compiler error messages
- Flex
	- https://github.com/westes/flex/releases
	- standard UNIX tool; its predecessor Lex was co-written in 1975 by Google's
	  Eric Schmidt
	- regular expression scanner
	- generates C code
	- last release: 2.6.3, 2016-12-30 (less than a week ago when this text was
	  written)
- GNU Bison
	- https://www.gnu.org/software/bison/
	- standard UNIX tool; its predecessor Yacc was published in 1975
	- full-blown parser (can, e.g., parse C++)
	- generates C code
	- generated code is permissively licensed instead of GNU GPL
	- last release: 2015-01-23
	- better error handling in Bison:
	  [link](http://stackoverflow.com/questions/22108506/choice-of-parser-generator)



## Hand-Written Parsers

### How to Write a Parser From Hand

http://blog.reverberate.org/2013/07/ll-and-lr-parsing-demystified.html


## Why deepstream Uses Flex

The number of parser states is huge, approximately #topics * #actions * 2 +
payload, where to factor 2 comes from ack messages. As of January 4, 2017, the
deepstream JavaScript client knows based on `src/constants/constants.js` about
20 different events and 35 actions. On the one hand, not all combinations of
events and actions are possible; on the other hand, some events and actions are
encoded with multiple characters in the message and thus, multiple parser
states. Hence, there are roughly 700 possible parser states. If the parser is
implemented manually, the programmer must decide in advanced if she wants to use
a table-based approach or a `goto`-based approach. Given the current parser
state and the next character in the input, the table-based approach uses a
look-up table to determine the next state and the corresponding code is executed
using a switch statement:
```
	while(*input)
	{
		switch(state)
		{
			case STATE_1:
				// handle STATE_1
				state = STATE_TABLE[state][*input];
				break;
			// ...
		}
	}
```
For comparison, a sketch of a `goto`-based approach:
```
STATE_1:
	switch(*input)
	{
		case '\0':
			goto CASE_EOF;
		case 'E':
			// handle 'E'
			goto STATE_E;
	}
```
Clearly, a hand-written parser is a massive effort and there is no way to, e.g.,
visualize the parser states (`bison --graph=<filename>`) or get warnings which
input may cause performance loss (`flex --perf-report`).

We tried both GNU Bison and Flex. They require similar implementation effort so
we tested the speed. Both generators had to parse
```
E|A|S|foo|bar|baz+
```
2^20 times. On a MacBook Air, Flex initially required 0.5s while Bison took only
0.25s-0.3s. Flex copies its input and uses free storage (malloc/realloc/free)
while Bison does not copy its input and uses memory allocation on the stack with
`alloca()`. Moving the string copying out of the inner loop made Flex
competitive with Bison (ca. 0.3s).

Christoph Conrads decided to use Flex because
- regular expressions are sufficient for deepstream messages,
- many programmers know how to use regular expressions,
- finite state machines are taught to every computer scientist whereas only
  compiler courses explain LL(k), LALR(k), LR(k), and GLR parsers,
- Flex allows the programmer to hook into its memory management, and
- the scanner generator can be instructed to trade time for space.
The more elaborate theory behind GNU Bison can cause error messages to be
incomprehensible to the average programmer (keywords here are shift/shift,
shift/reduce, and reduce/reduce conflicts). Being able to hook into memory
management means we could, for instance, statically allocate memory for Flex
seeing that it is called repeatedly.
