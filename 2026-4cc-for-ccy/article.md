<!-- cSpell:words strcmp  -->
<!-- LTeX: enabled=true -->
<!-- LTeX: dictionary=en-US: strcmp -->

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

## Cleaning It Up — and Breaking Performance

## Exploring Alternatives

## Stop Comparing Strings

## What This Taught Me