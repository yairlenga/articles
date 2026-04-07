#ifndef _CCY_CHECK_H
#define _CCY_CHECK_H

#include <stdbool.h>

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

#endif