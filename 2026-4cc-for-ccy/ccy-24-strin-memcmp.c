/*
 * currency_adjustments.c
 *
 * Example: large strcmp-based dispatch with date-dependent logic
 * (baseline version for optimization discussion)
 */

#include "ccy-lookup.h"

#include <string.h>
#include <stdbool.h>

// #define CCY_EQ(x, ccy) (x[0] == ccy[0] && strcmp(x+1, &(const char *) ccy[1])) == 0)
static inline bool CCY_EQ(const char *s, const char *ccy) {
    return memcmp(s, ccy, 4) == 0;
}

//#define CCY_IN(ccy, ...) ccy_in(ccy, (const char *[]) { __VA_ARGS__, NULL })
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
