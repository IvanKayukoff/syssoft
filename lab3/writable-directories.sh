#!/bin/bash

# This script prints a list of writable directories (in current directory) by specified user.
# You have to specify username as the first argument.

username=$1
[ -z $username ] &&
  echo "You have to specify the username as the first argument" &&
  exit 1

[ -z $(awk -F ":" "{ if (\$1 == \"$username\") print \$1}" /etc/passwd) ] &&
  echo "User \"$username\" does not exist :(" &&
  exit 1

IFS=$'\n'
directories=($(ls -l | grep -E '^d'))

IFS=$' \t\n'
groups=($(groups $username))

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

# Checks if the directory can be written by $username
# Argument: $1 - 'ls -l' output, accepts only 1 line
# Returns 0 if the directory can be written, otherwise - 1
# USAGE:
# user_writable "$ll_output" && echo yes || echo no
user_writable() {
  local ll_line=$1
  # Check if the file writable by user
  if [[ $username == $(echo $ll_line | awk '{print $3}') ]]; then
    if [[ $ll_line =~ ^d.w ]]; then
      return 0
    else
      return 1
    fi
  fi

  # Check if the file writable by one of the user's group
  if array_contains groups "$(echo $ll_line | awk '{print $4}')" ; then
    if [[ $ll_line =~ ^d.{4}w ]]; then
      return 0
    else
      return 1
    fi
  fi

  # Check if the file writable by others
  if [[ $ll_line =~ ^d.{7}w ]]; then
    return 0
  fi

  return 1
}

for file in "${directories[@]}"
do
  if user_writable "$file" ; then
    echo $file
  fi
done

