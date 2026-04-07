#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ccy-lookup.h"

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
    int n = argc > 1 ? atoi(argv[1]) : 1000000;

    if ( n<=0 ) { test_all(true); return EXIT_SUCCESS; }

    printf("Running %d iterations...\n", n);
    for (int i=0 ; i<n ; i++) {
        test_all(false);
    }
    return EXIT_SUCCESS ;
}