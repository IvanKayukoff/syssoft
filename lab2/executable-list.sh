#!/bin/bash

read -r -a groups <<< "$(groups)"

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

# Checks if the file can be executed by $USER
# Argument: $1 - 'ls -l' output, accepts only 1 line
# Returns 0 if the file can be executed, otherwise - 1
# USAGE:
# user_executable "$ll_output" && echo yes || echo no
user_executable() {
  local ll_line=$1
  # Check if the file executable by user
  if [[ $USER == $(echo $ll_line | awk '{print $3}') ]]; then
    if [[ $ll_line =~ ^-..x ]]; then
      return 0
    else
      return 1
    fi
  fi

  # Check if the file executable by one of the user's group
  if array_contains groups "$(echo $ll_line | awk '{print $4}')" ; then
    if [[ $ll_line =~ ^-.{5}x ]]; then
      return 0
    else
      return 1
    fi
  fi

  # Check if the file executable by others
  if [[ $ll_line =~ ^-.{8}x ]]; then
    return 0
  fi

  return 1
}

IFS=':'
read -r -a paths <<< "$PATH"
IFS=$'\n'

files=()
for item in "${paths[@]}"
do
  files+=($(ls -l $item | grep -v '^total'))
done

for item in "${files[@]}"
do
  if user_executable $item ; then
    echo $item
  fi
done

