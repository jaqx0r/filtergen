use nom::{
    branch::alt,
    bytes::complete::tag,
    character::complete::{alpha1, multispace0},
    combinator::{map, opt, value},
    error::VerboseError,
    sequence::{pair, preceded},
    IResult, Parser,
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

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum FilterOption {
    Local,
    Forward,
    OneWay,
    Log(Option<String>),
}

fn parse_option(input: &str) -> IResult<&str, FilterOption, VerboseError<&str>> {
    alt((
        value(FilterOption::Local, tag("local")),
        value(FilterOption::Forward, tag("forward")),
        value(FilterOption::OneWay, tag("oneway")),
        map(
            preceded(pair(tag("log"), multispace0), opt(alpha1)),
            |s: Option<&str>| FilterOption::Log(s.map(String::from)),
        ),
    ))
    .parse(input)
}

#[test]
fn parse_option_test() {
    assert_eq!(parse_option("local"), Ok(("", FilterOption::Local)));
    assert_eq!(parse_option("forward"), Ok(("", FilterOption::Forward)));
    assert_eq!(parse_option("oneway"), Ok(("", FilterOption::OneWay)));
    assert_eq!(parse_option("log"), Ok(("", FilterOption::Log(None))));
    assert_eq!(
        parse_option("log text"),
        Ok(("", FilterOption::Log(Some("text".to_string()))))
    );
}
