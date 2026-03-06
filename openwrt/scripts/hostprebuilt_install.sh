#!/bin/bash


[ -z "${1}" ] && return 0 || HOST_BUILD_DIR=${1}

echo "HOST_BUILD_DIR:${HOST_BUILD_DIR}"

INSTALL_FILES=$(find ${HOST_BUILD_DIR} -type f | grep -Ev \
'.prepared[0-9a-z]{32,32}$|.prepared[0-9a-z]{32,32}_check$|.configured$|.built_check|.built|stamp|.*_installed$|.prepared|.prereq-build');
echo "INSTALL_FILES:${INSTALL_FILES}"

for f in ${INSTALL_FILES};  do \
        if [ x"$(file -i ${f} | grep -v "charset=binary")" = x"" ] ; then \
                echo  binfile: ${f}; \
        else \
                echo  textfile: ${f}; \
                sed -i "s#_REPLACE_HOST_BUILD_DIR_#${HOST_BUILD_DIR}#g" ${f}; \
        fi \
done

