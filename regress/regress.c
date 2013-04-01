#include <apop.h>
int main (){
    apop_db_open("b.db");
    apop_data *d = apop_query_to_data(
        "select distinct ppl.'2001_upper_poverty',ln_mu,slope,/*slopeslope, p_ag__labour_t, */"
        "population/area/1000 dens, p_elec__con_t ,  p_urban_t "
        "from stats, ppl,lagslopes "
        "where stats.zila = ppl.admname and stats.year=2005 "
        "and stats.zila = lagslopes.zila "
        "and mu < 5 "
        "and dens < 5 "
        /*"and ppl.'2005_upper_poverty'> -1 "
        "and ppl.'2005_upper_poverty' is not null"*/
            );
    apop_data_print(d, .output_file="ddd");
    apop_data_show(apop_data_correlation(d));
    apop_model_print( apop_estimate(d , apop_ols));
}
