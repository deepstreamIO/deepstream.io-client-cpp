# deepstream Message Parser

## Problem Statement

### Requirements

- Can handle UTF-8, e.g., in usernames, event/RPC/record names
- Can parse all deepstream messages sent to clients

### Information

deepstream messages are LL(1):
```
TOPIC|ACTION|DATA+
```
where `|` is ASCII character 31 (unit separator), `+` is ASCII character 30
(record separator). `TOPIC` and `ACTION` are tokens from a fixed set of
alphanumeric strings. `DATA` does not contain `+`; UTF-8 messages encode
multi-byte characters by setting the highest bit the storage locations
occupied by this character so ASCII character 30 and 31 can only occur if these
are used in the string directly.


## Parser Generator vs Handwritten Parser

### Why Use Handwritten Parsers?

- no additional (build time) dependencies
- more simple build process
- no additional knowledge required
- parser generators are unsuited for practical applications
  ([link](http://blog.reverberate.org/2013/09/ll-and-lr-in-context-why-parsing-tools.html))

[Relevant quote](https://www.reddit.com/r/golang/comments/46bd5h/ama_we_are_the_go_contributors_ask_us_anything/d03zx6f/?st=ixa8rnfx&sh=dcabfaa2):
```
Yacc is a great tool, but it complicates the build process, requires maintenance, and means people who wish to work on the compiler must understand another tool and its input language. Translating the compiler to Go from C meantprogrammers didn't have to know C to contribute to the compiler; now they don't have to know yacc either.
There is also strong anecdotal evidence that error messages are easier to make high quality in a recursive descent parser than in a LALR(1) grammar.
And yes, it is a little faster. Yacc's output isn't especially well handled in Go because of all the indexing operations, although I suppose as compiler technology improves that might help.
Overall, it's nice to have one less tool to maintain and a cleaner, all-Go compiler.
```

### List of Potential Parser Generators

- Boost::Spirit
	- C++ with templates
	- too slow because of heavy template use
- Flex, Bison
	- better error handling in Bison:
	  [link](http://stackoverflow.com/questions/22108506/choice-of-parser-generator)
- ANTLR



## How to Write a Parser From Hand

http://blog.reverberate.org/2013/07/ll-and-lr-parsing-demystified.html
