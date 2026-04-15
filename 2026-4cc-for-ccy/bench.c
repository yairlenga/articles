// Single-File benchmark for the article:
//	Optimizing Chained strcmp Calls for Speed and Clarity -
//	From memcmp and bloom filters to 4CC encoding for small fixed-length string comparisons
//
// Compile with:
//  gcc -O2 -g bench.c -o fast -march=native
//  gcc -O2 -g bench.c -o orig -march=native -DORIG
// Run:
//     ./fast          # Execute Optimzied version - 100_000 iterations
//     ./fast 200000   # Execute 200_000 iterations
//     ./orig           # Execute Baseline - 100_000 iterations
//     ./orig 200000    # Execute 200_000 iterations
//
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* =========================
 * Enums
 * ========================= */

typedef enum rate_index {
    RATE_INDEX_NONE = 0,
    RATE_INDEX_LIBOR_3M,
    RATE_INDEX_SOFR,
    RATE_INDEX_SONIA,
    RATE_INDEX_SARON,
    RATE_INDEX_TONA,
    RATE_INDEX_ESTR
} rate_index_t;

typedef enum govt_curve_rule {
    GOVT_RULE_NONE = 0,
    GOVT_RULE_UST,
    GOVT_RULE_BUND,
    GOVT_RULE_GILT,
    GOVT_RULE_JGB,
    GOVT_RULE_CANADA
} govt_curve_rule_t;

/* =========================
 * Structure
 * ========================= */

typedef struct currency_adjustments {
    const char       *ccy;

    int  spot_settle_days;
    int  cash_settle_days;

    bool use_ois_discounting;
    bool has_government_curve;

    rate_index_t discount_index;
    rate_index_t projection_index;

    govt_curve_rule_t govt_rule;

    int reform_cutoff_ymd; /* YYYYMMDD */
} currency_adjustments_t;

int currency_lookup(
    const char *s,
    int today_ymd,
    currency_adjustments_t *adj);

#ifndef ORIG

	// Optimized Version

#define CCY_EQ(x, ccy) (*(int *)x == *(int*) ccy )

#define CCY_EQ0(ccy, x) (x && CCY_EQ(ccy, x))

#define CCY_IN_8(ccy, x1, x2, x3, x4, x5, x6, x7, x8, x9, ...) \
    CCY_EQ0(ccy, x1) || CCY_EQ0(ccy, x2) || CCY_EQ0(ccy, x3) || CCY_EQ0(ccy, x4) || \
    CCY_EQ0(ccy, x5) || CCY_EQ0(ccy, x6) || CCY_EQ0(ccy, x7) || CCY_EQ0(ccy, x8)

#define CCY_IN(ccy, ...) CCY_IN_8(ccy, __VA_ARGS__, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)

#else

	// Basic Version
static inline bool CCY_EQ(const char *s, const char *ccy)
{
    return strcmp(s, ccy) == 0 ;
}

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

#endif

/* =========================
 * Main function
 * ========================= */

/* Example code for article: simplified, self-contained, not from any production system */

#include <stdlib.h>

static void currency_adjustments_init(currency_adjustments_t *adj) ;

/* =========================
 * Main function
 * ========================= */

