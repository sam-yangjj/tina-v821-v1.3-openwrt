#!/bin/bash
# V821 Series lunch menu
# Author: zhangyuanjing <zhangyuanjing@allwinnertech.com>

# Read configuration from board's lunch_menu.mk file
function get_board_config() {
	local board_dir=$1
	local config_file="${board_dir}/lunch_menu.mk"
	local config_name=$2

	if [ -f "$config_file" ]; then
		local value=$(grep -E "^$config_name\s*:=\s*" "$config_file" | sed -E "s/^$config_name\s*:=\s*//;s/^\"(.*)\"$/\1/")
		echo -n "${value:-"N/A"}"
	else
		echo -n "N/A"
	fi
}

function print_lunch_menu_hook()
{
	local openwrt_target="$1"
	# Extract platform from the first line of the target file
	local first_line=$(head -n 1 "$openwrt_target" 2>/dev/null)
	local platform=$(echo "$first_line" | cut -d'-' -f1 2>/dev/null)
	local target_dir="${TINA_TOPDIR}/openwrt/target/${platform}"

	# Check if whitelist file exists
	if [ -f "$openwrt_target" ]; then
		if [ "$board_index" -eq 1 ]; then
			echo "---------------------------------------------------------------------------------------------------------"
		fi
		# Iterate through each board in whitelist order
		while IFS= read -r board_name || [ -n "$board_name" ]; do
			# Skip empty lines
			[ -z "$board_name" ] && continue

			# Extract directory name from board_name (remove -tina suffix)
			local dir_name=${board_name%-tina}

			# Construct full board directory path
			local board_dir="$target_dir/$dir_name"

			# Check if directory exists
			if [ -d "$board_dir" ]; then
				# Read all configurations from lunch_menu.mk
				local boot_medium=$(get_board_config "$board_dir" "BOOT_MEDIUM")
				local support_medium=$(get_board_config "$board_dir" "SUPPORT_BOOT_MEDIUM")
				local boot_type=$(get_board_config "$board_dir" "BOOT_TYPE")
				local hardware_name=$(get_board_config "$board_dir" "HARDWARE_NAME")
				local network=$(get_board_config "$board_dir" "NETWORK")
				local application=$(get_board_config "$board_dir" "APPLICATION")
				local url=$(get_board_config "$board_dir" "URL")

				# Display menu with proper color formatting
				local col0_width=38
				printf "\033[32m%-${col0_width}s\033[0m | %s\n" "${board_index}.  $board_name" "Application    (应用场景) : ${application:-\"N/A\"}"

				# Create two columns of menu items with proper alignment
				local col1_width=42
				printf "%-${col1_width}s | %s\n" " Boot Type      (启动方式) : ${boot_type:-\"N/A\"}" "Hardware Name  (硬件丝印) : ${hardware_name:-\"N/A\"}"
				echo "---------------------------------------------------------------------------------------------------------"
				board_index=$((board_index + 1))
			fi
		done < "$openwrt_target"
	else
		echo "OpenWrt Target file not found: $openwrt_target"
	fi
}

