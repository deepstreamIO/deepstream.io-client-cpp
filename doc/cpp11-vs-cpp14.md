# Differences between C++11 and C++14

## Language Differences

- return type deduction: `auto f(int i) { return 2*i; }`
- generic lambda expressions: `auto f = [](auto i) { return 2*i; };`
- digit separators: `int x = 1'000'000;`
- move semantics for lambda captures: `auto f = [&r = x]() { return r;};`
- `[[deprecated]]` attribute: `[[deprecated]] int foo() { return 0; }`
- binary literals: `int i = 0b0001;`
- sized deallocations
- `constexpr` relaxations
- templated variables
- `constexpr` methods are not const

- `shared_mutex`, `shared_lock`
- heterogeneous look-up
- user-defined literals
- tuple addressing via type


### Why `constexpr`-qualified methods are not `const` in C++14

https://akrzemi1.wordpress.com/2013/06/20/constexpr-function-is-not-const/


## Compiler Support

- GCC: gcc >5
