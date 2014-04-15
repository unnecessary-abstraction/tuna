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

/* Mapping for `threshold_out = onset_threshold_next(onset, next, threshold_in)`
 */
%apply (float *INOUT) {(env_t * threshold)}

/* Mapping for 'psd = fft_power_spectrum(cdata)' where psd is to be returned as
 * a numpy array.
 */
%apply (float * ARGOUT_ARRAY1, unsigned int DIM1) {(float * data, uint n)}

/* Ignore fft_get_data so that we can later add a wrapper with an identical
 * signature.
 */
%ignore fft_get_data;

%include "swig/libtuna.i"

/* Wrapper for `tol_calculate(t, data, results)` where both data and results are
 * pre-allocated numpy arrays.
 */
%apply (float * IN_ARRAY1, unsigned int DIM1) {(float complex *data, uint data_length)}
%apply (float * INPLACE_ARRAY1, unsigned int DIM1) {(float *results,
                                                    uint results_length)}
%rename (tol_calculate) tol_calculate_wrapper;

%inline %{
void tol_calculate_wrapper(struct tol * t, float complex * data, uint data_length,
                           float * results, uint results_length)
{
        /* Ignore array lengths. */
        __unused data_length;
        __unused results_length;
        tol_calculate(t, data, results);
}
%}

/* Wrapper for 'w = fft_get_data(fft)' where w is to be returned as a numpy
 * array.
 */
%apply (float ** ARGOUTVIEW_ARRAY1, unsigned int * DIM1) {(float ** data, uint * data_length)}

%rename (fft_get_data) fft_get_data_wrapper;

%inline %{
void fft_get_data_wrapper(struct fft * fft, float ** data, uint * data_length)
{
        *data = fft_get_data(fft);
        *data_length = fft_get_length(fft);
}
%}
