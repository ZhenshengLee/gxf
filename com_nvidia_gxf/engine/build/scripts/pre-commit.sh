#!/usr/bin/env bash
#####################################################################################
# Copyright (c) 2019-2024, NVIDIA CORPORATION. All rights reserved.

# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto. Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.
#####################################################################################

## Client side pre-commit hook
# To enable, copy or symlink this file into your
# .git/hooks folder
# cp engine/build/scripts/pre-commit.sh .git/hooks/pre-commit
# ln -s `pwd`/engine/build/scripts/pre-commit.sh .git/hooks/pre-commit

## Exit on errors
#  If you want to prevent the commit from being created if an error
#  is detected here, uncomment the following lines
#trap 'cancel_commit' ERR
#trap 'previous_command=$this_command; this_command=$BASH_COMMAND' DEBUG
#cancel_commit(){
#    echo "Error running pre-commit hook: $previous_command"
#    exit 1
#}

# If there are whitespace errors, print the offending file names and fail.
# See https://git-scm.com/docs/git-diff-index#Documentation/git-diff-index.txt---check

# Source the helper functions
source "engine/build/scripts/utility_functions.sh"

change_to_workspace_dir

git diff-index --check --cached HEAD --

log_message "Reformatting files"
if ! ./engine/build/scripts/before_commit.sh; then
    log_error "engine/build/scripts/before_commit.sh script failed. Please check the output and fix any issues."
    exit 1
fi


log_message "Checking for linter errors"
if ! bazel test //... --config lint; then
    log_error "Linting errors detected."
    log_error "Please remove linting errors before committing."
    exit 1
fi
exit 0