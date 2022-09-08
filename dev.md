# git-subrepo

git subrepo is external tools fulfilling similar role to
git submodules or git subtree, but aiming to avoid their 
disadvantages.

Main distinction is that it completely vendors in another
git repository  as plain git files, so for anyone cloning repository
they don't need to do anything special (like checkout submodules)
to get vendored files.

In addition to simply vendoring, git subrepo allows bidirectional
communication with vendored git repository: it is possible
to safely pull updates or push local changes.

Directories where vendored git repostory reside are marked
with `.gitrepo` file containing necessary metadata to make
bidirectional commit flow work.

This document describes how to pull fresh update to
`third_party/libhttpclient`

## Preflight
[Windows setup](https://github.com/ingydotnet/git-subrepo/issues/271#issuecomment-373722235)

## Workflow

1. Make sure that working directory has no uncommitted changes
2. `git subrepo ./third_party/libhttpclient`  (important: use forward slashes even on Windows)

On a good day that would be it, but in our case there are
conflicts between our local changes and remote changes we 
are trying to pull.

Git subrepo prints detailed instructions you can follow to
resolve conflict and commit changes. Here are exact steps taken
in our case:

1. `cd .git/tmp/subrepo/third_party/libHttpClient` . This is a clone of
   vendored git repository. 
2. `git status` to see merge conflicts
3. `<skipped>` resolve conflicts
4. change directory back to where `nakama-cpp-mono` is
5. `git subrepo commit third_party/libHttpClient`