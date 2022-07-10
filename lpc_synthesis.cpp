#include "lpc_synthesis.h"

//Decoder Function
arma::vec lcpDecode(arma::vec A, double *GFE){

    double G = GFE[0];
    double F = GFE[1];

    arma::vec xhat(WINDOW_LENGTH_2);
    xhat.zeros();

    arma::vec src(WINDOW_LENGTH_2);
    src.zeros();

    uint16_t step = 0;
    static uint16_t offset = 0;

    if (F>0)
    {
        step = round(1/F);
        if (step < WINDOW_LENGTH_2)
        {
            for(uint16_t i = offset; i<WINDOW_LENGTH_2; i+=step)
            {
                src(i) = sqrt(step);
                offset = step + i - WINDOW_LENGTH_2+1;
                //std::cout<< i << "OFF      "<<offset<<std::endl;
            }
        }
    }

    else
    {
        offset = 0;
        for(uint16_t i = 0; i<WINDOW_LENGTH_2; i++)
        {
            src = arma::randn(WINDOW_LENGTH_2);
        }
    }
    arma::vec A_0(1.0);
    A_0(0) = -1;
    arma::vec b = {1.0};
    A=join_cols(A_0,A);
    xhat = filter(b,A,sqrt(G)*src);


    /*if (lowcut > 0){
        std::cout<<"butterworth..."<<std::endl;
        xhat = filter(b_butterworth,a_butterworth,xhat);
    }*/

    return xhat;
}

//Filter Function
arma::vec filter(arma::vec b, arma::vec a, arma::vec X){

    if (b.n_rows < a.n_rows)
    {
        b = arma::join_vert(b,arma::zeros<arma::vec>(a.n_rows-1));
    }
    else if(b.n_rows > a.n_rows){
        a = arma::join_vert(a,arma::zeros<arma::vec>(b.n_rows-1));
    }

    int n = a.n_rows;

    arma::vec z(n);
    z.zeros();

    arma::vec Y(X.n_rows);
    Y.zeros();

    b = b/a(0);
    a = a/a(0);

    if(a.n_rows>1)
    {
        for(uint16_t m = 0; m < Y.n_rows; m++)
        {
            Y(m) = b(0)*X(m) + z(0);
            for(uint16_t i = 1; i < n-1; i++)
            {
                z(i-1) = (b(i)*X(m))+z(i)-(a(i)*Y(m));
            }

            z(n-2)=(b(n-1)*X(m)) - (a(n-1)*Y(m));
        }
        //z(a.n_rows)
    }
    else
    {
        for(uint16_t m = 0; m < X.n_rows; m++)
        {
            Y(m) = b(0)*X(m) + z(0);

            for(uint16_t i = 1; i < n; i++)
            {
                z(i-1) = b(i)*X(m)+z(i);
            }
            z(n-2)=b(n-1)*X(m);
        }
    }


    //z = z.rows(1,n-1);

    return Y;
}