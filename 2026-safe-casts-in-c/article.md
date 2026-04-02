# Safer Casting in C — Without Runtime Cost

C gives us two types of casting:
* Implicit casting - happens automatically in expressions (e.g. expressions mixing integer and floating point values), and in function calls (when values are converted to the parameter values)
* Explicit casting with the cast operator `(T) v` (e.g. `(int) x`) that can convert almost anything into anything else — and that’s exactly the problem.

Both introduce risks, with the explicit conversion happily converting:
* `T *` to/from `T **`,
* Arrays treated as pointers,
* Qualifiers stripped silently.

These are not theoretical issues. In real systems, they show up as memory corruption, subtle bugs, and crashes that are hard to trace back to the original cast. Given the grammar of `(T) v`, these casts are easy to miss in code reviews.

## Another problem: cast precedence

C casts have very high precedence - higher than most operators. This means they bind tightly to the expression that follows.

Example:
```c
char *p = ... ; 
long x = *(long *) p + 1;
```
is parsed as
```c
long x = (*(long *) p) + 1;                // Option A
```
A developer unfamiliar with exact precedence rules might expect something closer to one of the following:
```c
long x = *((long *) (p+1)) ;               // Option B (1 byte forward)
long x = *((long *) (p+sizeof(long))) ;    // Option C (next long)
```

These expressions look similar, but behave very differently:
* The first (A) reads a long from p, then adds 1
* The second (B) reads a long from p+1
* The third (C) reads a long from p+sizeof(long)

Because casts bind tightly, small changes in parentheses can silently change meaning - making such bugs hard to spot in code review.

Compare this to SQL's `CONVERT(type, value)` which makes conversions obvious and searchable. In this case one would write:
```c
long x = *CAST(long *, p) + 1 ;            // Add one to the long from p
long x = *CAST(long *, p+1) ;              // Pick the long starting one byte after p.
long x = *CAST(long *, p+sizeof(long)) ;   // Pick the second long from p
```

The goal is not to prevent all invalid casts — but to make incorrect ones fail early, and valid ones easy to audit.

## A Simple Idea
Replace `(T) v` with function-like macros that:
* Make casts visible
* Enforce basic correctness at compile time
* Add zero-runtime cost

This is not "Type-Safe C" - just "harder-to-misuse C".

## Example - Implicit conversion bug:

The following small program should print the absolute value of the first argument. Calling the wrong absolute value function results in implicit truncation and bad calculation.
```c
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
        double v = atof(argv[1]) ;
        double abs_v = abs(v) ; 
        printf("ABS(X)=%f\n", abs_v) ;
}
```
It compiles, but is wrong - `abs()` expects and returns `int`, so `v` is implicitly converted: double-> int -> double. This truncates the fractional part.

Output - expecting 3.14, getting 3.0
```
./a.out -3.14
ABS(X)=3.000000
```

# Structured CAST macros

## Design Goals
* Make casts visible and searchable.
* Catch common mistakes
* Zero runtime cost
* Simple syntax and drop-in usage

## Proposed API

```c
CAST(T, v)           /* generic entry point */
CAST_VAL(T, val)     /* scalar / arithmetic values */
CAST_PTR(T, ptr)     /* any pointer */
CAST_PTR1(T, ptr)    /* Same as CAST_PTR, limit to single-level pointers */
UNCONST_PTR(ptr)     /* remove qualifiers from the pointee types */
UNCONST_PTR1(ptr)    /* same as UNCONST_PTR, limit to single-level pointers */
```

## API Description
### CAST(T, v)

Generic entry point. This is the macro to use when the code should read like a normal cast, but still go through the structured API. The main value of CAST is readability: all explicit conversions now go through one visible, searchable API.

Usually, this is the first step - replace the hidden casts `(T) v`, with CAST(T, v).

Example:
```c
int x = CAST(int, y);
void *p = CAST(void *, buf);
```

### CAST_VAL(T, val)

