#!/bin/bash

# Avoid executing alisa make on .bashrc, alias make check should run in envsetup first
unalias make 2> /dev/null

function _load_config()
{
	local cfgkey=$1
	local cfgfile=$2
	local defval=$3
	local val=""

	[ -f "$cfgfile" ] && val="$(sed -n "/^\s*export\s\+$cfgkey\s*=/h;\${x;p}" $cfgfile | sed -e 's/^[^=]\+=//g' -e 's/^\s\+//g' -e 's/\s\+$//g')"
	eval echo "${val:-"$defval"}"
}

# cd openwrt dir
function cop()
{
	cd ${TINA_TOPDIR}/openwrt
}

# cd openwrt src dir
function copsrc()
{
	cd ${TINA_TOPDIR}/openwrt/openwrt
}

function copdl()
{
	cd ${TINA_TOPDIR}/openwrt/dl
}

function copbuild()
{
	cd ${TINA_TOPDIR}/openwrt/build
}


function cgeneric()
{
	local dkey="LICHEE_IC"
	local dval=$(_load_config $dkey ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval" ] && echo "ERROR: $dkey not set in .buildconfig" && return 1
	[ -d ${TINA_TOPDIR}/target/$dval/openwrt/generic ] && \
		cd ${TINA_TOPDIR}/target/$dval/openwrt/generic || \
		cd ${TINA_TOPDIR}/openwrt/target/$dval/generic
}


function ccommon()
{
	local dkey="LICHEE_IC"
	local dval=$(_load_config $dkey ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval" ] && echo "ERROR: $dkey not set in .buildconfig" && return 1
	[ -d ${TINA_TOPDIR}/target/$dval/openwrt/$dval-common ] && \
		cd ${TINA_TOPDIR}/target/$dval/openwrt/$dval-common || \
		cd ${TINA_TOPDIR}/openwrt/target/$dval/$dval-common
}

function cplat()
{
	local dkey1="LICHEE_IC"
	local dkey2="LICHEE_BOARD"
	local dval1=$(_load_config $dkey1 ${TINA_TOPDIR}/.buildconfig)
	local dval2=$(_load_config $dkey2 ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval1" ] && echo "ERROR: $dkey1 not set in .buildconfig" && return 1
	[ -z "$dval2" ] && echo "ERROR: $dkey2 not set in .buildconfig" && return 1
	[ -d ${TINA_TOPDIR}/target/$dval1/openwrt/$dval1-$dval2 ] && \
		cd ${TINA_TOPDIR}/target/$dval1/openwrt/$dval1-$dval2 || \
		cd ${TINA_TOPDIR}/openwrt/target/$dval1/$dval1-$dval2
}

function ctoolchain()
{
	cd ${TINA_TOPDIR}/prebuilt/
}

function chostbin()
{
	local dkey1="LICHEE_IC"
	local dkey2="LICHEE_BOARD"
	local dkey3="LICHEE_LINUX_DEV"
	local dval1=$(_load_config $dkey1 ${TINA_TOPDIR}/.buildconfig)
	local dval2=$(_load_config $dkey2 ${TINA_TOPDIR}/.buildconfig)
	local dval3=$(_load_config $dkey3 ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval1" ] && echo "ERROR: $dkey1 not set in .buildconfig" && return 1
	[ -z "$dval2" ] && echo "ERROR: $dkey2 not set in .buildconfig" && return 1
	[ -z "$dval3" ] && echo "ERROR: $dkey3 not set in .buildconfig" && return 1

	cd ${TINA_TOPDIR}/out/${dval1}/${dval2}/${dval3}/staging_dir/host/bin
}

# out target
function ctarget()
{
	local dkey1="LICHEE_IC"
	local dkey2="LICHEE_BOARD"
	local dkey3="LICHEE_LINUX_DEV"
	local dval1=$(_load_config $dkey1 ${TINA_TOPDIR}/.buildconfig)
	local dval2=$(_load_config $dkey2 ${TINA_TOPDIR}/.buildconfig)
	local dval3=$(_load_config $dkey3 ${TINA_TOPDIR}/.buildconfig)
	[ -z "$dval1" ] && echo "ERROR: $dkey1 not set in .buildconfig" && return 1
	[ -z "$dval2" ] && echo "ERROR: $dkey2 not set in .buildconfig" && return 1
	[ -z "$dval3" ] && echo "ERROR: $dkey3 not set in .buildconfig" && return 1

	cd ${TINA_TOPDIR}/out/${dval1}/${dval2}/${dval3}/build_dir/target
}

# cd wifimanager core dir
function cwifimg()
{
	cd ${TINA_TOPDIR}/platform/allwinner/wireless/wifimanager
}

# cd wifimanager makefile dir
function cwifimgm()
{
	cd ${TINA_TOPDIR}/openwrt/package/allwinner/wireless/wifimanager
}

# cd btmanager core dir
function cbtmg()
{
	cd ${TINA_TOPDIR}/platform/allwinner/wireless/btmanager
}

# cd btmanager makefile dir
function cbtmgm()
{
	cd ${TINA_TOPDIR}/openwrt/package/allwinner/wireless/btmanager
}

### check parameter
function check_tina_topdir()
{
	[ -z "${TINA_TOPDIR}" -o ! -e "${TINA_TOPDIR}/build/envsetup.sh" ] \
		&& echo "Couldn't locate the top of the tree.  Try setting TOP." \
		&& return -1

	return 0
}

# check to see if the supplied product is one we can build
function check_platform()
{
	local tinadir_openwrt_target="${TINA_TOPDIR}/openwrt/target"
	local tina_platform_choices="$(ls ${tinadir_openwrt_target} 2>/dev/null | sort | uniq)"
	local tinadir_openwrt_target_new="${TINA_TOPDIR}/target"
	local tina_platform_choices_new="$(ls ${tinadir_openwrt_target_new} 2>/dev/null | sort | uniq)"

	local v
	for v in ${tina_platform_choices} ${tina_platform_choices_new}
	do
		[ "$v" = "$1" -a -n "$v" ] && return 0
	done

	return -1
}

# check to see if the supplied product is one we can build
# $1 is ic name, $platform in func envsetup.
# $2 is board name, $board in func envsetup.
function check_board()
{
	[ -z "$1" -o -z "$2" ] && return -1
	local tinadir_openwrt_target="${TINA_TOPDIR}/openwrt/target"
	local tina_board_choices="$(ls ${tinadir_openwrt_target}/$1 2>/dev/null | sort | uniq)"
	local tinadir_openwrt_target_new="${TINA_TOPDIR}/target"
	local tina_board_choices_new="$(ls ${tinadir_openwrt_target_new}/$1/openwrt 2>/dev/null | sort | uniq)"

	local v=
	for v in ${tina_board_choices} ${tina_board_choices_new}
	do
		[ "$v" = "$1-$2" -a -n "$v" ] && return 0
	done

	return -1
}

function get_storage_type()
{
	check_tina_topdir || return -1
	[ -z "${TINA_TARGET_PLAT}" ] && return -1
	[ -z "${TINA_TARGET_BOARD}" ] && return -1
	return -1
}

function print_lunch_menu()
{
	local uname=$(uname)
	echo
	echo "You're building on" $uname
	echo
	echo "Lunch menu... pick a combo:"

	local tinafile_openwrt_target=${TINA_TOPDIR}/openwrt/.openwrt_targets
	local board_index=1
	local first_line
	local platform

	# Check if target file exists
	if [ -e "$tinafile_openwrt_target" ]; then
		# Get first chip name as platform to check
		first_line=$(head -n 1 "$tinafile_openwrt_target")
		platform=$(echo "$first_line" | cut -d'-' -f1)

		# Check if first platform has hook script
		if [ -e ${TINA_TOPDIR}/openwrt/target/${platform}/${platform}-common/${platform}_print_lunch_menu_hook.sh ]; then
			# Get all unique platforms from target file
			local platforms=$(cat "$tinafile_openwrt_target" | cut -d'-' -f1 | sort -u)

			# Iterate through each platform
			for plat in $platforms; do
				# Check if this platform has hook script
				if [ -e ${TINA_TOPDIR}/openwrt/target/${plat}/${plat}-common/${plat}_print_lunch_menu_hook.sh ]; then
					# Create a temporary file for this platform's targets
					local temp_file=$(mktemp)

					# Make sure temp file was created successfully
					if [ -n "$temp_file" ]; then
						# Write targets to temp file
						grep "^$plat-" "$tinafile_openwrt_target" > "$temp_file"

						# Only process if temp file has content
						if [ -s "$temp_file" ]; then
							# Source and call the hook script with this platform's targets
							source ${TINA_TOPDIR}/openwrt/target/${plat}/${plat}-common/${plat}_print_lunch_menu_hook.sh
							print_lunch_menu_hook "$temp_file"
						fi

						# Clean up temporary file
						rm -f "$temp_file" 2>/dev/null
					fi
				fi
			done
		else
			# If first platform doesn't have hook script, use original method
			cat -n ${tinafile_openwrt_target}
		fi
	else
		# If target file doesn't exist
		echo "Target file not found: $tinafile_openwrt_target"
	fi
}

# Tab completion for lunch.
function _lunch()
{
	local cur prev
	local tinafile_openwrt_target=${TINA_TOPDIR}/openwrt/.openwrt_targets

	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"
	prev="${COMP_WORDS[COMP_CWORD-1]}"
	COMPREPLY=( $(compgen -W "$(cat ${tinafile_openwrt_target})" -- ${cur}) )
	return 0
}

function check_for_v_series()
{
	# The V series has special behavior due to the way the software packages (eyesee-mpp)
	# are compiled. During the compilation process, it is necessary to clean
	# up the previous build artifacts, otherwise, various configuration
	# issues may occur.

	# get last platfrom as default platform
	local confile="${TINA_TOPDIR}/.buildconfig"
	local select=$1
	if [ -f "${confile}" ]; then
		local dkey1="LICHEE_IC"
		local dkey2="LICHEE_BOARD"
		local dval1=$(_load_config $dkey1 ${TINA_TOPDIR}/.buildconfig)
		local dval2=$(_load_config $dkey2 ${TINA_TOPDIR}/.buildconfig)
	fi

	# check for v series
	if [[ "${select:0:1}" != "v" ]]; then
		return 0
	fi

	if [ ! -z "${dval1}" ]; then
		if [ "$select" != "${dval1}-${dval2}" ]; then
			if [ -d "${TINA_TOPDIR}/out/${dval1}/${dval2}" ]; then
				# Get all link project using ln -s
				mapfile -t result < <(grep -rn "ln -s \$(SRC_CODE_DIR) \$(SRC_COMPILE_DIR)" ${TINA_TOPDIR}/openwrt/package | awk -F: '{print $1}')

				# check PKG NAME and if this package is built
				for package_makefile in "${result[@]}"; do
					pkg_name=$(cat "$package_makefile" | grep -oP 'PKG_NAME:=\K.*')
					if [ -d "${TINA_TOPDIR}/out/${dval1}/${dval2}/openwrt/build_dir/target/$pkg_name" ]; then
						relative_path=$(realpath --relative-to=${TINA_TOPDIR}/openwrt/package $package_makefile | sed 's|/Makefile||')
						echo -e "\e[47;30mlunch new board, clean previous '$relative_path' related build artifacts.\e[0m"
						${TINA_TOPDIR}/build.sh openwrt_rootfs package/subpackage/$relative_path/clean
					fi
				done
			fi
		fi
	fi
}

function lunch
{
	check_tina_topdir || return -1

	# get last platfrom as default platform
	local confile="${TINA_TOPDIR}/.buildconfig"
	local last
	if [ -f "${confile}" ]; then
		#last="$(awk -F[=_] '/CONFIG_TARGET_[a-z_0-9]*[^_]=y/{print $3 "_" $4; exit}' ${T}/.config)"
		#last="$(sed -n -r '/^CONFIG_TARGET_BOARD=.*$/{s/^CONFIG_TARGET_BOARD=\"(.*)\"$/\1/;p;q}' ${confile})"
		local dkey1="LICHEE_IC"
		local dkey2="LICHEE_BOARD"
		local dval1=$(_load_config $dkey1 ${TINA_TOPDIR}/.buildconfig)
		local dval2=$(_load_config $dkey2 ${TINA_TOPDIR}/.buildconfig)
		last=$dval1-$dval2
	fi

	# select platform
	local select
	if [ "$1" ] ; then
		select=$1
	else
		print_lunch_menu
		echo -n "Which would you like?"
		[ -n "${last}" ] && echo -n " [Default ${last}]"
		echo -n ": "
		read select
	fi

	# format $select
	select=$(echo ${select} |sed 's/-tina//g')

	local tinafile_openwrt_target=${TINA_TOPDIR}/openwrt/.openwrt_targets
	if [ -z "${select}" ]; then
		select="${last}-tina"
	elif (echo -n $select | grep -q -e "^[0-9][0-9]*$"); then
		select=$(eval "awk 'NR==${select}' ${tinafile_openwrt_target}")
	elif (echo -n $select | grep -q -e "^.*-.*"); then
		select="$select-tina"
	else
		echo
		echo "Invalid lunch combo: $select" >&2
		return -1
	fi

	# check platform
	select=$(echo ${select} |sed 's/-tina//g')
	local board=$(echo -n $select | awk -F- '{print $NF}')
	local platform=$(echo -n $select | sed "s/-$board//g")
	check_platform ${platform}
	if [ $? -ne 0 ]; then
		echo
		echo "** Don't have a platform spec for: '$platform'" >&2
		echo "** Do you have the right repo manifest?" >&2
		platform=
		return -1
	fi

	# check board
	check_board ${platform} ${board}
	if [ $? -ne 0 ]; then
		echo
		echo "** Don't have a board spec for: '$board'" >&2
		echo "** Do you have the right repo manifest?" >&2
		board=
		return -1
	fi

	local variant=$(echo -n $select | awk -F- '{print $3}')
	if [ $? -ne 0 ]
	then
		echo
		echo "** Invalid variant: '$variant'" >&2
		echo "** Must be one of ${VARIANT_CHOICES}" >&2
		variant=
		return -1
	fi

	# phase sysconfig.fex
	local tinafile_sysconfig="${TINA_TOPDIR}/device/config/chips/${platform}/configs/${board}/sys_config.fex"

	local storage_type=
	storage_type=$([ -e "$tinafile_sysconfig" ] && awk -F"=" '/^storage_type/{print $2}' $tinafile_sysconfig | sed 's/^[ \t]*//g' )
	if [ x"$storage_type" = x"3" ]; then
		local flash="nor"
	else
		local flash="default"
	fi

	# check for v series check chip alter
	if [ -e ${TINA_TOPDIR}/openwrt/target/${platform}/${platform}-common/${platform}_select_lunch_hook.sh ]; then
		source ${TINA_TOPDIR}/openwrt/target/${platform}/${platform}-common/${platform}_select_lunch_hook.sh
		select_lunch_hook ${select}
		if [ $CHIP_ALTER_ERROR -eq 1 ]; then
			return 1
		fi
	fi

	# check for v series multimedia clean status
	check_for_v_series ${select}

	echo "Jump to longan autoconfig"
	echo "${TINA_TOPDIR}/build.sh autoconfig -o openwrt -i $platform -b $board \
		-n $flash "

	${TINA_TOPDIR}/build.sh autoconfig -o openwrt -i $platform -b $board \
		-n $flash

	# load platform specific script if exists
	[ -e ${TINA_TOPDIR}/openwrt/target/${platform}/${platform}-common/${platform}_common-setup.sh ] \
		&& source ${TINA_TOPDIR}/openwrt/target/${platform}/${platform}-common/${platform}_common-setup.sh

	[ -e ${TINA_TOPDIR}/openwrt/target/${platform}/${platform}-${board}/${platform}_${board}-setup.sh ] \
		&& source ${TINA_TOPDIR}/openwrt/target/${platform}/${platform}-${board}/${platform}_${board}-setup.sh

	[ -e ${TINA_TOPDIR}/target/${platform}/openwrt/${platform}-common/${platform}_common-setup.sh ] \
		&& source ${TINA_TOPDIR}/target/${platform}/openwrt/${platform}-common/${platform}_common-setup.sh

	[ -e ${TINA_TOPDIR}/target/${platform}/openwrt/${platform}-${board}/${platform}_${board}-setup.sh ] \
		&& source ${TINA_TOPDIR}/target/${platform}/openwrt/${platform}-${board}/${platform}_${board}-setup.sh

	#Make sure to use the correct board's bsp defconfig
	${TINA_TOPDIR}/build.sh loadconfig

	if [ -e ${TINA_TOPDIR}/openwrt/target/${platform}/${platform}-common/${platform}_select_lunch_hook.sh ]; then
		select_lunch_hook_action ${select}
	fi

	return  0
}

function add_lunch_combo()
{
	[ -z "$@" ] && return -1;

	local tinafile_openwrt_target=${TINA_TOPDIR}/openwrt/.openwrt_targets
	local all=$(cat ${tinafile_openwrt_target})
	echo ${all} $@ | xargs -n1 | sort | uniq > ${tinafile_openwrt_target}
}

function make()
{
	check_tina_topdir || return -1

	if [ x"${TINA_TOPDIR}" = x"$(pwd)" ] ; then
		echo "===There is tina environment.==="
		echo " Note: make is the shell functon in envsetup.sh. "
	else
		echo "===There isn't tina environment.==="
		echo " Note: will use shell command origin rather than the functon. "
		command make $@
		return $?
	fi
	echo ""

	local allargs=$@
	local action action_args
	action=openwrt_build
	while [ $# -gt 0 ]; do
		case "$1" in
		kernel_menuconfig|menuconfig|ota_menuconfig|openwrt_rootfs|openwrt_build|clean|distclean)
			action=$1
			shift;
			action_args="$@"
			break;
			;;
		*)
			# default
			action=openwrt_build
			action_args="$@"
			break;
		esac
	done
	echo "== action: $action, action_args: $action_args =="

	if [ x"$action" = x"kernel_menuconfig" ] ; then
		${TINA_TOPDIR}/build.sh loadconfig
		[ $? -ne 0 ] && echo "Error: when exec loadconfig" && return -1;
		${TINA_TOPDIR}/build.sh menuconfig
		[ $? -ne 0 ] && echo "Error: when exec menuconfig" && return -1;
		${TINA_TOPDIR}/build.sh saveconfig
		[ $? -ne 0 ] && echo "Error: when exec saveconfig" && return -1;
		return 0;
	fi

	if [ x"$action" = x"menuconfig" ] ; then
		${TINA_TOPDIR}/build.sh openwrt_menuconfig $@
		return $?;
	fi

	if [ x"$action" = x"ota_menuconfig" ] ; then
		${TINA_TOPDIR}/build.sh openwrt_menuconfig OTA=_ota
		return $?;
	fi

	if [ x"$action" = x"openwrt_rootfs" ] ; then
		${TINA_TOPDIR}/build.sh openwrt_rootfs $@
		return $?;
	fi

	if [ x"$action" = x"openwrt_build" ] ; then
		${TINA_TOPDIR}/build.sh tina $@
		return $?;
	fi

	if [ x"$action" = x"clean" ] ; then
		${TINA_TOPDIR}/build.sh $action $@
		return $?;
	fi

	if [ x"$action" = x"distclean" ] ; then
		${TINA_TOPDIR}/build.sh $action $@
		return $?;
	fi

	return $?;
}

function mm
{
	local T=${TINA_TOPDIR}/
	local orgin=`pwd`
	local path=`pwd`
	local target=
	local cmd=

	#find target makefile
	#trap 'echo $orgin >> ~/mm; trap - SIGINT; ' SIGINT
	while [ x`pwd` != x$T ] && [ x`pwd` != x"/" ]
	do
		find  -maxdepth 1 -name Makefile | xargs cat | grep "define Package" > /dev/null
		is_package=$?
		find  -maxdepth 1 -name Makefile | xargs cat | grep "define KernelPackage" > /dev/null
		is_kernel_package=$?
		if [ $is_package -eq 1 ] && [ $is_kernel_package -eq 1 ]; then
			cd ../
		else
			path=`pwd`
			target=${path##*/}
			cd $T
			cmd="compile V=s"
			for i in $*; do
				[ x$i = x"-B" ] && {
					# -B clean the package
					print_red "make openwrt_rootfs package/$target/clean V=s"
					make openwrt_rootfs package/$target/clean V=s
				}
				[ x${i:0:2} = x"-j" ] && cmd=$cmd" "$i
			done
			print_red "make openwrt_rootfs package/$target/$cmd"
			make openwrt_rootfs package/$target/$cmd
			cd $orgin
			#trap - SIGINT
			return
		fi
	done
	cd $orgin
	#trap - SIGINT
	print_red "Can't not find Tina Package Makefile!"
}

function mmo
{
	local T=${TINA_TOPDIR}/
	local orgin=`pwd`
	local target=$1
	local cmd=

	[ x$target = x ] && {
		print_red "mmo need pkg name."
		return
	}
	cd $T
	cmd="compile V=s"
	for i in $*; do
		[ x$i = x"-B" ] && {
			# -B clean the package
			print_red "make openwrt_rootfs package/$target/clean V=s"
			make openwrt_rootfs package/$target/clean V=s
		}
		[ x${i:0:2} = x"-j" ] && cmd=$cmd" "$i
	done
	print_red "make openwrt_rootfs package/$target/$cmd"
	make openwrt_rootfs package/$target/$cmd
	cd $orgin
}

function _m
{
	[ -z "$TINA_TOPDIR" ] && return
	local cur prev list
	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"
	prev="${COMP_WORDS[COMP_CWORD-1]}"
	list="$(grep "\"\$action\" = x\".*\"" $BASH_SOURCE | awk -F'"' '{print $4}' )"
	COMPREPLY=( $(compgen -W "${list}" -- ${cur}) )
}

function _make
{
	[ -z "$TINA_TOPDIR" ] && return
	[ x"${TINA_TOPDIR}" == x"$(pwd)" ] || return
	local cur prev list
	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"
	prev="${COMP_WORDS[COMP_CWORD-1]}"
	list="$(grep "\"\$action\" = x\".*\"" $BASH_SOURCE | awk -F'"' '{print $4}' )"
	COMPREPLY=( $(compgen -W "${list}" -- ${cur}) )
}


function m
{
	(\cd ${TINA_TOPDIR} && make $@)
}

function p
{
	(\cd ${TINA_TOPDIR} && pack $@)
}

function mp
{
	m $@ && p
}

function _cleanenv()
{
	# clean tina4.0 env
	unset TINA_TARGET_ARCH
	unset TARGET_UBOOT
	unset LICHEE_CHIP_CONFIG_DIR
	unset TINA_BUILD_TOP
	unset TARGET_BUILD_VARIANT
	unset TINA_VERBOSE
	unset LICHEE_BRANDY_SPL
	unset TARGET_PLAN
	unset BUILD_ENV_SEQUENCE_NUMBER
	unset TARGET_BOARD
	unset LICHEE_BUSSINESS
	unset LICHEE_ARISC_PATH
	unset LICHEE_REDUNDANT_ENV_SIZE
	unset TARGET_BUILD_TYPE
	unset TARGET_KERNEL_VERSION
	unset TARGET_PLATFORM
	unset TARGET_CHIP
	unset LICHEE_PACK_OUT_DIR
	unset LICHEE_PLAT_OUT
	unset LICHEE_BOARD
	unset LICHEE_BOARD_CONFIG_DIR
	unset TINA_TOP
	unset LICHEE_BRANDY_DEFCONF
	unset TARGET_PRODUCT
	unset VARIANT_CHOICES
	unset PLATFORM_CHOICES
}


function envsetup_openwrt()
{
	# check bash
	if [ "x$SHELL" != "x/bin/bash" ]; then
		case `ps -o command -p $$` in
			*bash*)
				;;
			*)
				echo -n "WARNING: Only bash is supported, "
				echo "use of other shell would lead to erroneous results"
				;;
		esac
	fi

	# clean env
	_cleanenv

	# reset these variables.
	local tinafile_openwrt_target=${TINA_TOPDIR}/openwrt/.openwrt_targets
	[ -e ${tinafile_openwrt_target} ] && rm ${tinafile_openwrt_target}
	touch ${tinafile_openwrt_target}

	# Execute the contents of any vendorsetup.sh files we can find.
	local vendors vendor
	local tinadir_openwrt_target="${TINA_TOPDIR}/openwrt/target"
	local tinadir_openwrt_target_new="${TINA_TOPDIR}/target"
	verdors="$(find -L ${tinadir_openwrt_target} ${tinadir_openwrt_target_new} -maxdepth 4 -name 'vendorsetup.sh' 2>/dev/null | sort)"

	for verdor in ${verdors}
	do
		source ${verdor}
	done

	# completion
	complete -F _lunch lunch
	complete -F _m m
	complete -F _make make
}


### MAIN ###

# check top of SDK
if [ ! -f "${TINA_TOPDIR}/build/.tinatopdir" ]; then
	echo "ERROR: Not found .tinatopdir"
	return -1;
fi


envsetup_openwrt


