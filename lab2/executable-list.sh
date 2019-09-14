#!/bin/bash

read -r -a groups <<< "$(groups)"

# DEBUG MODE ON
for item in "${groups[@]}"
do
  echo "$item"
done
echo
echo "Number of elements: ${#groups[@]}"

# Checks if the element is present in the array
# Returns 0 if arr contains the argument, otherwise - 1
# USAGE:
# array_contains arr "a b" && echo yes || echo no
# array_contains arr "d e" && echo yes || echo no
array_contains () {
  local array="$1[@]"
  local seeking=$2
  local in=1
  for element in "${!array}"; do
    if [[ $element == "$seeking" ]]; then
      in=0
      break
    fi
  done
  return $in
}

IFS=':'
read -r -a paths <<< "$PATH"

# DEBUG MODE ON
for item in "${paths[@]}"
do
  echo "$item"
done
echo
echo "Number of elements: ${#paths[@]}"

#TODO print list of binaries

