#!/bin/bash

OUT="$(basename $1 .pp).output"
RES="$(basename $1 .pp).result"

./checkit $1 2>&1 >${OUT}

if [[ ! -f ${RES} ]]; then
  echo expected result is missing
  exit 1
elif ! cmp ${OUT} ${RES}; then
  diff -u ${OUT} ${RES}
  exit 1
fi

exit 0
