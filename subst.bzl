"""Generate files from templates with substitutions."""

def _subst_template_impl(ctx):
    """Implementation of substitution from template."""
    output = ctx.actions.declare_file(ctx.label.name)
    ctx.actions.expand_template(
        template = ctx.file.src,
        output = output,
        substitutions = ctx.attr.substitutions,
    )
    runfiles = ctx.runfiles(files = [output])
    files = depset([output])
    if ctx.attr.executable:
        return [
            DefaultInfo(
                executable = output,
                runfiles = runfiles,
            ),
        ]
    else:
        return [
            DefaultInfo(
                files = files,
                runfiles = runfiles,
            ),
        ]

subst_template = rule(
    implementation = _subst_template_impl,
    attrs = {
        "src": attr.label(
            doc = "Source template to generate from.",
            mandatory = True,
            allow_single_file = True,
        ),
        "substitutions": attr.string_dict(
            doc = "Substitutions to apply to the template",
        ),
        "executable": attr.bool(
            default = False,
            doc = "Set if the substitution is an executable.",
        ),
    },
)
