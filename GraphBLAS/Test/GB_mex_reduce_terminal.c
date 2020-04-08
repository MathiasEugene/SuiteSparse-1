//------------------------------------------------------------------------------
// GB_mex_reduce_terminal: [c,flag] = sum(A), reduce to scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Compute c=max(A,x) where all entries in A are known a priori to be <= x.
// x becomes the terminal value of a user-defined max monoid.

#include "GB_mex.h"

#define USAGE "c = GB_mex_reduce_terminal (A, terminal)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GrB_BinaryOp_free (&Max) ;          \
    GrB_Monoid_free (&Max_Terminal) ;   \
    GB_mx_put_global (true, 0) ;        \
}

void maxdouble (double *z, const double *x, const double *y) ;

void maxdouble (double *z, const double *x, const double *y)
{
    // this is not safe with NaNs
    (*z) = ((*x) > (*y)) ? (*x) : (*y) ;
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global (true) ;
    GrB_Matrix A = NULL ;
    GrB_BinaryOp Max = NULL;
    GrB_Monoid Max_Terminal = NULL ;
    GrB_Info info ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 1 || nargin > 2)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    if (A->type != GrB_FP64)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A must be double precision") ;
    }

    // get the terminal value, if present.  Default is 1.
    double GET_SCALAR (1, double, terminal, 1) ;

    // printf ("\nterminal %g\n", terminal) ;

    // create the Max operator
    info = GrB_BinaryOp_new (&Max, maxdouble, GrB_FP64, GrB_FP64, GrB_FP64);
    if (info != GrB_SUCCESS)
    {
        printf ("error: %d %s\n", info, GrB_error ( )) ;
        mexErrMsgTxt ("Max failed") ;
    }

    // printf ("create the monoid:\n") ;

    // create the Max monoid
    info = GxB_Monoid_terminal_new_FP64 (&Max_Terminal, Max, (double) 0,
        terminal) ;
    if (info != GrB_SUCCESS)
    {
        printf ("error: %d %s\n", info, GrB_error ( )) ;
        mexErrMsgTxt ("Max_Terminal failed") ;
    }

    // reduce to a scalar
    double c ;
    info = GrB_Matrix_reduce_FP64 (&c, NULL, Max_Terminal, A, NULL) ;
    if (info != GrB_SUCCESS)
    {
        printf ("error: %d %s\n", info, GrB_error ( )) ;
        mexErrMsgTxt ("reduce failed") ;
    }

    // printf ("result %g\n", c) ;

    // return C to MATLAB as a scalar
    pargout [0] = mxCreateDoubleScalar (c) ;

    FREE_ALL ;
}

