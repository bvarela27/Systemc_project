#ifndef SVD
#define SVD

int Singular_Value_Decomposition(double* A, int nrows, int ncols, double* U, double* singular_values, double* V, double* dummy_array);
void Singular_Value_Decomposition_Solve(double* U, double* D, double* V, double tolerance, int nrows, int ncols, double *B, double* x);

#endif
