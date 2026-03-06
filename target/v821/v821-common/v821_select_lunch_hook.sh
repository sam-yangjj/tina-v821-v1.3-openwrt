#!/bin/bash
local select_chip_alter_selected_chip
local select_baud_rate

function select_lunch_hook()
{
	local select=$1
	local last_selection_file="${TINA_TOPDIR}/.buildconfig_chip_alter"
	local baud_rate_file="${TINA_TOPDIR}/.buildconfig_baud_rate"

	# check for v series
	if [[ "${select:0:1}" != "v" ]]; then
		return 0
	fi

	local chip_alter
	local chip_options=("V821L2-XXX" "V821M2-WXX" "V821L2-WXX" "V521D2-WXX" "V821L2-BXX" "V821M2-WBX" "V821L2-WBX" "V521D2-WBX" "V821D2-WBX")
	local baud_rate
	local baud_rate_options=("115200" "1500000")  # Define the baud rate options

	if [[ -f "$last_selection_file" ]]; then
		last_selected_chip=$(cat "$last_selection_file")
	else
		last_selected_chip="none"
	fi

	if [[ -f "$baud_rate_file" ]]; then
		last_selected_baud_rate=$(cat "$baud_rate_file")
	else
		last_selected_baud_rate="none"
	fi

	# Check if the selected board is 'v821'
	if [[ "${select:0:4}" == "v821" ]]; then
		echo "You're selecting board: $select, choose your chip:"

		# Loop through the chip options and display them
		for i in "${!chip_options[@]}"; do
			echo "    $((i + 1)) ${chip_options[$i]}"  # Display the index and chip name
		done

		if [[ "$last_selected_chip" != "none" ]]; then
			echo -n "Which chip would you like? [$last_selected_chip]: "
		else
			echo -n "Which chip would you like? "
		fi
		read chip_alter
	fi

	# Check if the input is empty or invalid
	if [ -z "${chip_alter}" ]; then
		if [ -z "${last_selected_chip}" ]; then
			echo
			echo "Invalid input: empty selection."
			echo "** Must be one of ${chip_options[*]}"
			CHIP_ALTER_ERROR=1
			return -1
		fi
		chip_alter=${last_selected_chip}
	fi

	# Check if the input is a valid number within the available chip options
	if [[ "${chip_alter}" =~ ^[0-9]+$ ]] && ((chip_alter >= 1 && chip_alter <= ${#chip_options[@]})); then
		# Validate the number and select the chip accordingly
		select_chip_alter_selected_chip="${chip_options[$((chip_alter - 1))]}"  # Adjust index because arrays are 0-based
		echo "Select chip: $select_chip_alter_selected_chip"
	elif [[ " ${chip_options[@]} " =~ " ${chip_alter} " ]]; then
		# If the input is a valid chip name
		select_chip_alter_selected_chip="$chip_alter"
		echo "Select chip: $select_chip_alter_selected_chip"
	else
		# If the input is invalid (not a valid number or chip name)
		echo "Invalid input: $chip_alter"
		echo "** Must be one of ${chip_options[*]}"
		CHIP_ALTER_ERROR=1
		return -1
	fi

	# Ask for baud rate selection
	echo "Select debug uart baud rate:"
	for i in "${!baud_rate_options[@]}"; do
		echo "    $((i + 1)) ${baud_rate_options[$i]}"  # Display the index and baud rate
	done

	if [[ "$last_selected_baud_rate" != "none" ]]; then
		echo -n "Which baud rate would you like? [$last_selected_baud_rate]: "
	else
		echo -n "Which baud rate would you like? "
	fi
	read baud_rate

	# If the input is empty, use the last selected baud rate or default to a valid one
	if [ -z "${baud_rate}" ]; then
		if [ -z "${last_selected_baud_rate}" ]; then
			baud_rate="115200"  # Default to 115200 if nothing is selected
		else
			baud_rate=${last_selected_baud_rate}
		fi
	fi

	# Check if the input is a valid number within the available chip options
	if [[ "${baud_rate}" =~ ^[0-9]+$ ]] && ((baud_rate >= 1 && baud_rate <= ${#baud_rate_options[@]})); then
		# Validate the number and select the baud rate accordingly
		select_baud_rate="${baud_rate_options[$((baud_rate - 1))]}"  # Adjust index because arrays are 0-based
		echo "Select baud rate: $select_baud_rate"
	elif [[ " ${baud_rate_options[@]} " =~ " ${baud_rate} " ]]; then
		# If the input is baud rate
		select_baud_rate="$baud_rate"
		echo "Select baud rate: $select_baud_rate"
	else
		# If the input is invalid (not a valid number or chip name)
		echo "Invalid input: $baud_rate, set to default 1500000"
		select_baud_rate="1500000"  # Default to 115200 if nothing is selected
	fi

	# Save selections to files
	echo "$select_chip_alter_selected_chip" > "$last_selection_file"
	echo "$select_baud_rate" > "$baud_rate_file"

	CHIP_ALTER_ERROR=0
	echo
}


function select_lunch_hook_action()
{
	echo "Setting quick_config for chip alter $select_chip_alter_selected_chip"

	local v821_chips=("V821L2-XXX" "V821M2-WXX" "V821L2-WXX" "V521D2-WXX")
	local is_v821_chip=0

	for chip in "${v821_chips[@]}"; do
		if [[ "$select_chip_alter_selected_chip" == "$chip" ]]; then
			is_v821_chip=1
			break
		fi
	done

	if [[ $is_v821_chip -eq 1 ]]; then
		quick_config config_v821
	else
		quick_config config_v821b
	fi

	echo "Setting quick_config for debug baud rate $select_baud_rate"
	quick_config config_debug_baud_$select_baud_rate
	return 0;
}
