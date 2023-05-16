# Code style and Best Practices

This is not an exhaustive list, so please try to familiarize yourself with the general coding style of duckOS when contributing :)

For information on common coding patterns, like error handling and reference counting, see [common_patterns.md](common_patterns.md).

Your IDE can help automatically format code for you by using the formatting specified in the included `.clang-format` file.

## General Formatting
- Use the included `.clang-format` in your IDE for automatic formatting.
- Use tabs for indentation.
- Break at 120 lines.
    - If a declaration, function call, etc, goes over 120 lines, split each argument into its own line and align to the opening parenthesis.

## Naming Conventions
Case:
- Name variables / methods / functions in `snake_case`.
- Name structs / classes / namespaces in `UpperCamelCase`.
- Name constants and constant defines with `SCREAMING_CASE`. 

Names:
- Give class member variables names starting with `m_`.
- Give static class variables names starting with `s_`.
- Give global variables names starting with `g_`.

###### Example:
```cpp
#define FOO 4

int g_foo = FOO;

struct CoolStruct {
	int cool_int;
};

class CoolClass {
public:
	static bool s_cool_flag = true;
	int get_foo();
	
private:
	int m_foo;
};
```

## Pointers and References

Add a space after (but not before) `*` and `&`. In other words, attach `*` and `&` to the type, not the name. This helps make it clear whether the variable is a reference, pointer, or not.

###### Example:
```cpp
int& foo;
void* bar;
char* baz[];
```

### Classes and Structs

As a general rule of thumb, use classes for anything with complex functionality and structs for bits of data that need to be conveniently grouped together.

Data members in classes should be named with the `s_` and `m_` prefixes detailed in the naming section above. Data members in structs should not be prefixed.

Typically, it's good practice to make member variables in classes private, and to add public getters and setters (however applicable) for them.

Classes in userspace that will always be referenced with smart pointers should inherit from `Duck::Object`. See [common_patterns.md](common_patterns.md) for more details.

###### Example:
```cpp
class MyClass {
public:
	// It's okay to implement short, simple getters and setters like these in the header.
	void set_foo(int foo) { m_foo = foo; }
	int get_foo() { return foo; }
	
private:
	int m_foo;
};
```

## "using" statements

Please do not put `using` statements in header files.

Please try to avoid `using` statements in general. However, it is okay to use them in implementation files, but only for the two most commonly used namespaces: `Duck` and `kstd`.

## Omission of braces in control flow

It is okay to omit braces from control flow statements - but only if there is a SINGLE line following it, including comments. This way, it is visually obvious which control flow statements lines of code belong to.

###### Correct:
```cpp
if(foo)
	bar();

while(foo)
	bar();

if(foo) {
	// We need to bar.
	bar();
}
```

###### Okay:
```cpp
if(foo) {
	bar();
}
```

###### Wrong:
```cpp
if(foo)
	// We need to bar.
	bar();
```

## TODO
When something needs to be finished at some point in the future, simply leave a comment that starts with `FIXME:` or `TODO:` and describe what needs to be done or fixed.

###### Example
```cpp
// Chosen by fair dice roll. Guaranteed to be random.
int random() {
	// TODO: This seems to throw some things off, figure out why...
	return 4;
}
```

## Code Documentation

Please try to document your code when you can. Use [Doxygen-Style](https://www.jetbrains.com/help/clion/creating-and-viewing-doxygen-documentation.html) documentation comments to embed documentation in comments.

Make sure to describe what functions / methods do, what their parameters are for, and what they return. Describe variables and classes with triple-slash (`///`) comments.

###### Example:
```cpp
/**
 * Does a foo. This information will show in some IDEs.
 * @param bar The bar to use.
 * @return A foo value.
 */
int foo(int bar);

/// The baz value. This information will show in some IDEs.
int baz;

/// A foobar class. This information will show in some IDEs.
class FooBar { ... };
```

## Ultimately,

A lot of the code in this codebase is old and doesn't follow the formatting to a tee - if you're working on some code that's formatted wrong, feel free to fix it. You don't need to fix code formatting on files you aren't touching, though.