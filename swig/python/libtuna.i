%{
    #define SWIG_FILE_WITH_INIT
%}

%include "numpy.i"
%fragment("NumPy_Fragments");

%init %{
    import_array();
%}

/* The default typemaps for numpy arrays use `int` for the array length but the
 * C functions in TUNA use `uint` instead as array lengths can't be negative. So
 * we need to add our own typemaps. We can't use `uint` directly so we use the
 * full name `unsigned int`.
 */
%numpy_typemaps(float, NPY_FLOAT, unsigned int)

/* Mapping for window_init_sine(w) where w is a pre-allocated numpy array. */
%apply (float* INPLACE_ARRAY1, unsigned int DIM1) {(float *window, uint length)}

%include "swig/libtuna.i"