Used for value conversions: integers, floating-point values, enums, booleans, and similar arithmetic cases.

Example:
```c
int n = CAST_VAL(int, d);
double x = CAST_VAL(double, count);
```
This macro should reject pointer expressions, so pointer-to-integer or pointer-to-float mistakes do not silently slip through. The macro is similar to C++ `static_cast<T>(v)`, so if your team maintain both C++ and C code, you may even name is `STATIC_CAST`.

### CAST_PTR(T, ptr)

Used for pointer conversions where the only requirement is that `ptr` is a pointer expression.

Example:
```
void *p = CAST_PTR(void *, src);
char *s = CAST_PTR(char *, p);
```
This macro is intentionally lightweight. Its main job is to separate pointer casts from value casts, making them easy to audit. It will reject argument which is NOT a pointer. The macro is somewhat similar to C++ `reinterpret_cast<T>(p)`, which allows reinterpretation of raw points, so if your team maintain both C++ and C code, you may name it `REINTERPRET_CAST`.

### CAST_PTR1(T, ptr)

Used for single-level pointers only. In other words, `ptr` must be a T *-style pointer, not T **.

Example:
```c
void *buf = ...
char *s = CAST_PTR1(char *, buf);
struct node *n = CAST_PTR1(struct node *, p);
```
Typical invalid cases:
```c
char **pp = ... ;
char *p = CAST_PTR1(char *, pp);   /* should fail */
```
This is useful because a common hard-to-detect cast mistake in real code is accidentally mixing T * and T ** (or other level of nesting, array-ness)

### UNCONST_PTR(ptr)

Removes the 'const' qualifier from the pointee type of a pointer expression.

Example:
```c
const char *p = get_text();
    // We know we can modify the value, get UNCONST_PTR pointer
char *q = UNCONST_PTR(p) ;
```

This is intentionally narrow: it is for pointer-to-data cases such as const T * -> T *. It does not try to be a general-purpose “remove const from anything” macro.

That restriction is intentional. Applying “unconst” to plain values or even structs by value is not useful; it handle the common case when we want to change the mutability of referenced changes. Common use case is when a function return a value that must be freed - but we want to treat the value itself is immutable.
```c
   const char *x = get_value() ;
   do_something(x) ;
   free(UNCONST_PTR(x)) ;     // free requires void *
```

### UNCONST_PTR1(ptr)

Explicit form of `UNCONST_PTR` for the common `T *` case. This makes the intent even clearer: remove qualifiers from a first-level pointer, not from nested pointers.

Example:
```c
const struct header *h;
struct header *mh = UNCONST_PTR1(h);
```
In general, for most cases, we want to use UNCONST_PTR1, which indicate that we expect the pointer to non-mutable object. The macro rejects nested pointers.

## Before / After
```c
/* before */
int n = (int) x;
char *p = (char *) buf;
free((void *) s);

/* after */
int n = CAST_VAL(int, x);
char *p = CAST_PTR1(char *, buf);
free(UNCONST_PTR1(s));
```

All conversions are now:
* Visible
* Structured
* Easy to grep and audit

# Combining with static checkers

Beside making the conversion more readable/searchable, it is possible to combine those macros with strict checking by enabling the conversion warnings with gcc/clang. Basic idea:
1. compile with `gcc -Wconversion`, which warns on: Implicit narrowing (long -> int, int->short), signed/unsigned conversions and float/int.
2. Review each conversion. Fix broken conversions, add explicit conversions with CAST, CAST_PTR1, UNCONST_PTR1 as needed.
3. Raise conversion to warning to errors with `-Werror=conversion`

At this point, any code changes that result in implicit or illegal conversions (as per Macro restrictions) will be flagged as an error - preventing unexpected surprises at run time.

For even stronger validation - consider enabling gcc/clang cast related warnings (`-Wcast-qual`, `-Wcast-align`, ...). You can also get additional mileage using gcc/clang analyzer, and additional tools like `clang-tidy` or commercial tools like `coverity`.

