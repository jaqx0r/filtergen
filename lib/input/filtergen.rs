use nom::{
    branch::alt,
    bytes::complete::tag,
    character::complete::{alpha1, char, multispace0},
    combinator::{map, opt, value},
    error::VerboseError,
    multi::many0,
    sequence::{pair, preceded, terminated},
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

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Specifier {
    FilterOption(FilterOption),
    Negated(Box<Specifier>),
}

fn parse_specifier(input: &str) -> IResult<&str, Specifier, VerboseError<&str>> {
    alt((
        map(parse_option, |o| Specifier::FilterOption(o)),
        map(
            preceded(pair(tag("!"), multispace0), parse_specifier),
            |s| Specifier::Negated(Box::new(s)),
        ),
    ))
    .parse(input)
}

#[test]
fn parse_specifier_test() {
    assert_eq!(
        parse_specifier("local"),
        Ok(("", Specifier::FilterOption(FilterOption::Local)))
    );
    assert_eq!(
        parse_specifier("!local"),
        Ok((
            "",
            Specifier::Negated(Box::new(Specifier::FilterOption(FilterOption::Local)))
        ))
    );
}

fn parse_rule(input: &str) -> IResult<&str, Vec<Specifier>, VerboseError<&str>> {
    terminated(many0(preceded(multispace0, parse_specifier)), char(';')).parse(input)
}

#[test]
fn parse_rule_test() {
    assert_eq!(
        parse_rule("local forward !local;"),
        Ok((
            "",
            vec![
                Specifier::FilterOption(FilterOption::Local),
                Specifier::FilterOption(FilterOption::Forward),
                Specifier::Negated(Box::new(Specifier::FilterOption(FilterOption::Local))),
            ]
        ))
    );
}

pub fn parse(input: &str) -> IResult<&str, Vec<Vec<Specifier>>, VerboseError<&str>> {
    many0(preceded(multispace0, parse_rule)).parse(input)
}

#[test]
fn parse_test() {
    assert_eq!(
        parse("local forward;
forward !local;"),
        Ok(("",
        vec![
            vec![
                Specifier::FilterOption(FilterOption::Local),
                Specifier::FilterOption(FilterOption::Forward),
            ],
            vec![
                Specifier::FilterOption(FilterOption::Forward),
                Specifier::Negated(Box::new(Specifier::FilterOption(FilterOption::Local))),
            ],
        ])))
}
