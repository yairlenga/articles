<!-- cSpell:words strcmp asof memcmp charcmp strin FourCC -->
<!-- LTeX: enabled=true -->
<!-- LTeX: ignore strcmp, asof, memcmp -->

# Optimizing Chained `strcmp` Calls for Speed and Clarity

## From `memcmp` and bloom filters to 4CC encoding for small fixed-length string comparisons


While working on a financial modeling system, we started noticing a gradual degradation in performance. In the beginning - nothing dramatic, just a steady increase in runtime, as the code evolved over years of use.

After some profiling, the issue was traced to a core module where a critical part of business logic was encoded. The structure of the code was not an accident - it started small, and grew in complexity over the years - conditions, edge cases, and date-based complexities.

## The Code We Started With

At the center were functions that took action based on currency codes (ISO 3-letter codes), and an as-of date. One specific function was used to load the correct modeling parameters into a configuration structure.
```c
bool model_ccy_lookup(const char *s, int asof, struct model_param *param)
{
    // Major Currencies
    if ( strcmp(s, "USD") == 0 || strcmp(s, "EUR") == 0 || ...) {
        ...
    // Asia-Core
    } else if ( strcmp(s, "CNY") == 0 || strcmp(s, "HKD") == 0 || ... ) {
        ...
    } else if ( ... ) {
        ...
    } else {
        ...
    }
}    
```

The above is a simplified version. The actual logic was more involved, but the structure was the same - long chains of `strcmp`, grouped by common business rules. 

## When It Became a Bottleneck

The slowdown wasn't caused by a single change. It was gradual - over the years. Each added currency introduced a little more work. The chain was organized by "popularity" - major currencies ended up with 2-3 `strcmp` calls, and that was never changed. But the average cost went up, users of the system for 'non-major' currencies noticed a significant and growing performance penalty.

Profiling quickly revealed the issue. The block was executed frequently, and most of the work was repeated `strcmp`. In the worst case scenario, none of the conditions match, which meant repeated sequences of failed `strcmp`.

Refactoring the logic into lookup structure was not a possibility. The conditions were tied to business rules that had to stay explicit, visible and auditable. Specifically, each block had nested if, based on the `asof` date parameter. So many sub-conditions looked like the example below - clean business logic, but expensive to execute.

```c
    if ( strcmp(s, "USD") == 0 || strcmp(s, "EUR") == 0 || ...) {
        *param = major_ccy_param ;
        if ( strcmp(s, "USD") == 0 ) {
            if ( asof < USD_CUTOFF ) {
                param->p1 = ... ;
            }
        } else if ( strcmp(s, "EUR") == 0 ) {
            if ( asof < EUR_CUTOFF ) {
                param->p2 = ... ;
            }
        }
    }    
```

## Making `strcmp` Less Expensive

#### Re-coding the conditions

The first step was not about performance. It was about making the code easier to change.

Instead of writing:

```c
if (strcmp(s, "USD") == 0 || strcmp(s, "EUR") == 0 || ...)
```

we introduced a small helper:

```c
#define CCY_EQ(x, y) (strcmp((x), (y)) == 0)

if (CCY_EQ(s, "USD") || CCY_EQ(s, "EUR") || ...)
```

This didn’t make things faster, but it gave us a single place to experiment with different implementations.

#### Reducing function calls

The first observation was simple: most of these checks fail.

A typical path would evaluate several CCY_EQ calls — sometimes 3–5, sometimes more than 10 — before finding a match or falling through. In those cases, we were paying the cost of a full `strcmp` call each time. (Note: the code was optimized for the "USD" case, which requires only 2 calls, both returning `true`)

Since most comparisons fail early, and a significant part of the cost is the function call itself, we tried a small change: (tagging it as `strcmp`).

```c
static inline bool CCY_EQ(const char *x, const char *ccy) 
{
    return x[0] == ccy[0] && strcmp(x+1, ccy+1) == 0 ;
}
```

This turns each comparison into:
* a cheap first-character check
* followed by a `strcmp` only if needed

In practice, this brought the cost of a failed comparison much closer to a single character test.

#### Faster compare with memcmp

