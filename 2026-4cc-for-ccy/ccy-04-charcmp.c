/*
 * currency_adjustments.c
 *
 * Example: large strcmp-based dispatch with date-dependent logic
 * (baseline version for optimization discussion)
 */
#include "ccy-lookup.h"

#include <string.h>

#define CCY_EQ(x, ccy) (x[0] == ccy[0] && x[1] == ccy[1] && x[2] == ccy[2] && x[3] == ccy[3] )

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
    if (CCY_EQ(s, "USD") || CCY_EQ(s, "EUR") || CCY_EQ(s, "JPY") ||
        CCY_EQ(s, "GBP") || CCY_EQ(s, "CHF") || CCY_EQ(s, "CAD") ||
        CCY_EQ(s, "AUD")) {

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
    } else if (CCY_EQ(s, "CNY") || CCY_EQ(s, "HKD") || CCY_EQ(s, "SGD") ||
               CCY_EQ(s, "KRW") || CCY_EQ(s, "TWD")) {

        adj->use_ois_discounting = 1;
        adj->has_government_curve = 0;

        if (CCY_EQ(s, "KRW"))
            adj->spot_settle_days = 1;

    /* ===== 3: asia emerging ===== */
    } else if (CCY_EQ(s, "INR") || CCY_EQ(s, "IDR") || CCY_EQ(s, "MYR") ||
               CCY_EQ(s, "THB") || CCY_EQ(s, "PHP") || CCY_EQ(s, "VND")) {

        adj->use_ois_discounting = 0;

    /* ===== 4: europe non-euro ===== */
    } else if (CCY_EQ(s, "SEK") || CCY_EQ(s, "NOK") || CCY_EQ(s, "DKK") ||
               CCY_EQ(s, "PLN") || CCY_EQ(s, "CZK") || CCY_EQ(s, "HUF") ||
               CCY_EQ(s, "RON")) {

        adj->use_ois_discounting = 1;
        adj->has_government_curve = 1;

    /* ===== 5: latin america ===== */
    } else if (CCY_EQ(s, "MXN") || CCY_EQ(s, "BRL") || CCY_EQ(s, "ARS") ||
               CCY_EQ(s, "CLP") || CCY_EQ(s, "COP") || CCY_EQ(s, "PEN")) {

        adj->use_ois_discounting = 0;

        if (CCY_EQ(s, "BRL"))
            adj->cash_settle_days = 1;

    /* ===== 6: africa ===== */
    } else if (CCY_EQ(s, "ZAR") || CCY_EQ(s, "NGN") || CCY_EQ(s, "KES")) {

        adj->use_ois_discounting = 0;

    /* ===== 7: gulf ===== */
    } else if (CCY_EQ(s, "AED") || CCY_EQ(s, "SAR") || CCY_EQ(s, "QAR") ||
               CCY_EQ(s, "KWD") || CCY_EQ(s, "BHD") || CCY_EQ(s, "OMR")) {

        adj->use_ois_discounting = 0;

    /* ===== 8: middle east / misc ===== */
    } else if (CCY_EQ(s, "ILS") || CCY_EQ(s, "TRY") || CCY_EQ(s, "EGP") ||
               CCY_EQ(s, "MAD")) {

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
