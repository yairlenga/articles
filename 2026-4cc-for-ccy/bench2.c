
#include <stdbool.h>

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* =========================
 * Enums and structs from original code
 * ========================= */

typedef enum rate_index {
    RATE_INDEX_NONE = 0, RATE_INDEX_LIBOR_3M, RATE_INDEX_SOFR, RATE_INDEX_SONIA,
    RATE_INDEX_SARON, RATE_INDEX_TONA, RATE_INDEX_ESTR
} rate_index_t;

typedef enum govt_curve_rule {
    GOVT_RULE_NONE = 0, GOVT_RULE_UST, GOVT_RULE_BUND, GOVT_RULE_GILT,
    GOVT_RULE_JGB, GOVT_RULE_CANADA
} govt_curve_rule_t;

typedef struct currency_adjustments {
    const char       *ccy;
    int  spot_settle_days;
    int  cash_settle_days;
    bool use_ois_discounting;
    bool has_government_curve;
    rate_index_t discount_index;
    rate_index_t projection_index;
    govt_curve_rule_t govt_rule;
    int reform_cutoff_ymd;
} currency_adjustments_t;

static inline void currency_adjustments_init(currency_adjustments_t *adj)
{
    adj->ccy = "";
    adj->spot_settle_days = 2;
    adj->cash_settle_days = 2;
    adj->use_ois_discounting = 0;
    adj->has_government_curve = 0;
    adj->discount_index = RATE_INDEX_NONE;
    adj->projection_index = RATE_INDEX_NONE;
    adj->govt_rule = GOVT_RULE_NONE;
    adj->reform_cutoff_ymd = 0;
}

/* =========================
 * Optimized Handler Logic
 * ========================= */

// Signature for the rule functions
typedef bool (*handler_fn)(int today_ymd, currency_adjustments_t *adj);

// 3D array for fast O(1) lookup. 26x26x26 = 17576 elements.
static handler_fn mapper[26][26][26];

// --- Specific Handlers ---

static bool handle_none(int today_ymd, currency_adjustments_t *adj) {
    return false;
}

static bool handle_USD(int today_ymd, currency_adjustments_t *adj) {
    adj->has_government_curve = 1;
    adj->use_ois_discounting = 1;
    adj->govt_rule = GOVT_RULE_UST;
    adj->reform_cutoff_ymd = 20230701;
    if (today_ymd < adj->reform_cutoff_ymd) {
        adj->discount_index   = RATE_INDEX_LIBOR_3M;
        adj->projection_index = RATE_INDEX_LIBOR_3M;
        adj->use_ois_discounting = 0;
    } else {
        adj->discount_index   = RATE_INDEX_SOFR;
        adj->projection_index = RATE_INDEX_SOFR;
    }
    return true;
}

static bool handle_EUR(int today_ymd, currency_adjustments_t *adj) {
    adj->has_government_curve = 1;
    adj->use_ois_discounting = 1;
    adj->govt_rule = GOVT_RULE_BUND;
    adj->discount_index   = RATE_INDEX_ESTR;
    adj->projection_index = RATE_INDEX_ESTR;
    return true;
}

// Generic handler for simple groups (e.g., Asia Emerging, Africa, Gulf, Mid-East, S-Asia)
static bool handle_no_ois_no_govt(int today_ymd, currency_adjustments_t *adj) {
    adj->use_ois_discounting = 0;
    return true;
}

static bool handle_europe_non_euro(int today_ymd, currency_adjustments_t *adj) {
    adj->use_ois_discounting = 1;
    adj->has_government_curve = 1;
    return true;
}

// ... Add the remaining specific handlers here (JPY, GBP, CHF etc.) ...

/* =========================
 * Initialization and Lookup
 * ========================= */

static inline int char_idx(char c) {
    return (unsigned char)c - 'A';
}

