#include <apop.h>

int make_lag_table(){
    apop_table_exists("lags", 'd');
    return apop_query("create table lags as "
        "select now.ln_mu - lag.ln_mu as diff, now.year as year, now.zila "
        "from stats now, stats lag "
        "where now.year = lag.year-1 and now.zila=lag.zila");
}

int main(){
    apop_db_open("b.db");
    Apop_stopif(make_lag_table(), return 1, 0, "Error setting up year-over-year Δs.");

    //Now find lines of best fit for each series of lags. β_0 is mean year-over-year Δ; β_1 is δΔ.
    apop_data *zilas = apop_query_to_text("select admname from ppl");
    apop_query("create table lagslopes (zila, slope, slopeslope);");
    for (int i=0; i< *zilas->textsize; i++){
        apop_data *d=apop_query_to_data("select diff, year-2000 "
                "from lags where zila='%s'", *zilas->text[i]);
        apop_model *e = apop_estimate(d, apop_ols);
        apop_query("insert into lagslopes values('%s', %g, %g);",
        *zilas->text[i], apop_data_get(e->parameters, .row=0, .col=-1), apop_data_get(e->parameters, .row=1, .col=-1));
        printf("%15s %g\n",*zilas->text[i], apop_data_get(apop_estimate(d, apop_ols)->parameters, .row=1, .col=-1));
        //apop_model_print(apop_estimate(d, apop_ols));
    }

}