int currency_lookup(
    const char *s,
    int today_ymd,
    currency_adjustments_t *adj)
{
    if (s == NULL || adj == NULL)
        return 0;

    currency_adjustments_init(adj);
    adj->ccy = s;

    /* ===== 1: majors ===== */
    if (CCY_IN(s, "USD", "EUR", "JPY", "GBP", "CHF", "CAD", "AUD")) {

        adj->has_government_curve = 1;
        adj->use_ois_discounting = 1;

        if (CCY_EQ(s, "USD")) {
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

        } else if (CCY_EQ(s, "EUR")) {
            adj->govt_rule = GOVT_RULE_BUND;
            adj->discount_index   = RATE_INDEX_ESTR;
            adj->projection_index = RATE_INDEX_ESTR;

        } else if (CCY_EQ(s, "JPY")) {
            adj->govt_rule = GOVT_RULE_JGB;
            adj->reform_cutoff_ymd = 20220101;

            if (today_ymd < adj->reform_cutoff_ymd) {
                adj->discount_index   = RATE_INDEX_LIBOR_3M;
                adj->projection_index = RATE_INDEX_LIBOR_3M;
                adj->use_ois_discounting = 0;
            } else {
                adj->discount_index   = RATE_INDEX_TONA;
                adj->projection_index = RATE_INDEX_TONA;
            }

        } else if (CCY_EQ(s, "GBP")) {
            adj->govt_rule = GOVT_RULE_GILT;
            adj->reform_cutoff_ymd = 20220101;

            if (today_ymd < adj->reform_cutoff_ymd) {
                adj->discount_index   = RATE_INDEX_LIBOR_3M;
                adj->projection_index = RATE_INDEX_LIBOR_3M;
                adj->use_ois_discounting = 0;
            } else {
                adj->discount_index   = RATE_INDEX_SONIA;
                adj->projection_index = RATE_INDEX_SONIA;
            }

        } else if (CCY_EQ(s, "CHF")) {
            adj->reform_cutoff_ymd = 20220101;

            if (today_ymd < adj->reform_cutoff_ymd) {
                adj->discount_index   = RATE_INDEX_LIBOR_3M;
                adj->projection_index = RATE_INDEX_LIBOR_3M;
                adj->use_ois_discounting = 0;
            } else {
                adj->discount_index   = RATE_INDEX_SARON;
                adj->projection_index = RATE_INDEX_SARON;
            }

        } else if (CCY_EQ(s, "CAD")) {
            adj->govt_rule = GOVT_RULE_CANADA;

        } else {
            /* AUD */
            adj->has_government_curve = 0;
        }

    /* ===== 2: asia core ===== */
    } else if (CCY_IN(s, "CNY", "HKD", "SGD", "KRW", "TWD")) {

        adj->use_ois_discounting = 1;
        adj->has_government_curve = 0;

        if (CCY_EQ(s, "KRW"))
            adj->spot_settle_days = 1;

    /* ===== 3: asia emerging ===== */
    } else if (CCY_IN(s, "INR", "IDR", "MYR", "THB", "PHP", "VND")) {

        adj->use_ois_discounting = 0;

    /* ===== 4: europe non-euro ===== */
    } else if (CCY_IN(s, "SEK", "NOK", "DKK", "PLN", "CZK", "HUF", "RON")) {

        adj->use_ois_discounting = 1;
        adj->has_government_curve = 1;

    /* ===== 5: latin america ===== */
    } else if (CCY_IN(s, "MXN", "BRL", "ARS", "CLP", "COP", "PEN")) {

        adj->use_ois_discounting = 0;

        if (CCY_EQ(s, "BRL"))
            adj->cash_settle_days = 1;

    /* ===== 6: africa ===== */
    } else if (CCY_IN(s, "ZAR", "NGN", "KES")) {

        adj->use_ois_discounting = 0;

    /* ===== 7: gulf ===== */
    } else if (CCY_IN(s, "AED", "SAR", "QAR", "KWD", "BHD", "OMR")) {

        adj->use_ois_discounting = 0;

    /* ===== 8: middle east / misc ===== */
    } else if (CCY_IN(s, "ILS", "TRY", "EGP", "MAD")) {

        adj->use_ois_discounting = 0;

    /* ===== 9: singleton ===== */
    } else if (CCY_EQ(s, "NZD")) {

        adj->use_ois_discounting = 1;
        adj->has_government_curve = 1;

    /* ===== 10: south asia ===== */
    } else if (CCY_EQ(s, "PKR") || CCY_EQ(s, "BDT")) {

        adj->use_ois_discounting = 0;

    } else {
        return 0;
    }

    return 1;
}

static void currency_adjustments_init(currency_adjustments_t *adj)
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
 * Test driver
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

    if ( n<=0 ) { test_all(true); return EXIT_SUCCESS; }

    printf("Running %d iterations...\n", n);
    for (int i=0 ; i<n ; i++) {
        test_all(false);
    }
    return EXIT_SUCCESS ;
}
