use build_info;
use clap::{Args, Parser, ValueEnum};
use std::fs::File;
use std::io::{self, Write};
use std::path::PathBuf;

#[derive(Parser)]
#[command(version=build_info::STABLE_GIT_DESCRIBE)]
#[allow(dead_code)]
/// filtergen generates packet filtering rules from a high level description language
struct Opts {
    /// compile only, no generate
    #[arg(short, long)]
    compile: bool,

    /// don't resolve hostnames or portnames
    #[arg(short = 'R', long = "no-resolve")]
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
    #[arg(short = 'F', long, value_name = "POLICY")]
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
    let opts = Opts::parse();

    let mut output = match opts.output {
        Some(x) => Box::new(match File::create(x.clone()) {
            Ok(f) => f,
            Err(err) => {
                eprintln!(
                    "filtergen: can't open output file \"{}\": {}\n",
                    x.display(),
                    err
                );
                std::process::exit(1);
            }
        }) as Box<dyn Write>,
        None => Box::new(io::stdout()) as Box<dyn Write>,
    };

    output.write(b"Hi\n").unwrap();
}
