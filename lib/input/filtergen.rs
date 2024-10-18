use nom::{
    IResult,
    Parser,
    branch::alt,
    bytes::complete::tag,
    combinator::value,
    error::VerboseError,
};

#[derive(Debug, Eq, PartialEq, Clone, Copy)]
pub enum Keyword {
    Accept,
    Dest,
    DPort,
    Drop,
    Forward,
    IcmpType,
    Input,
    Local,
    Log,
    Masq,
    OneWay,
    Output,
    Proto,
    Proxy,
    Redirect,
    Reject,
    Source,
    SPort,
    Text,
}

#[derive(Debug, Eq, PartialEq, Clone, Copy)]
pub enum Option {
    Local,
    Forward,
    OneWay,
    Log,
}

fn parse_option(input: &str) -> IResult<&str, Option, VerboseError<&str>> {
    alt((
        value(Option::Local, tag("local")),
        value(Option::Forward, tag("forward")),
        value(Option::OneWay, tag("oneway")),
        value(Option::Log, tag("log") ),
    )).parse(input)
}

#[test]
fn parse_option_test() {
    assert_eq!(parse_option("local"), Ok(("", Option::Local)));
    assert_eq!(parse_option("forward"), Ok(("", Option::Forward)));
    assert_eq!(parse_option("oneway"), Ok(("", Option::OneWay)));
    assert_eq!(parse_option("log"), Ok(("", Option::Log)));    
}
