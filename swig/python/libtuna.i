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

/* Mapping for `frames_out = buffer_acquire(frames_in)`. */
%apply (unsigned int *INOUT) {(uint *frames)}

/* Mapping for `tol_get_coeffs(w)` where w is a pre-allocated numpy array. */
%apply (float * INPLACE_ARRAY1, unsigned int DIM1) {(float *dest, uint length)}

%include "swig/libtuna.i"

/* Wrapper for `tol_calculate(t, data, results)` where both data and results are
 * pre-allocated numpy arrays.
 */
%apply (float * IN_ARRAY1, unsigned int DIM1) {(float *data, uint data_length)}
%apply (float * INPLACE_ARRAY1, unsigned int DIM1) {(float *results,
                                                    uint results_length)}
%rename (tol_calculate) tol_calculate_wrapper;

%inline %{
void tol_calculate_wrapper(struct tol * t, float * data, uint data_length,
                           float * results, uint results_length)
{
        /* Ignore array lengths. */
        __unused data_length;
        __unused results_length;
        tol_calculate(t, data, results);
}
%}
