"""
 SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 SPDX-License-Identifier: Apache-2.0

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
"""

"""
General purpose utility functions for bazel. Not specific to any package.
"""

load("@bazel_skylib//rules:diff_test.bzl", "diff_test")
load("@bazel_skylib//rules:write_file.bzl", "write_file")

def deepcopy(obj):
    """ Deepcopy an object that can be represented as a json. """

    # Since Starlark does not have a native way to deepcopy objects we resort to json.
    return json.decode(json.encode(obj))

def remove_duplicate(values):
    """ Remove duplicate values from a list. """

    # If starlark had sets this could be implemented a lot simpler.
    unique_values = []
    for value in values:
        if value not in unique_values:
            unique_values.append(value)
    return unique_values

def package_relative_label(target):
    """
    Bazel's native.package_relative_label() is only available as of bazel 6.1.0. This macro enables
    this functionality in earlier version. If a target string without package is passed (ie. `:bar`)
    this creates a label that is relative to the package from which this macro is called.

    Example: //foo:bar.py -> //foo:bar.py
             :bar.py      -> //foo:bar.py

    Args:
      target (str): A string corresponding to a target.

    Returns:
      Label: The created label corresponding to the target string.
    """
    if target.startswith("@") or target.startswith("//"):
        return Label(target)
    if not target.startswith(":"):
        target = ":" + target
    return Label("//" + native.package_name() + target)

def label_workspace_relative_path(label):
    """
    Get a label's path relative to the workspace.

    Example: //foo:bar.py -> foo/bar.py
             :bar.py      -> foo/bar.py

    Args:
      label (Label): A bazel label.

    Returns:
      str: The label's path relative to the workspace.
    """
    return label.package + "/" + label.name

def package_path_and_name(target):
    """
    Get a target's package path (including the workspace) and the label name.

    Example: //foo:bar.py -> (@workspace//foo, bar.py)
             :bar.py      -> (@workspace//foo, bar.py)

    Args:
      target (str): A string corresponding to a target.

    Returns:
      Tuple(str, str): The path of the package and the name of the label.
    """
    label = package_relative_label(target)
    package_path = "@" + label.workspace_name + "//" + label.package
    name = label.name
    return package_path, name

def copy_generated_file_to_source(bin_target, src_target, tags = [], add_diff_test = True):
    """
    Copies a generated file from the `bazel-bin` folder to the source folder. Also adds a test to
    make sure that the copied file does not diverge from the generated file.

    Args:
      bin_target (str): The autogenerated file target.
      src_target (str): The target where the bin_target will be copied to.
      add_diff_test (bool): If true a test will be generated that checks that the file in the
                            workspace has not diverged from the autogenerated file.
    """

    # Convert the labels to be relative to the package.
    bin_target = package_relative_label(bin_target)
    src_target = package_relative_label(src_target)

    # Get the paths corresponding to the labels.
    bin_path = label_workspace_relative_path(bin_target)
    src_path = label_workspace_relative_path(src_target)

    # Generate a shell script that copies the file from the bin folder to the src folder.
    update_script = "update_{}.sh".format(src_target.name)
    write_file(
        name = "generate_update_script",
        out = update_script,
        content = [
            # This depends on bash, would need tweaks for Windows
            "#!/bin/bash",
            "set -e",
            "cd ${BUILD_WORKSPACE_DIRECTORY:?}",
            "cp -fv bazel-bin/{} {}".format(bin_path, src_path),
        ],
    )

    # Create an executable target that runs the generated update script.
    update_target_name = src_target.name + "-update"
    native.sh_binary(
        name = update_target_name,
        srcs = [update_script],
        data = [bin_target],
        tags = tags,
    )

    # Create test that checks the copied file is the same as the generated file.
    if add_diff_test:
        diff_test(
            name = "_diff_check_" + src_target.name,
            failure_message = "Please run: 'bazel run {}'.".format(package_relative_label(update_target_name)),
            file1 = src_target,
            file2 = bin_target,
        )
