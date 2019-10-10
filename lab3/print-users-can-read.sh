#!/bin/bash

file_name=$1

permission=$(ls -l $file_name)

if [[ $permission =~ ^.r ]]; then
  permission_for_owner=1
  owner=$(echo $permission | awk '{print $3}')
fi

if [[ $permission =~ ^.{4}r ]]; then
  permission_for_group=1
  group_name=$(echo $permission | awk '{print $4}')
fi

if [[ $permission =~ ^.{7}r ]]; then
  permission_for_other=1
fi

group_members=($(awk -F ":" "{ if (\$1 == \"$group_name\") print \$4}" /etc/group | awk '/./'))

if [[ $permission_for_other ]]; then
  result=($(awk -F ":" '{print $1}' /etc/passwd))
else
  result=("${group_members[@]}")
  result+=("$owner")
fi

if [[ ! $permission_for_group ]]; then
  for member in ${group_members[@]}
  do
    result=("${result[@]/$member}")
  done
fi

if [[ ! $permission_for_owner ]]; then
  result=("${result[@]/$owner}")
fi

for i in "${result[@]}"; do 
  echo "$i"
done

