#!/bin/bash

IFS=''
exec 3>&2 2>>$HOME/lab1_err
while cat << EOF
  1) Print name of working directory
  2) Change the working directory
  3) Run a command
  4) Create a directory
  5) Remove a directory
  6) Exit
  Enter a number of a command:
EOF

read cmd
do
  case $cmd in
    1) pwd -P || echo 'Something went wrong with pwd' >&3 ;;
    2) echo 'Enter new path:'; read npath
       cd "$npath" || echo 'Something went wrong with cd' >&3 ;;
    3) echo 'Enter command to run:'; read rcmd
       bash -c "$rcmd" || echo "Something went wrong with $rcmd" >&3 ;;
    4) echo 'Enter directory name:'; read dirname
       mkdir -- "$dirname" || echo 'Something went wrong with mkdir' >&3 ;;
    5) echo 'Enter directory name:'; read dirname
       if [ -d $dirname ]; then
         echo "Remove the $dirname directory? y/[N]:"; read agreement
         echo $agreement | rm -rI -- "$dirname" || echo 'Something went wrong with rm' >&3
       else
         echo "$dirname directory does not exist"
       fi ;;
    6) exit 0 ;;
    '') true ;;
    *) echo 'Incorrect syntax, try again' >&3 ;;
  esac
  echo
done

