#!/bin/bash
#Support DCF file programming from dcf_dir

DCF_ENTRY_MAX=0

# The path to search through, the search fucntion currently
# requires a full path
DCF_DIR=/home/ddi/code/sqa/config_test/dcf_dir/

# Return the maximum number of dcf entries
function get_dcf_entry_max()
{
  DCF_COUNT=0
  search_dir=$DCF_DIR
  for entry in "$search_dir"/*
  do
    # echo "$entry"
    DCF_COUNT=$(($DCF_COUNT + 1))
  done
  return $DCF_COUNT
}

# Return the DCF name for the first argument
function get_dcf_name()
{
  DCF_COUNT=0
  search_dir=$DCF_DIR
  for entry in "$search_dir"/*
  do
    if [ $DCF_COUNT -eq $1 ]; then
      echo $(basename $entry)
      return 0
    fi
    DCF_COUNT=$(($DCF_COUNT + 1))
  done
  echo "DCF index not found \n"
  return 1
}