The next observation was also easy - all currency codes are short, and have fixed size - 4 characters (including the terminating NUL). There is no need to use `strcmp` to find the string end. Instead, we compare with fixed number of bytes.

```c
static inline bool CCY_EQ(const char *s, const char *ccy)
{
    return memcmp(s, ccy, 4) == 0;
}
```
As a bonus, `memcmp` with fixed size is typically inlined by the compiler into a fast sequence of load/compare, avoiding function call overhead entirely.

This alone gave noticeable improvement, on top of the char compare + `strcmp` approach.

#### Full inlining

As an experiment, we pushed the idea of single byte compare, and expanded the comparison into explicit character checks:
```c
static inline bool CCY_EQ(const char *s, const char *ccy)
{
   return s[0] == ccy[0] && s[1] == ccy[1] && s[2] == ccy[2] && s[3] == ccy[3] ;
}
```
This had an interesting side effect. Both gcc and clang were able to optimize these expressions quite aggressively — reordering comparisons, and even combining conditions across different branches.

For example, currencies with a common prefix (like "INR" and "IDR") would share part of the decision path, reducing redundant checks.

At this point, we had significantly reduced the cost of each comparison. But the structure of the code was still the same — long chains of conditions — and the total cost still grew with the number of entries.

#### Summary - improving on `strcmp`

The performance gains are impressive - small, local changes to the implementation resulted in up to **5.7X speedup** over `strcmp`. Leveraging compiler optimization takes those improvements further - up to **8X in the best case scenario**.

The benchmark approximates a realistic distribution: most calls target a small set of major currencies, and fewer calls to the other currencies. A smaller number of calls (<1%) were made with currencies that were not handled in the lookup logic.

The test was executed with various compilation flags - both for gcc and for clang: `-Og`, `-O`, `-O2` and the aggressive `-O3`.

The table below summarizes relative performance. The baseline case is `strcmp`, compiled with `-O2`, normalized to 1.0. Higher scores are faster.

```
GCC                   -Og   -O    -O2    -O3
--------------------- ----  ----  ----   ----
ccy-01-strcmp         0.95  1.00  1.00   0.93
ccy-02-strcmp1        1.11  1.97  2.34   6.27
ccy-03-memcmp         0.81  0.87  5.71   5.47
ccy-04-charcmp        1.09  2.57  2.62   8.01
CLANG
ccy-01-strcmp         1.00  1.00  0.99  0.95
ccy-02-strcmp1        3.22  3.22  4.43  4.40
ccy-03-memcmp         5.58  5.34  5.62  5.19
ccy-04-charcmp        4.03  4.68  8.18  7.76
```

#### Key takeaway:
> Even without changing the structure of the code,
> the cost of each failed comparison can be reduced dramatically.
                                                            

## Cleaning It Up — and Breaking Performance

We managed to address the performance problem. Now, it's time to address the code quality problem - make it easier to maintain, audit, and read. The preferred solution was to replace the chained-if with single calls: `CCY_IN`:

```c
// OLD - chained IF
if (CCY_EQ(s, "USD") || CCY_EQ(s, "EUR") || CCY_EQ(s, "JPY") ||
        CCY_EQ(s, "GBP") || CCY_EQ(s, "CHF") || CCY_EQ(s, "CAD") ||
        CCY_EQ(s, "AUD")) {
            ...
        }

// NEW - Similar to SQL "IN" clause.
if (CCY_IN(s, "USD", "EUR", "JPY", "GBP", "CHF", "CAD", "AUD")) {
    ...
}
```

The initial implementation was simple:
```c
static inline bool ccy_in(const char *s, const char **ccy_list)
{
    while (*ccy_list) {      
        if (CCY_EQ(s, *ccy_list))
            return true;
        ccy_list++;
    }
    return false;
}

#define CCY_IN(ccy, ...) ({ \
    static const char *ccy_list[] = { __VA_ARGS__, NULL } ; \
    ccy_in(ccy, ccy_list) ; \
    })
```
But the results were poor - worse than the equivalent chained-if implementation.

## Exploring Alternatives

We tried different directions - and they all hit the same wall.

