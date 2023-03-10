%module(package="pyspiceql") memoized_functions


%{
  #include "memoized_functions.h"
%}

%rename(Memo_ls) SpiceQL::Memo::ls;
%rename(Memo_getTimeIntervals) SpiceQL::Memo::getTimeIntervals;

%include "memoized_functions.h"