#
#   module  : builtin.sh
#   version : 1.2
#   date    : 06/07/20
#
#   Generate builtin.h and builtin.c
#
echo checking builtin.h and builtin.c
todo=0
ls -1Q src/*.c | sed 's/^/#include /' >builtin.tmp
if [ ! -f builtin.h ]
then
  echo creating builtin.h and builtin.c
  todo=1
else
  diff builtin.tmp builtin.h
  if [ $? -eq 0 ]
  then
    echo builtin.h and builtin.c are up-to-date
    rm builtin.tmp
  else
    echo updating builtin.h and builtin.c
    todo=1
  fi
fi
if [ $todo -eq 1 ]
then
  mv builtin.tmp builtin.h
  sed 's/.*\///;s/\..*//;s/.*/inter(ENV, "&", do_&);/' <builtin.h >builtin.c
fi
