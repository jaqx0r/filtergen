#[allow(unused_variables)]
use std::path::PathBuf;
use argh::FromArgs;

#[derive(FromArgs)]
#[allow(dead_code)]
/// filtergen
struct Opts {
    /// compile only, no generate
    #[argh(switch, short='c')]
    compile: bool,

    /// don't resolve hostnames or portnames
    #[argh(switch, short='R')]
    noresolve: bool,

    /// source language, default: "filtergen"
    #[argh(option, short='s', default="String::from(\"filtergen\")")]
    source: String,

    /// generate for target, default "iptables"
    #[argh(option, short='t', default="String::from(\"iptables\")")]
    target: String,

    /// don't process input, generate flush rules
    #[argh(option, short='F', arg_name="policy")]
    flush: Option<String>,

    /// write the generated packet filter to filename
    #[argh(option, short='o')]
    output: Option<PathBuf>,

    /// show program version
    #[argh(switch, short='V')]
    version: bool,

    /// source input
    #[argh(positional)]
    input: PathBuf,
}

fn main() {
    let _opts: Opts = argh::from_env();


}
