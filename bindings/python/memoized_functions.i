%module(package="pyspiceql") memoized_functions


%{
  #include "memoized_functions.h"
%}

%rename(Memo_ls) SpiceQL::Memo::ls;
%rename(Memo_getTimeIntervals) SpiceQL::Memo::getTimeIntervals;
%rename(Memo_translateNameToCode) SpiceQL::Memo::translateNameToCode;
%rename(Memo_translateCodeToName) SpiceQL::Memo::translateCodeToName;

%include "memoized_functions.h"