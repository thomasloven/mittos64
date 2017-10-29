#!/usr/bin/env bash

set -e

first_commit=`git rev-list --max-parents=0 master`

revs=`git rev-list --reverse ${first_commit}..master`

git tag | xargs git tag -d

for rev in $revs; do
  message=`git log --pretty=tformat:%s -1  $rev`
  chapter=`echo $message | sed -e "/^Chapter \(.*\):.*$/!d;s//ch\1/"`
  if [[ -n $chapter ]]; then
    git tag $chapter $rev
  fi
done

