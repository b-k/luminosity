#include <apop.h>

apop_data *make_histo(char const *zila, int year){
    apop_data *d = apop_data_calloc(65,1);
    for (int i=0; i< 64; i++){
        char *col; asprintf(&col, "year_%i_num_intensity_%i", year, i);
        double val=apop_query_to_float("select %s from ppl where "
                    "ADMName='%s'", col, zila);
        Apop_stopif(isnan(val), continue, 0, "couldn't find %s for %s", col, zila);
        apop_data_set(d, i+1, 0, val);
        free(col);
    }
    return d;
}

typedef struct {double yule, kl, mean, ln,lnstderr, lnkl; int error;} ykl_s;

ykl_s make_yule(char const *zila, int y){
    static gsl_matrix *indices;
    if (!indices){
       indices = gsl_matrix_calloc(65,1);
       for (int i=0; i< 64; i++) gsl_matrix_set(indices, i,0, i);
    }
    apop_data *col = make_histo(zila, y);
    apop_data ww = (apop_data){.weights=col->vector, .matrix=indices};
    apop_data *d = apop_data_transpose(col);
    apop_data *exp = apop_data_rank_expand(d);
 apop_model *m = apop_estimate(exp, apop_yule);
 apop_model *n = apop_estimate(exp, apop_lognormal);
    ykl_s out = (ykl_s){
                .yule=apop_data_get(m->parameters, .col=-1/*, .rowname="mu"*/),
                .ln=apop_data_get(n->parameters, .col=-1/*, .rowname="mu"*/),
                .lnstderr=sqrt(apop_data_get(n->parameters, .col=-1, .row=1/*, .rowname="mu"*/)),
                .kl = apop_kl_divergence(apop_estimate(&ww, apop_pmf), m),
                .lnkl = apop_kl_divergence(apop_estimate(&ww, apop_pmf), n),
                .mean = apop_matrix_mean(col->matrix)
    };
    apop_data_free(d);
    apop_data_free(exp);
    apop_model_free(m);
    return out;
}

int main(){
    printf("zila|year|yule_p|kl_div|mu|ln_mu|ln_sigma|ln_kl\n");
    apop_db_open("b.db");
    apop_data *zilas = apop_query_to_text("select admname from ppl");
    for (int i=0; i< *zilas->textsize; i++)
        for (int y=2001; y<= 2005; y++){
            ykl_s ykl = make_yule(*zilas->text[i], y);
            printf("%20s| %i| %g| %g| %g| %g| %g|%g\n", *zilas->text[i], y, ykl.yule, ykl.kl, ykl.mean, ykl.ln, ykl.lnstderr, ykl.lnkl);
        }
    //apop_plot_histogram(m->data->weights, 64, .output_file="histo");
}
