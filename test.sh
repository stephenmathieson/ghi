#!/bin/bash

throw () {
  echo "error: $1"
  exit 1
}

ghi stephenmathieson/git-{standup,sync} -o repos
[ $? -eq 0 ] || { throw "got a non-zero exit code"; }

for e in standup sync; do
  command -v "git-$e" > /dev/null 2>&1 || {
    throw "failed to install git-$e"
  }
done
