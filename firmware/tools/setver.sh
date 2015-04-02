#!/bin/bash

build=$((`grep BUILD version.h | sed 's|.*BUILD\s\([0-9]\{1,9\}\).*|\1|'`+1))
sed  -i "s|^.*BUILD.*$|#define BUILD ${build}|" version.h

commit=$(git log | grep -c '^commit')
sed  -i "s|^.*COMMIT.*$|#define COMMIT ${commit}|" version.h

compile_date=`date -u`

sed -i "s|^// compiled on.*|// compiled on ${compile_date}|" version.h

