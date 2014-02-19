%{
    #define SWIG_FILE_WITH_INIT
%}

%include "numpy.i"
%fragment("NumPy_Fragments");

%init %{
    import_array();
%}

%include "swig/libtuna.i"
