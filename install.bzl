"""Install rules.

Based on the GNU Makefile standard targets: https://www.gnu.org/prep/standards/html_node/Standard-Targets.html
"""

load("@bazel_skylib//rules:common_settings.bzl", "BuildSettingInfo")

def _install_impl(ctx):
    """Install some targets somewhere in the filesystem."""

    # Like https://www.gnu.org/prep/standards/html_node/DESTDIR.html#DESTDIR
    prefix = ctx.attr.prefix[BuildSettingInfo].value
    output = ctx.actions.declare_file("install_script")
    commands = "#!/usr/bin/env bash\n" + RUNFILES_BOILERPLATE
    runfiles = ctx.runfiles(files = ctx.files._runfiles_helper)
    for target in ctx.attr.targets:
        target_dir = prefix + ctx.attr.targets.get(target)
        commands += "install -d {}\n".format(target_dir)
        mode = "-m 644"

        # Like https://www.gnu.org/prep/standards/html_node/Command-Variables.html#Command-Variables
        if (target[DefaultInfo].files_to_run and
            target[DefaultInfo].files_to_run.executable and
            not target[DefaultInfo].files_to_run.executable.is_source):
            mode = ""
        installables = ["$(rlocation _main/{})".format(x.short_path) for x in target[DefaultInfo].files.to_list()]
        commands += "install {mode} {target} {dest}\n".format(mode = mode, target = " ".join(installables), dest = target_dir)
        runfiles = runfiles.merge_all([
            target[DefaultInfo].default_runfiles,
            ctx.runfiles(target[DefaultInfo].files.to_list()),
        ])
    ctx.actions.write(
        output = output,
        is_executable = True,
        content = commands,
    )
    return [
        DefaultInfo(
            executable = output,
            runfiles = runfiles,
        ),
    ]

RUNFILES_BOILERPLATE = """
# --- begin runfiles.bash initialization v3 ---
# Copy-pasted from the Bazel Bash runfiles library v3.
set -uo pipefail; set +e; f=bazel_tools/tools/bash/runfiles/runfiles.bash
source "${RUNFILES_DIR:-/dev/null}/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "${RUNFILES_MANIFEST_FILE:-/dev/null}" | cut -f2- -d' ')" 2>/dev/null || \
  source "$0.runfiles/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  { echo>&2 "ERROR: cannot find $f"; exit 1; }; f=; set -e
# --- end runfiles.bash initialization v3 ---
"""

install = rule(
    implementation = _install_impl,
    doc = "Install some targets to the filesystem.",
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
        ),
        "targets": attr.label_keyed_string_dict(
            doc = "Installation targets of label:destination",
            allow_files = True,
        ),
        "prefix": attr.label(
            mandatory = True,
            doc = "Installation prefix directory as a build setting.",
        ),
        # Bazel ships with a useful bash function for querying the absolute path to runfiles at
        # runtime.
        "_runfiles_helper": attr.label(
            default = "@rules_shell//shell/runfiles",
            doc = "Label pointing to bash runfile helper",
        ),
    },
    executable = True,
)
