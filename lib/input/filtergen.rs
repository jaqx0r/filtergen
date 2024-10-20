use nom::{
    branch::alt,
    bytes::complete::tag,
    character::complete::{alpha1, alphanumeric1, multispace0, multispace1},
    combinator::{cut, map, opt, recognize, value},
    error::{context, VerboseError},
    multi::{many0, separated_list1},
    sequence::{delimited, pair, preceded, terminated, tuple},
    IResult, Parser,
};

fn parse_identifier(input: &str) -> IResult<&str, &str, VerboseError<&str>> {
    recognize(pair(
        alt((alphanumeric1, tag("_"))),
        many0(alt((alphanumeric1, tag("_")))),
    ))
    .parse(input)
}

#[test]
fn parse_identifier_test() {
    assert_eq!(parse_identifier("1"), Ok(("", "1")));
    assert_eq!(parse_identifier("foo"), Ok(("", "foo")));
    assert_eq!(parse_identifier("_foo"), Ok(("", "_foo")));
    assert_eq!(parse_identifier("foo_oo"), Ok(("", "foo_oo")));
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum FilterOption<'a> {
    Local,
    Forward,
    OneWay,
    Log(Option<&'a str>),
}

fn parse_option(input: &str) -> IResult<&str, FilterOption, VerboseError<&str>> {
    alt((
        value(FilterOption::Local, tag("local")),
        value(FilterOption::Forward, tag("forward")),
        value(FilterOption::OneWay, tag("oneway")),
        map(
            preceded(pair(tag("log"), multispace0), opt(alpha1)),
            |s: Option<&str>| FilterOption::Log(s.to_owned()),
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
        Ok(("", FilterOption::Log(Some("text"))))
    );
}

fn argument_list<'a, O, F>(arg_parser: F) -> impl Parser<&'a str, Vec<O>, VerboseError<&'a str>>
where
    F: Parser<&'a str, O, VerboseError<&'a str>>,
{
    delimited(
        tag("{"),
        preceded(multispace0, separated_list1(multispace1, arg_parser)),
        context("closing brace", cut(preceded(multispace0, tag("}")))),
    )
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Direction<'a> {
    Input(Vec<&'a str>),
    Output(Vec<&'a str>),
}

fn parse_direction(input: &str) -> IResult<&str, Direction, VerboseError<&str>> {
    map(
        pair(
            alt((tag("input"), tag("output"))),
            preceded(
                multispace0,
                alt((
                    argument_list(parse_identifier),
                    map(parse_identifier, |i: &str| vec![i]),
                )),
            ),
        ),
        |(d, v)| match d {
            "input" => Direction::Input(v),
            "output" => Direction::Output(v),
            _ => unreachable!(),
        },
    )
    .parse(input)
}

#[test]
fn parse_direction_test() {
    assert_eq!(
        parse_direction("input eth0"),
        Ok(("", Direction::Input(vec!["eth0"])))
    );
    assert_eq!(
        parse_direction("output lo"),
        Ok(("", Direction::Output(vec!["lo"])))
    );
    assert_eq!(
        parse_direction("input { eth0 lo }"),
        Ok(("", Direction::Input(vec!["eth0", "lo"])))
    );
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Target {
    Accept,
    Reject,
    Drop,
    Masq,
    Proxy,
    Redirect,
}

fn parse_target(input: &str) -> IResult<&str, Target, VerboseError<&str>> {
    alt((
        value(Target::Accept, tag("accept")),
        value(Target::Reject, tag("reject")),
        value(Target::Drop, tag("drop")),
        value(Target::Masq, tag("masq")),
        value(Target::Proxy, tag("proxy")),
        value(Target::Redirect, tag("redirect")),
    ))
    .parse(input)
}

#[test]
fn parse_target_test() {
    assert_eq!(parse_target("accept"), Ok(("", Target::Accept)));
    assert_eq!(parse_target("reject"), Ok(("", Target::Reject)));
    assert_eq!(parse_target("drop"), Ok(("", Target::Drop)));
    assert_eq!(parse_target("masq"), Ok(("", Target::Masq)));
    assert_eq!(parse_target("proxy"), Ok(("", Target::Proxy)));
    assert_eq!(parse_target("redirect"), Ok(("", Target::Redirect)));
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct Host<'a> {
    pub host: &'a str,
    pub mask: Option<&'a str>,
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum HostSpecifier<'a> {
    Source(Vec<Host<'a>>),
    Dest(Vec<Host<'a>>),
}

fn parse_host_argument(input: &str) -> IResult<&str, Host, VerboseError<&str>> {
    map(
        pair(
            parse_identifier,
            opt(preceded(
                tuple((multispace0, tag("/"), multispace0)),
                parse_identifier,
            )),
        ),
        |(h, m)| Host { host: h, mask: m },
    )
    .parse(input)
}

#[test]
fn parse_host_argument_test() {
    assert_eq!(
        parse_host_argument("hostname"),
        Ok((
            "",
            Host {
                host: "hostname",
                mask: None
            }
        ))
    );
    assert_eq!(
        parse_host_argument("hostname/mask"),
        Ok((
            "",
            Host {
                host: "hostname",
                mask: Some("mask")
            }
        ))
    );
    assert_eq!(
        parse_host_argument("hostname / mask"),
        Ok((
            "",
            Host {
                host: "hostname",
                mask: Some("mask")
            }
        ))
    );
}

fn parse_host(input: &str) -> IResult<&str, HostSpecifier, VerboseError<&str>> {
    map(
        pair(
            alt((tag("source"), tag("dest"))),
            preceded(
                multispace0,
                alt((
                    argument_list(parse_host_argument),
                    map(parse_host_argument, |h| vec![h]),
                )),
            ),
        ),
        |(h, v)| match h {
            "source" => HostSpecifier::Source(v),
            "dest" => HostSpecifier::Dest(v),
            _ => unreachable!(),
        },
    )
    .parse(input)
}

#[test]
fn parse_host_test() {
    assert_eq!(
        parse_host("source host"),
        Ok((
            "",
            HostSpecifier::Source(vec![Host {
                host: "host",
                mask: None
            }])
        ))
    );

    assert_eq!(
        parse_host("dest host/mask"),
        Ok((
            "",
            HostSpecifier::Dest(vec![Host {
                host: "host",
                mask: Some("mask")
            }])
        ))
    );

    assert_eq!(
        parse_host("dest { host/mask host2}"),
        Ok((
            "",
            HostSpecifier::Dest(vec![
                Host {
                    host: "host",
                    mask: Some("mask")
                },
                Host {
                    host: "host2",
                    mask: None
                }
            ])
        ))
    );
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub struct PortRange<'a> {
    pub min: &'a str,
    pub max: Option<&'a str>,
}

fn parse_port_argument(input: &str) -> IResult<&str, PortRange, VerboseError<&str>> {
    map(
        pair(
            parse_identifier,
            opt(preceded(
                tuple((multispace0, tag(":"), multispace0)),
                parse_identifier,
            )),
        ),
        |(min, max)| PortRange { min, max },
    )
    .parse(input)
}

#[test]
fn parse_port_argument_test() {
    assert_eq!(
        parse_port_argument("1"),
        Ok((
            "",
            PortRange {
                min: "1",
                max: None
            }
        ))
    );
    assert_eq!(
        parse_port_argument("ssh"),
        Ok((
            "",
            PortRange {
                min: "ssh",
                max: None
            }
        ))
    );
    assert_eq!(
        parse_port_argument("1:1024"),
        Ok((
            "",
            PortRange {
                min: "1",
                max: Some("1024")
            }
        ))
    );
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Port<'a> {
    Source(Vec<PortRange<'a>>),
    Dest(Vec<PortRange<'a>>),
}

fn parse_port(input: &str) -> IResult<&str, Port, VerboseError<&str>> {
    map(
        pair(
            alt((tag("sport"), tag("dport"))),
            preceded(
                multispace0,
                alt((
                    argument_list(parse_port_argument),
                    map(parse_port_argument, |p| vec![p]),
                )),
            ),
        ),
        |(p, v)| match p {
            "sport" => Port::Source(v),
            "dport" => Port::Dest(v),
            _ => unreachable!(),
        },
    )
    .parse(input)
}

#[test]
fn parse_port_test() {
    assert_eq!(
        parse_port("sport ssh"),
        Ok((
            "",
            Port::Source(vec![PortRange {
                min: "ssh",
                max: None
            }])
        ))
    );
    assert_eq!(
        parse_port("dport 0:1024"),
        Ok((
            "",
            Port::Dest(vec![PortRange {
                min: "0",
                max: Some("1024")
            }])
        ))
    );
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Specifier<'a> {
    FilterOption(FilterOption<'a>),
    Negated(Box<Specifier<'a>>),
    Direction(Direction<'a>),
    Host(HostSpecifier<'a>),
    Port(Port<'a>),
    Target(Target),
}

fn parse_specifier(input: &str) -> IResult<&str, Specifier, VerboseError<&str>> {
    alt((
        map(parse_direction, Specifier::Direction),
        map(parse_host, Specifier::Host),
        map(parse_port, Specifier::Port),
        map(parse_target, Specifier::Target),
        map(parse_option, Specifier::FilterOption),
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
    assert_eq!(
        parse_specifier("accept"),
        Ok(("", Specifier::Target(Target::Accept),))
    );
    assert_eq!(
        parse_specifier("sport ssh"),
        Ok((
            "",
            Specifier::Port(Port::Source(vec![PortRange {
                min: "ssh",
                max: None
            }]))
        ))
    );
}

fn parse_rule(input: &str) -> IResult<&str, Vec<Specifier>, VerboseError<&str>> {
    terminated(many0(preceded(multispace0, parse_specifier)), tag(";")).parse(input)
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
    assert_eq!(
        parse_rule("input eth0 forward;"),
        Ok((
            "",
            vec![
                Specifier::Direction(Direction::Input(vec!["eth0"])),
                Specifier::FilterOption(FilterOption::Forward),
            ],
        ))
    );
    assert_eq!(
        parse_rule("input eth0 accept;"),
        Ok((
            "",
            vec![
                Specifier::Direction(Direction::Input(vec!["eth0"])),
                Specifier::Target(Target::Accept),
            ],
        ))
    );
    assert_eq!(
        parse_rule("input eth0 source internet accept;"),
        Ok((
            "",
            vec![
                Specifier::Direction(Direction::Input(vec!["eth0"])),
                Specifier::Host(HostSpecifier::Source(vec![Host {
                    host: "internet",
                    mask: None
                }])),
                Specifier::Target(Target::Accept),
            ]
        ))
    );
    assert_eq!(
        parse_rule("output eth0 dest ns dport 53 accept;"),
        Ok((
            "",
            vec![
                Specifier::Direction(Direction::Output(vec!["eth0"])),
                Specifier::Host(HostSpecifier::Dest(vec![Host {
                    host: "ns",
                    mask: None
                }])),
                Specifier::Port(Port::Dest(vec![PortRange {
                    min: "53",
                    max: None
                }])),
                Specifier::Target(Target::Accept),
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
        parse(
            "local forward;
forward !local;"
        ),
        Ok((
            "",
            vec![
                vec![
                    Specifier::FilterOption(FilterOption::Local),
                    Specifier::FilterOption(FilterOption::Forward),
                ],
                vec![
                    Specifier::FilterOption(FilterOption::Forward),
                    Specifier::Negated(Box::new(Specifier::FilterOption(FilterOption::Local))),
                ],
            ]
        ))
    )
}
