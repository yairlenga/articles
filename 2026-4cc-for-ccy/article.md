<!-- cSpell:words strcmp asof memcmp -->
<!-- LTeX: enabled=true -->
<!-- LTeX: ignore strcmp, asof, memcmp -->

# Optimizing Chained strcmp Calls for Speed and Clarity — Without Refactoring
## From memcmp and bloom filters to 4CC and compile-time expansion

## The Code We Started With

While working on a financial modeling system, we started noticing a gradual degradation in performance. In the beginning - nothing dramatic - just steady increase in runtime, as the code evolved - over years of usage.

After some profiling the issue was traced to a core module where a critical part of business logic was encoded. The structure of the code was not an accident - it started small, and grew in complexity over the years - conditions, edge cases and history.

At the center were functions that took action based on currency code (ISO 3-letter currency code), and as-of date. One specific function was used to load the correct modeling parameters into a configuration structure.
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

The above is simplified version. The actual logic was more involved, but the structure was the same - long chains of strcmp, grouped by common business rules. 

## When It Became a Bottleneck

The slowdown wasn't caused by a single change. It was gradual - over the years. Each added currency introduced a little more work. The chain was organized by "popularity" - major currencies ended up with 2-3 strcmp, and that never was changed. But the average cost went up, users using the system for "non-major" currencies noticed significant and growing performance penalty.

Profiling quickly revealed the issue. The block was executed frequently, and most of the work was repeated strcmp. In the worst case scenario, none of the condition match, which meant repeated sequences of failed strcmp.

Refactoring the logic into lookup structure was not a possibility. The conditions were tied to business rules that had to stay explicit, visible and auditable. Specifically, each block had nested if, based on the `asof` date parameter. So many sub-conditions looked like below - clean business logic, but expensive to execute.

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

### Re-coding the conditions

The first step was not about performance. It was about making the code easier to change.

Instead of writing:

```c
if (strcmp(s, "USD") == 0 || strcmp(s, "EUR") == 0 || ...)
```

we introduced a small helper:

```
#define CCY_EQ(x, y) (strcmp((x), (y)) == 0)

if (CCY_EQ(s, "USD") || CCY_EQ(s, "EUR") || ...)
```

This didn’t make things faster, but it gave us a single place to experiment with different implementations.

### Reducing function calls

The first observation was simple: most of these checks fail.

A typical path would evaluate several CCY_EQ calls — sometimes 3–5, sometimes more than 10 — before finding a match or falling through. In those cases, we were paying the cost of a full `strcmp` call each time. (Note: the code was optimized for the "USD" case, which require only 2 calls, both returning `true`)

Since most comparisons fail early, and a significant part of the cost is the function call itself, we tried a small change:

```
static inline bool CCY_EQ(const char *x, const char *ccy) 
{
    return x[0] == ccy[0] && strcmp(x+1, ccy+1) == 0 ;
}
```

This turns each comparison into:
* a cheap first-character check
* followed by a `strcmp` only if needed

In practice, this brought the cost of a failed comparison much closer to a single character test.

### Faster compare with memcmp

The next observation was also easy - all currency codes are short, and have fixed size - 4 characters (including the terminating null). There is no need to use `strcmp` to find the string end. Instead of we compare with fixed number of bytes.
```
static inline bool CCY_EQ(const char *s, const char *ccy)
{
    return memcmp(s, ccy, 4) == 0;
}
```
As a bonus, `memcmp` with fixed is typically inlined by the compiler into a fast sequence of load/compare, avoiding function call overhead entirely.

This a lone gave noticeable improvement, on top of the char compare + `strcmp` approach.

### Full inlining

As an experiment, we pushed the idea of single byte compare, and expanded the comparison into explicit character checks:
```
static inline bool CCY_EQ(const char *s, const char *ccy)
{
   return s[0] == ccy[0] && s[1] == ccy[1] && s[2] == ccy[2] && s[3] == ccy[3] ;
}
```
This had an interesting side effect. Both gcc and clang were able to optimize these expressions quite aggressively — reordering comparisons, and even combining conditions across different branches.

For example, currencies with a common prefix (like "INR" and "IDR") would share part of the decision path, reducing redundant checks.

At this point, we had significantly reduced the cost of each comparison. But the structure of the code was still the same — long chains of conditions — and the total cost still grew with the number of entries.

## Cleaning It Up — and Breaking Performance

## Exploring Alternatives

## Stop Comparing Strings

## What This Taught Me