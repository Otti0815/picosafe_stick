#!/bin/bash

# tuneables
iterations=100
blocksize="65535"

filename="`mktemp`"

for i in `seq $iterations`; do
  dd if=/dev/urandom of=$filename bs=$blocksize count=$i > /dev/null 2>/dev/null
  sum1="`md5sum $filename`"
  ../picosafe_aes -k testkey    $filename
  ../picosafe_aes -k testkey -d $filename
  sum2="`md5sum $filename`"

  if [ "$sum1" != "$sum2" ];
  then
    echo "error $i"
    exit 1
  fi

  echo $i
done

echo "ok"
