#!/bin/bash

# This script prints users with more amount of processes than specified

boundary=$1

if [[ ! $boundary =~ ^[0-9]+$ ]]; then
  echo "You have to enter a positive integer as the first argument"
  exit 1
fi

IFS=$'\n'
processes=($(ps -ef | awk 'NR>1'))

declare -A PROC_AMOUNT

for item in "${processes[@]}"
do
  owner=$(echo $item | awk '{print $1}')
  if [ ${PROC_AMOUNT[$owner]+_} ]; then
    (( PROC_AMOUNT[$owner]++ ))
  else
    (( PROC_AMOUNT[$owner]=1 ))
  fi
done

result=()
for item in "${!PROC_AMOUNT[@]}"
do
if (( ${PROC_AMOUNT[$item]} > $boundary )); then
  result+=($item)
fi
done

printf "%s\n" "${result[@]}" | sort -d