# Implementation

Each of the macro is implemented with an expression that will force compile time checks, with zero run-time checks. Because "C" does not have standard for some of the checks (e.g. is_pointer) - we are using compiler extension that exists in gcc/clang. If you use other compilers that do not support gcc/clang extensions - It's possible to implement *SOME* of the restrictions with C23.

It's important to highlight one aspect of those macro - they capture the *intended usage of the cast*. While current implementation/compilers might not be able to fully enforce the restriction - future implementation and compiler versions might include new extensions, and future updates to the C standard might support stronger enforcement.

You can retrieve implementation of those macro for gcc 13, and clang 18, that I validated on Ubuntu 2024 (under WSL) from GITHUB gist, and drop it into your code base. single header file.

Below is short description for the implementation of CAST, CAST_VAL. Other macros implementation follow the same line. 

## Implementing the basic CAST
```c
#define CAST(T, v) ((T) v)
```

## Implementing the CAST_VAL(T, val)

The implementation of all macros that enforce restriction on the converted value is to use 2 expressions:
- The first expression enforce the restriction at compile time, but has no run-time effect.
- The second expression perform the conversion, using the basic CAST
```c
#define CAST_VAL(T, v) (CAST_REQUIRE_VALUE(v), CAST(T, v))

static inline void cast_require_value(double v) { (void) v; }

#define CAST_REQUIRE_VALUE(v) ((void)sizeof(cast_require_value(v)))
```
The `cast_require_value` function does nothing - but it will only accept `double` value. In "C", scalar numeric values - integer, floating-pointer, enum, ... will be promoted to double as needed. Therefore, the call to `cast_require_value` will succeed if a value is passed, and will fail with non-value - specifically - pointers, unions, structures, ...

We mentioned that we want ZERO run-time effect. This is achieved by applying the `sizeof` of the restriction. Per C langauge rules - sizeof does NOT evaluate the expression - it calculate (at compile time) the size (both gcc and clang evaluate `sizeof(void)` to 1).

# Summary

C casting is extremely powerful - and that’s exactly the problem. Both implicit and explicit casts can silently introduce bugs that are hard to detect and even harder to trace.

This article proposes a simple approach:
* Replace `(T) v` with function-like macros
* Make conversions visible and searchable
* Catch common mistakes at compile time
* Keep zero runtime cost

The goal is not to make C “type-safe”, but to make casting:
* Explicit
* Auditable
* Harder to misuse

Sometimes, the biggest improvements in C don’t come from new language features — but from better discipline, encoded in small reusable patterns.

# Caveats and Limitations

This approach is intentionally lightweight and pragmatic, but it comes with a few important caveats:

### Compiler support

The implementation relies on GCC/Clang extensions (e.g. type checks via expressions that trigger compile-time errors).  
It has been tested with GCC 13 and Clang 18, but may not work — or may require adjustments — on other compilers.

### Not a complete type system

These macros do not make C type-safe.  
They only enforce a limited set of structural constraints (e.g. pointer vs value, pointer depth).

Incorrect casts are still possible — but many common mistakes become compile-time errors.

### Error messages

Some invalid uses will produce compiler errors that are not always intuitive, especially when triggered through macro expansion.

This is a trade-off for zero runtime cost and portability.

### Requires discipline

The benefits come from **consistent usage**:
* Replace `(T)v` with `CAST(...)`
* Enable strict warnings (`-Wconversion`, `-Werror`)

Partial adoption reduces effectiveness.

### Test before adoption

This approach should be validated in your codebase:
* Check compatibility with your compiler/toolchain
* Evaluate error messages and developer experience
* Ensure it integrates well with existing coding guidelines

# Disclaimer

This is a personal approach based on practical experience working with C codebases.  
It does not represent any official guideline or the opinion of my employer.

As with any low-level technique, evaluate carefully before adopting it in production.