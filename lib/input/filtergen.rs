use nom::{
    IResult,
    Parser,
    branch::alt,
    bytes::complete::tag,
    combinator::map,
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
        map(tag("local"), |_| Option::Local),
        map(tag("forward"), |_| Option::Forward),
        map(tag("oneway"), |_| Option::OneWay),
        map(tag("log"), |_| Option::Log),
    )).parse(input)
}

#[test]
fn parse_option_test() {
    assert_eq!(parse_option("local"), Ok(("", Option::Local)));
    assert_eq!(parse_option("forward"), Ok(("", Option::Forward)));
    assert_eq!(parse_option("oneway"), Ok(("", Option::OneWay)));
    assert_eq!(parse_option("log"), Ok(("", Option::Log)));    
}
