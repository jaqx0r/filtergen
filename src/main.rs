use std::path::PathBuf;
use clap::{Args, Parser, ValueEnum};

#[derive(Parser)]
#[command(version="0.13.1")]
#[allow(dead_code)]
/// filtergen generates packet filtering rules from a high level description language
struct Opts {
    /// compile only, no generate
    #[arg(short, long)]
    compile: bool,

    /// don't resolve hostnames or portnames
    #[arg(short='R', long="no-resolve")]
    noresolve: bool,

    /// source language
    #[arg(short, long, default_value_t=String::from("filtergen"))]
    source: String,

    /// generate for target
    #[arg(short, long, default_value_t=String::from("iptables"))]
    target: String,

    /// write the generated packet filter to filename
    #[arg(short, long)]
    output: Option<PathBuf>,

    #[command(flatten)]
    action: Actions,
}

#[derive(Args)]
#[group(required = true, multiple = false)]
struct Actions {
    /// don't process input, generate flush rules
    #[arg(short='F', long, value_name="POLICY")]
    flush: Option<Policy>,

    /// source input
    input: PathBuf,
}

#[derive(Clone, ValueEnum)]
enum Policy {
    /// Accept all
    Accept,

    /// Drop all
    Drop,

    /// Reject all
    Reject,
}

fn main() {
    let _opts = Opts::parse();
}