void init_currency_mapper(void) {
    static bool initialized = false;
    if (initialized) return;

    // Set all to default fail-handler
    for (int i = 0; i < 26; i++) {
        for (int j = 0; j < 26; j++) {
            for (int k = 0; k < 26; k++) {
                mapper[i][j][k] = handle_none;
            }
        }
    }

    // Register specific currencies
    mapper[char_idx('U')][char_idx('S')][char_idx('D')] = handle_USD;
    mapper[char_idx('E')][char_idx('U')][char_idx('R')] = handle_EUR;

    // Register groups (Asia Emerging example)
    const char *asia_emerging[] = {"INR", "IDR", "MYR", "THB", "PHP", "VND"};
    for (int i = 0; i < 6; i++) {
        const char *c = asia_emerging[i];
        mapper[char_idx(c[0])][char_idx(c[1])][char_idx(c[2])] = handle_no_ois_no_govt;
    }

    // Register groups (Europe Non-Euro)
    const char *eur_non[] = {"SEK", "NOK", "DKK", "PLN", "CZK", "HUF", "RON", "NZD"};
    for (int i = 0; i < 8; i++) {
        const char *c = eur_non[i];
        mapper[char_idx(c[0])][char_idx(c[1])][char_idx(c[2])] = handle_europe_non_euro;
    }

    initialized = true;
}

int currency_lookup(
    const char *s,
    int today_ymd,
    currency_adjustments_t *adj)
{
    // Ensure valid input and exactly 3 characters
//    if (!s || !adj || s[0] == '\0' || s[1] == '\0' || s[2] == '\0' || s[3] != '\0') 
//        return 0;

    int i0 = char_idx(s[0]);
    int i1 = char_idx(s[1]);
    int i2 = char_idx(s[2]);

    // Fast boundary check for A-Z
//    if ((unsigned int)i0 > 25 || (unsigned int)i1 > 25 || (unsigned int)i2 > 25) 
//        return 0;

    currency_adjustments_init(adj);
    adj->ccy = s;

    // Jump directly to the rule (O(1))
    return mapper[i0][i1][i2](today_ymd, adj) ? 1 : 0;
}

/* =========================
 * Main function (for testing)
 * ========================= */
 int test_all(bool verbose)
{
    currency_adjustments_t adj;
    int today = 20240115;

    /* ---- currencies handled by the function ---- */
    static const char *valid_codes[] = {
        "USD","EUR","JPY","GBP","CHF","CAD","AUD",
        "CNY","HKD","SGD","KRW","TWD",
        "INR","IDR","MYR","THB","PHP","VND",
        "SEK","NOK","DKK","PLN","CZK","HUF","RON",
        "MXN","BRL","ARS","CLP","COP","PEN",
        "ZAR","NGN","KES",
        "AED","SAR","QAR","KWD","BHD","OMR",
        "ILS","TRY","EGP","MAD",
        "NZD",
        "PKR","BDT"
    };

    /* ---- valid ISO codes NOT handled above ---- */
    /* these will take the worst path (fail all strcmp checks) */
    static const char *miss_codes[] = {
        "ISK","HRK","BGN","UAH","RUB",
        "LKR","NPR","MMK","KZT","UZS",
        "GHS","TZS","UGX","RWF","XOF",
        "XAF","JMD","DOP","BBD","BMD"
    };

    int i;

    for (i = 0; i < (int)(sizeof(valid_codes)/sizeof(valid_codes[0])); i++) {
        int i=0 ;
        if (currency_lookup(valid_codes[i], today, &adj)) {
            if ( verbose ) printf("%s -> OK (discount=%d, settle=%d)\n",
                   valid_codes[i],
                   adj.discount_index,
                   adj.spot_settle_days);
        } else {
            printf("%s -> NOT FOUND (unexpected)\n", valid_codes[i]);
        }
        for (int j=0 ; j<15-i ; j++) currency_lookup(valid_codes[i], today, &adj) ;

    }

    for (i = 0; i < (int)(sizeof(miss_codes)/sizeof(miss_codes[0])); i++) {
        int i=0 ;
        if (!currency_lookup(miss_codes[i], today, &adj)) {
            if ( verbose ) printf("%s -> NOT FOUND (expected)\n", miss_codes[i]);
        } else {
            printf("%s -> FOUND (unexpected)\n", miss_codes[i]);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int n = argc > 1 ? atoi(argv[1]) : 100000;

    init_currency_mapper();
    if ( n<=0 ) { test_all(true); return EXIT_SUCCESS; }

    printf("Running %d iterations...\n", n);
    for (int i=0 ; i<n ; i++) {
        test_all(false);
    }
    return EXIT_SUCCESS ;
}
