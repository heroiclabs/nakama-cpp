#!/bin/bash
set -ue
shopt -s failglob
shopt -s extglob

artifacts_dir=${1:?pass directory with downloaded artifacts}

find_git_sha() {
  local artifacts one
  artifacts=( ${artifacts_dir}/* )
  one="${artifacts[0]}"
  # libnakama-linux-amd64-MinSizeRel-git.12345 -> 12345
  git_sha="${one##*-git.}"
}

prep_osx_universal() {
  echo '::group::Prepare generic OSX universal binaries'
  local out=${artifacts_dir}/libnakama-macosx-universal-git.${git_sha}
  mkdir -p ${out}

  # Everything, but library should be identical. Copy one arch and then delete lib
  # TODO: dont expect arm64 to always be present, pick first match instead
  cp -rf ${artifacts_dir}/libnakama-macosx-arm64*/macosx-arm64 ${out}/macosx-universal
  rm -rf ${out}/macosx-universal/nakama-sdk.framework/Versions/A/nakama-sdk

  lipo -create \
    -o ${out}/macosx-universal/nakama-sdk.framework/Versions/A/nakama-sdk \
    ${artifacts_dir}/libnakama-macosx-*/*/nakama-sdk.framework/Versions/A/nakama-sdk

  rm -rf ${artifacts_dir}/libnakama-macosx-!(universal)-*-git.${git_sha}
  echo '::endgroup::'
}

combine_generic() {
  echo '::group::Combine generic'
  local out=libnakama-git.${git_sha}
  mkdir -p ${out}
  cp -rf ${artifacts_dir}/libnakama-?(linux|win|macosx|ios)*/* ${out}/
  rm -rf ${artifacts_dir}/libnakama-?(linux|win|macosx|ios)*-git.${git_sha}
  echo "::set-output name=generic-artifact-name::${out}"
  echo "::set-output name=generic-artifact-dir::${out}"
  echo '::endgroup::'
}

post_run_checks() {
  # check that artifacts_dir is empty
  if (: "${artifacts_dir}"/*) 2>/dev/null; then
    echo "ERROR: not all artifacts were processed"
    ls -1 "${artifacts_dir}"
    exit 1;
  fi
}

find_git_sha
prep_osx_universal
combine_generic
post_run_checks
