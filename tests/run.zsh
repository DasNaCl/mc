#!/usr/bin/env zsh

WHITE="\e[0m"
GREEN="\e[32m"
RED="\e[31m"

EXE="$1"

if [ -z "${EXE}" ]; then
  echo -ne "${RED}Path to compiler expected as first argument!${WHITE}\n"
  exit 1
fi

for directory in */; do
  directory=${directory%*/}
  testcount=$(ls -1q "$directory"/**/*.mcc | wc -l)

  echo -ne "--- Testing $directory with $testcount tests ---\n\n"

  successes=0
  fails=0
  for f in $directory/**/*.mcc; do  
    posneg=$(echo -ne "$f" | awk -F/ '{print $(NF-1)}')
    testname=$(echo -ne "$f" | awk -F/ '{print $NF}')
    echo -ne "$posneg test: $testname ...."
    TESTOUT=$(cat "$f" | exec ${EXE} | colordiff --suppress-common-lines -y ${f%.*}.txt -)
    TESTRES=$(echo -ne "${TESTOUT}" | wc -l)

    if (( ${TESTRES} > 0 )); then
      echo -ne " ${RED}FAIL${WHITE} with ${TESTRES} differing lines of output.\n"
      fails=$(( fails + 1 ))
    else
      echo -ne "${GREEN}SUCCESS${WHITE}.\n"
      successes=$(( success + 1 ))
    fi
  done
  SUCCCOL="$WHITE"
  if (( ${successes} > 0 )); then
    SUCCCOL="$GREEN"
  fi
  FAILCOL="$GREEN"
  if (( ${fails} > 0 )); then
    FAILCOL="$RED"
  fi
  echo -ne "\n--- Tested $directory (${SUCCCOL}$successes${WHITE} + ${FAILCOL}$fails${WHITE} = $testcount) ---\n"
done

