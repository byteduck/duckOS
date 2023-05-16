# Common Patterns

This document goes over some patterns commonly found in code and how they should be implemented in duckOS.

For information on how to format and document code, see [style.md](style.md).

## Objects, Reference Counting, and Smart Pointers

Use reference-counted smart pointers (`kstd::Arc` in the kernel and `Duck::Ptr`) for dynamically allocated objects that will be passed around. Make sure to use weak pointers (`kstd::Weak` / `Duck::WeakPtr`) where applicable to avoid strong reference loops.

### Duck::Object

Classes (in userspace) that will always be used with reference counting should typically inherit from `Duck::Object` (see [libraries/libduck/Object.h](libraries/libduck/Object.h)). For instance, `Gfx::Image` inherits from `Duck::Object` since we want to be able to pass around a pointer to an image and automatically destroy it once there are no more references to it.

When inheriting from `Duck::Object`, you must do the following:
- Use the `DUCK_OBJECT_DEF(ClassName)` macro in the public section of your class's definition.
  - If defining an abstract class with pure virtual methods, use `DUCK_OBJECT_VIRTUAL(ClassName)` instead.
- Make all initializers private.
- (Optional) Privately override the `initialize` method of `Duck::Object`. This will be called after an instance of your class is constructed.
  - If inheriting from a base class that derives from `Duck::Object`, make sure to call `SuperClass::initialize()` in your `initialize` method if you override it.
  - This is important because `self()`, which obtains a smart pointer to the object, will not function in initializers.

Then, you can instantiate new instances of the class with `ClassName::make(args...)`. This will return a strong smart pointer (`Duck::Ptr`) to the instantiated object. Within the class, the `self()` method can be used to get a strong reference to `this`.

**NEVER** use raw pointers to pass around classes derived from `Duck::Object`.

**NEVER** directly call the constructors of classes derived from `Duck::Object`.

**ALWAYS** use `self()` from within classes derived from `Duck::Object` to pass a smart pointer of `this` to something.

###### Example:

```cpp
/// A cool class
class MyClass: public Duck::Object {
public:
	DUCK_OBJECT_DEF(MyClass)
	
	void bar();
	
private:
	MyClass() = default;
	
	void initialize() override {
		// Use self() to get a smart pointer to `this`
		Duck::Ptr<MyClass> cool_ptr = self();
	}
};

/// Makes a new instance of `MyClass`.
void foo() {
	// You could use `auto`, but we explicitly define the type here for illustration purposes.
	Duck::Ptr<MyClass> my_class = MyClass::make();
	
	// my_class will automatically be deallocated here, unless we stored a reference to it somewhre else!
}
```

## Singletons

Singleton classes should be used when functionality is best wrapped into a class, but there should only be one instance.

All singleton classes should have a static function named `inst` that returns a reference to the instance of the singleton, creating it if it does not exist yet.

###### Example:

```cpp
class CoolClass {
public:
	static CoolClass& inst() {
		if(!s_inst)
			s_inst = new CoolClass();
		return *s_inst;
	}
	
private:
	static CoolClass* s_inst = nullptr;
};
```

## Error handling

In functions and methods that may result in an error, return a `Duck::Result` (`kstd::Result` in the kernel), which can contain an integer error code, a string describing the error, or both. If you also need to return something along with the result in the case of a success, use `Duck::ResultRet<T>` (`kstd::ResultRet<T>` in the kernel).

When calling a function that returns a `Result` or `ResultRet` from a function that returns a `Result` or `ResultRet`, you can use the `TRY` macro to automatically return from the function if an error occurs.

###### Example:
```cpp
using namespace Duck;

// Returns either an int, or an error.
ResultRet<float> foo() {
	if(something)
		return Result(2, "Something bad happened."); // An error code of 2 with a message
	else
		return 12.3; // Success! Return 12.3 and a successful result.
		
}

Result bar() {
	// Call foo. If foo returns an error, return the same error. Otherwise, continue.
	float baz = TRY(foo());
	
	// Do something...
	
	// Success!
	return Result::SUCCESS;
}

// We could clean this up by using the TRY macro.
Result bar_2() {
	// This functions identically to the code in bar().
	ResultRet<float> baz_err = foo();
	if(baz_err.is_error())
		return baz_err.result();
	float baz = baz_err.value();
	
	// Do something...
	
	// Success!
	return Result::SUCCESS;
}

```

## Delegates

Try to use the [delegate pattern](https://en.wikipedia.org/wiki/Delegation_pattern) in cases where some class needs to rely on a helper class for some functionality outside of the regular pattern of inheritance. This is comparable to the use of interfaces in other languages.

For instance, `UI::TableView` is a good example of the delegate pattern. When using a TableView, you can inherit from `UI::TableViewDelegate` and use `table_view->set_delegate(my_delegate)` to set the TableView's delegate. The TableView then calls out to the delegate when determining the number of rows, the contents of the cells, etc.

Ideally, delegate methods should be named with some obvious prefix. Additionally, you may want to pass a reference to the object being delegated as the first argument in case one delegate class may be delegating multiple of the same type of class.

**ALWAYS** reference delegates in the delegated class with weak pointers to avoid reference loops.

###### Example:

In this example, `FooClass` and its companion delegate class, `FooDelegate`, handle functionality for printing the name of some object that implements `FooDelegate`. Meanwhile, `BarClass` implements `FooDelegate`.

```cpp
class FooDelegate {
public:
	virtual const char* fc_name() = 0;
};

class FooClass: public Duck::Object {
public:
	DUCK_OBJECT_DEF(FooClass);
	
	void say_hi() {
		// Make sure the weak reference isn't null...
		ASSERT(!m_delegate.expired());
		
		// Get our name from the delegate and print it!
		auto name = m_delegate.lock()->fc_name(self());
		printf("Hi! My name is %s.\n", name);
	}
	
	void set_delegate(Duck::Ptr<FooDelegate> delegate) {
		m_delegate = delegate;
	}
	
private:
	Duck::WeakPtr<FooDelegate> m_delegate;
};

class BarClass: public Duck::Object, public FooDelegate {
public:
	DUCK_OBJECT_DEF(BarClass);
	
	const char* fc_name() override {
		return "Bar";
	}
	
	void print_foo_name() {
		// Make a new foo and set ourselves as its delegate.
		auto my_foo = FooClass::make();
		my_foo->set_delegate(self());
		
		// Ask the foo to print its name.
		my_foo->print_name();
	}
};
```