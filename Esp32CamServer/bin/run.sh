#!/bin/bash
# encoding utf-8

full_path=$(readlink -f "$0")
app_root=${full_path/\/bin\/run.sh/}

export app_root
if [[ -f ${app_root}/bin/simonCAM_profile ]]; then
  echo -e "\033[32;1mprofile exists\033[0m"
  source "${app_root}"/bin/simonCAM_profile
fi
#数据库路径
db_path="$app_root/db"

# 检查jdk
function checkJdk() {
  which java
  if [[ $? = 0 ]]; then
    versionTmp=$(java -version 2>&1 | grep \")
    version=$(echo "${versionTmp%\"*}" | sed -n "s/.*\"//p")
    echo "$version"
    if [[ "$version" =~ 17.* ]]; then
      return 0
    fi
  fi
  if [[ ! -d "$JAVA_HOME" ]]; then
    echo -e "\033[31;1m[1]\033[0m \033[34mJAVA_HOME\033[0m is not specified"
    exit 1
  fi
  echo -e "\033[34mJAVA_HOME\033[0m is $JAVA_HOME"
  export PATH=$PATH:$JAVA_HOME/bin
}

function setDbPath() {
  # shellcheck disable=SC2154
  if [[ -d "$simonCAM_cache_dir" ]]; then
    db_path="$(readlink -f ${simonCAM_cache_dir})"
  fi
  echo -e "\033[34mdb path\033[0m is $db_path"

}
#运行jar包
function runApp() {
  jarFile="$(ls ${app_root}/lib/*jar)"
  if [[ $? -ne 0 ]]; then
    echo -e "\033[31;1m[2]\033[0m \033[34mjarFile\033[0m not exists is $app_root/lib"
    exit 2
  fi
  cd "$app_root" || exit
  java -Ddb.path="$db_path" -Djdk.crypto.KeyAgreement.legacyKDF=true -jar $jarFile
}

function main() {
  checkJdk
  setDbPath
  runApp
}

main
