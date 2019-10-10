#!/bin/bash

# This script prints all users that can read specified file.
# You have to specify the path to the desired file as the first script argument.

# Check that user's specified the file path
file_name=$1
[ -z "$file_name" ] && echo "You have to specify the path to the desired file" && exit 1

# Check that the specified file exists
ll_line=$(ls -l $file_name 2>/dev/null)
[ ! $? == 0 ] && echo "File \"$file_name\" not found :(" && exit 1

owner=$(echo $ll_line | awk '{print $3}')
group_name=$(echo $ll_line | awk '{print $4}')
group_members=($(awk -F ":" "{ if (\$1 == \"$group_name\") print \$4}" /etc/group | awk '/./'))

if [[ $ll_line =~ ^.{7}r ]]; then
  result=($(awk -F ":" '{print $1}' /etc/passwd))
else
  result=("${group_members[@]}")
  result+=("$owner")
fi

if [[ ! $ll_line =~ ^.{4}r ]]; then
  for member in ${group_members[@]}
  do
    result=("${result[@]/$member}")
  done
fi

if [[ ! $ll_line =~ ^.r ]]; then
  result=("${result[@]/$owner}")
fi

for username in "${result[@]}"; do 
  echo "$username"
done