* Applying the `strcmp` speed-up 'tricks' (the `strcmp1` and `memcmp` variants), improved the results but still significantly below the chained-if approach. Getting the best outcome was challenging - required fine-tuning the various compiler options - which is not ideal for maintainability.
* We tried to "flatten" the data structure - moved from `const char **` to `const char [][4]` - expecting the reduced indirection to improve performance. Result: No impact (test ccy-24-strin-4).
* As an experiment, the code was modified to use a bloom filter to reduce the number of comparisons - making the code more complex. The net effect of higher "fixed cost" associated with every call was lower performance.

The table below summarizes relative performance of various `CCY_IN` implementations. The baseline case is `strcmp`, compiled with `-O2`, normalized to 1.0. Higher scores are faster. The best score (4.8X for the 'memcmp) is almost 2X slower vs the best score of the chained-if approach (8X).

```
GCC                   -Og   -O    -O2    -O3
--------------------- ----  ----  ----   ----
strcmp (BASELINE)     0.95  1.00  1.00   0.93
ccy-21-strin          0.75  0.91  0.91   0.88
ccy-22-strin-4        0.72  0.90  0.89   0.88
ccy-23-strin-cmp1     0.87  2.03  2.08   2.66
ccy-24-strin-memcmp   0.63  0.83  3.41   3.38
ccy-25-strin-filter   1.30  2.83  3.75   3.60
ccy-26-strin-filter4  1.29  2.70  3.74   3.58
CLANG
ccy-21-strin          0.81  0.87  1.01  0.87
ccy-22-strin-4        0.85  0.81  1.01  0.98
ccy-23-strin-cmp1     1.99  1.67  3.00  3.00
ccy-24-strin-memcmp   3.27  2.75  4.80  5.05
ccy-25-strin-filter   2.96  2.40  3.69  3.44
ccy-26-strin-filter4  2.81  2.85  3.35  3.47
```

Notes:

* Tests 'filter' and 'filter4' were using Bloom filter to skip `strcmp`.
* Tests 'strin-4' and 'strin-filter4' were using flat data structure.
* Tests 'strcmp1' and 'memcmp' used the `strcmp` speedup 'tricks' described before.

#### Key Takeaway:

> All of those approaches were still comparing strings, one character at a time. 

## Stop Comparing Strings

Realizing that `strcmp` **is** the bottleneck, we looked at alternatives. We already observed that all strings are short, have the same length, and fit into a 32-bit integer. So we decided to try to use the [FourCC (4cc) Encoding](https://en.wikipedia.org/wiki/FourCC). The basic idea is to pack 4 bytes into an integer, and replace repeated char compares with a single integer comparison. For example: "USD" becomes 0x00534455 (or 0x41524400 on big-endian architectures).

```c
// Before
strcmp(s, "USD")

// After
*(int *) s == 0x00534455
```

The CCY_EQ is now **reinterpreting** the 4-byte strings as an integer:
```c
#define CCY_EQ(x, ccy) (*(int *)x == *(int*) ccy )
```
> This turns each comparison into a single integer load and compare.

Notes:
* On modern X86/X64_86/ARM, it's OK to fetch an integer via an *unaligned* pointer - a key enabler for this approach - sometimes with a minor performance cost.
* Possible to write standard-compliant implementation - slightly noisier.
* No explicit conversion needed.

Even in this basic form, the macro outperforms all previous `strcmp`-based implementations—without requiring compiler optimization. The raw speed gain from comparing 4cc codes as integers is higher than the gains from optimizing the number of calls to `strcmp`.

At this point, we combined the 4cc encoding with the various CCY_IN implementations. This provides better performance vs. the previous CCY_IN that were based on `strcmp`.

#### Hint from `charcmp`

The latest version was still failing to match the performance of the `charcmp` approach, where the `strcmp` was unrolled into a series of single-character comparison. This gave us a hint - *unrolling*. Our code was using loops for membership tests. Can we combine all the findings from the various tests into a clean, performant implementation?

We tried:

```c
#define CCY_EQ(x, ccy) (*(int *)x == *(int*) ccy )

#define CCY_EQ0(ccy, x) (x && CCY_EQ(ccy, x))

#define CCY_IN_8(ccy, x1, x2, x3, x4, x5, x6, x7, x8, x9, ...) \
    CCY_EQ0(ccy, x1) || CCY_EQ0(ccy, x2) || CCY_EQ0(ccy, x3) || CCY_EQ0(ccy, x4) || \
    CCY_EQ0(ccy, x5) || CCY_EQ0(ccy, x6) || CCY_EQ0(ccy, x7) || CCY_EQ0(ccy, x8)

#define CCY_IN(ccy, ...) CCY_IN_8(ccy, __VA_ARGS__, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)

```
Combining all the findings from the previous benchmarks:
* CCY_IN is unrolling the compare.
* CCY_EQ performs a fast integer comparison.
* The compiler short-circuit the chained-if, when less than 8 values are provided.

The result: we have clean source conditions: `if (CCY_IN(s, "USD", "EUR", "JPY", "GBP", ...))`, that get expanded into efficient and highly optimizable code:
```c
int sv = *(int *) s;
if ( sv == 0x00445355 || sv == 0x00525545 || sv == 0x0059504A || sv == 0x00504247 || ...) 
```

#### STR_IN with 4cc encoding:

The table below summarizes the results using 4cc encoding. In all cases, comparing 4cc encoded strings outperforms character by character comparison. Combining it with the CCY_IN construct did NOT have negative performance - it actually improves performance - the final code is running 10X faster vs the initial implementation, and 20% faster vs the chained-if approach.

```
GCC                   -Og   -O    -O2    -O3
--------------------- ----  ----  ----   ----
strcmp (BASELINE)     0.95  1.00  1.00   0.93
ccy-21-strin          0.75  0.91  0.91   0.88
ccy-31-4cc            4.86  6.12  11.26  10.79
ccy-32-4cc-in         2.31  3.90  3.38   3.50
ccy-33-4cc-in4        2.36  4.11  3.55   3.53
ccy-34-4cc-filter     2.41  4.03  5.08   4.86
ccy-35-4cc-filter4    2.43  3.98  5.32   5.63
ccy-36-4cc-opt        4.83  6.42  10.50  10.79
CLANG
ccy-24-strin-memcmp   3.27  2.75  4.80  5.05
ccy-31-4cc            8.56  8.46  8.66  8.76
ccy-32-4cc-in         3.20  3.50  7.68  8.37
ccy-33-4cc-in4        3.59  3.42  9.53  10.18
ccy-34-4cc-filter     5.02  4.48  6.66  6.66
ccy-35-4cc-filter4    4.03  4.51  6.07  6.49
ccy-36-4cc-opt        8.46  8.37  8.66  8.56
```

## What This Taught Me

The project provided an opportunity to explore the topic of `strcmp` performance, which we sometimes treat as a black box. It was also a good experience of trying to balance performance requirements and non-functional requirements (readability, maintainability, auditability):

On the learning side:
* Start with local optimization, change structure when you hit a wall.
* Clean code and high performance are not always in conflict - sometimes it is possible to achieve both.


Recap of ideas:
* The `strcmp` may be expensive, but it does not have to be. When `strcmp` sits at the core of hot code, it's worth asking: what are the alternatives? Few options we visited in the article:

  > Reduce the number of `strcmp` calls by performing some comparison at call site. This is extremely effective if most calls are likely to fail.

      ```c
      static inline bool CCY_EQ(const char *s1, const char *s2) {
        return *s1 == *s2 && strcmp(s1+1, s2+1) == 0 ;
      }
      ```

* Fixed size strings are opportunities for performance improvements. Treating those strings as raw data (as opposed to NUL-terminated strings) unlock strategies for performance improvements:

  > Consider `memcmp` to replace `strcmp` for fixed size strings - the difference is big.

* FourCC (and its big brother EightCC) enable efficient processing of strings (and other data items) - without low-level bit tricks, SIMD wizardry, or hard-to-maintain code. They are simple to implement, and do not require dependency on 3rd party libraries (Reminder: See note about portability).

## Disclaimer

The examples and benchmarks in this article, including linked code snippets, are simplified and reconstructed for illustration purposes. They are not taken from any production system, and do not reflect the design or implementation of any specific codebase.

This is a personal approach based on general experience working with C codebases. It does not represent any official guideline or the opinion of my employer.

As with any low-level technique, evaluate carefully before adopting it in production.

