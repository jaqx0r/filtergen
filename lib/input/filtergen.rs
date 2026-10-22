use winnow::{
    ascii::{alpha1, multispace0, multispace1},
    combinator::{alt, cut_err, delimited, opt, preceded, separated},
    error::{ContextError, ErrMode},
    token::{literal, take_while},
    ModalResult, Parser,
};

#[cfg(test)]
use pretty_assertions::assert_eq;

fn parse_identifier<'a>(input: &mut &'a str) -> ModalResult<&'a str> {
    take_while(1.., |c: char| c.is_ascii_alphanumeric() || c == '_').parse_next(input)
}

#[test]
fn parse_identifier_test() {
    assert_eq!(Ok(("", "1")), parse_identifier.parse_peek("1"),);
    assert_eq!(Ok(("", "foo")), parse_identifier.parse_peek("foo"),);
    assert_eq!(Ok(("", "_foo")), parse_identifier.parse_peek("_foo"),);
    assert_eq!(Ok(("", "foo_oo")), parse_identifier.parse_peek("foo_oo"),);
}

// fn parse_string_literal(input: &str) -> IResult<&str, &str, VerboseError<&str>> {
//     delimited(
//         tag("\""),
//         recognize(many0(alt((multispace1, alphanumeric1, tag("_"))))),
//         tag("\""),
//     )
//     .parse(input)
// }

// #[test]
// fn parse_string_literal_test() {
//     assert_eq!(parse_string_literal("\"\""), Ok(("", "")));
//     assert_eq!(parse_string_literal("\"a\""), Ok(("", "a")));
//     assert_eq!(parse_string_literal("\"a1\""), Ok(("", "a1")));
//     assert_eq!(parse_string_literal("\"a 1\""), Ok(("", "a 1")));
//     assert_eq!(parse_string_literal("\"a_1\""), Ok(("", "a_1")));
// }

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum FilterOption<'a> {
    Local,
    Forward,
    OneWay,
    Log(Option<&'a str>),
}

fn parse_option<'a>(input: &mut &'a str) -> ModalResult<FilterOption<'a>> {
    alt((
        literal("local").value(FilterOption::Local),
        literal("forward").value(FilterOption::Forward),
        literal("oneway").value(FilterOption::OneWay),
        preceded((literal("log"), multispace0), opt(alpha1))
            .map(|s: Option<&str>| FilterOption::Log(s.to_owned())),
    ))
    .parse_next(input)
}

#[test]
fn parse_option_test() {
    assert_eq!(
        Ok(("", FilterOption::Local)),
        parse_option.parse_peek("local"),
    );
    assert_eq!(
        Ok(("", FilterOption::Forward)),
        parse_option.parse_peek("forward"),
    );
    assert_eq!(
        Ok(("", FilterOption::OneWay)),
        parse_option.parse_peek("oneway"),
    );
    assert_eq!(
        Ok(("", FilterOption::Log(None))),
        parse_option.parse_peek("log"),
    );
    assert_eq!(
        Ok(("", FilterOption::Log(Some("text")))),
        parse_option.parse_peek("log text"),
    );
}

fn argument_list<'a, O, F>(arg_parser: F) -> impl Parser<&'a str, Vec<O>, ErrMode<ContextError>>
where
    F: Parser<&'a str, O, ErrMode<ContextError>>,
{
    delimited(
        preceded('{', multispace0),
        //alt((
        separated(0.., arg_parser, multispace1),
        //  preceded(peek('}'), cut_err(empty.value(Vec::new()))).context(StrContext::Expected(
        //      StrContextValue::Description("at least one element required inside the braces"),
        //  )),
        //)),
        cut_err(preceded(multispace0, '}')),
    )
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Direction<'a> {
    Input(Vec<&'a str>),
    Output(Vec<&'a str>),
}

fn parse_direction<'a>(input: &mut &'a str) -> ModalResult<Direction<'a>> {
    (
        alt((literal("input"), literal("output"))),
        preceded(
            multispace0,
            alt((
                argument_list(parse_identifier),
                parse_identifier.map(|i: &str| vec![i]),
            )),
        ),
    )
        .map(|(d, v)| match d {
            "input" => Direction::Input(v),
            "output" => Direction::Output(v),
            _ => unreachable!(),
        })
        .parse_next(input)
}

#[test]
fn parse_direction_test() {
    assert_eq!(
        Ok(("", Direction::Input(vec!["eth0"]))),
        parse_direction.parse_peek("input eth0"),
    );
    assert_eq!(
        Ok(("", Direction::Output(vec!["lo"]))),
        parse_direction.parse_peek("output lo"),
    );
    assert_eq!(
        Ok(("", Direction::Input(vec!["eth0", "lo"]))),
        parse_direction.parse_peek("input { eth0 lo }")
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

fn parse_target<'a>(input: &mut &'a str) -> ModalResult<Target> {
    alt((
        literal("accept").value(Target::Accept),
        literal("reject").value(Target::Reject),
        literal("drop").value(Target::Drop),
        literal("masq").value(Target::Masq),
        literal("proxy").value(Target::Proxy),
        literal("redirect").value(Target::Redirect),
    ))
    .parse_next(input)
}

#[test]
fn parse_target_test() {
    assert_eq!(Ok(("", Target::Accept)), parse_target.parse_peek("accept"));
    assert_eq!(Ok(("", Target::Reject)), parse_target.parse_peek("reject"));
    assert_eq!(Ok(("", Target::Drop)), parse_target.parse_peek("drop"));
    assert_eq!(Ok(("", Target::Masq)), parse_target.parse_peek("masq"));
    assert_eq!(Ok(("", Target::Proxy)), parse_target.parse_peek("proxy"));
    assert_eq!(
        Ok(("", Target::Redirect)),
        parse_target.parse_peek("redirect")
    );
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

fn parse_host_argument<'a>(input: &mut &'a str) -> ModalResult<Host<'a>> {
    (
        parse_identifier,
        opt(preceded((multispace0, '/', multispace0), parse_identifier)),
    )
        .map(|(h, m)| Host { host: h, mask: m })
        .parse_next(input)
}

#[test]
fn parse_host_argument_test() {
    assert_eq!(
        Ok((
            "",
            Host {
                host: "hostname",
                mask: None
            }
        )),
        parse_host_argument.parse_peek("hostname")
    );
    assert_eq!(
        Ok((
            "",
            Host {
                host: "hostname",
                mask: Some("mask")
            }
        )),
        parse_host_argument.parse_peek("hostname/mask")
    );
    assert_eq!(
        Ok((
            "",
            Host {
                host: "hostname",
                mask: Some("mask")
            }
        )),
        parse_host_argument.parse_peek("hostname / mask")
    );
}

fn parse_host<'a>(input: &mut &'a str) -> ModalResult<HostSpecifier<'a>> {
    (
        alt((literal("source"), literal("dest"))),
        preceded(
            multispace0,
            alt((
                argument_list(parse_host_argument),
                parse_host_argument.map(|h| vec![h]),
            )),
        ),
    )
        .map(|(h, v)| match h {
            "source" => HostSpecifier::Source(v),
            "dest" => HostSpecifier::Dest(v),
            _ => unreachable!(),
        })
        .parse_next(input)
}

#[test]
fn parse_host_test() {
    assert_eq!(
        Ok((
            "",
            HostSpecifier::Source(vec![Host {
                host: "host",
                mask: None
            }])
        )),
        parse_host.parse_peek("source host"),
    );

    assert_eq!(
        Ok((
            "",
            HostSpecifier::Dest(vec![Host {
                host: "host",
                mask: Some("mask")
            }])
        )),
        parse_host.parse_peek("dest host/mask")
    );

    assert_eq!(
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
        )),
        parse_host.parse_peek("dest { host/mask host2}")
    );
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub struct PortRange<'a> {
    pub min: &'a str,
    pub max: Option<&'a str>,
}

fn parse_port_argument<'a>(input: &mut &'a str) -> ModalResult<PortRange<'a>> {
    (
        parse_identifier,
        opt(preceded((multispace0, ':', multispace0), parse_identifier)),
    )
        .map(|(min, max)| PortRange { min, max })
        .parse_next(input)
}

#[test]
fn parse_port_argument_test() {
    assert_eq!(
        Ok((
            "",
            PortRange {
                min: "1",
                max: None
            }
        )),
        parse_port_argument.parse_peek("1")
    );
    assert_eq!(
        Ok((
            "",
            PortRange {
                min: "ssh",
                max: None
            }
        )),
        parse_port_argument.parse_peek("ssh")
    );
    assert_eq!(
        Ok((
            "",
            PortRange {
                min: "1",
                max: Some("1024")
            }
        )),
        parse_port_argument.parse_peek("1:1024")
    );
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Port<'a> {
    Source(Vec<PortRange<'a>>),
    Dest(Vec<PortRange<'a>>),
}

fn parse_port<'a>(input: &mut &'a str) -> ModalResult<Port<'a>> {
    (
        alt((literal("sport"), literal("dport"))),
        preceded(
            multispace0,
            alt((
                argument_list(parse_port_argument),
                parse_port_argument.map(|p| vec![p]),
            )),
        ),
    )
        .map(|(p, v)| match p {
            "sport" => Port::Source(v),
            "dport" => Port::Dest(v),
            _ => unreachable!(),
        })
        .parse_next(input)
}

#[test]
fn parse_port_test() {
    assert_eq!(
        Ok((
            "",
            Port::Source(vec![PortRange {
                min: "ssh",
                max: None
            }])
        )),
        parse_port.parse_peek("sport ssh")
    );
    assert_eq!(
        Ok((
            "",
            Port::Dest(vec![PortRange {
                min: "0",
                max: Some("1024")
            }])
        )),
        parse_port.parse_peek("dport 0:1024")
    );
}

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Protocol<'a> {
    TCP,
    UDP,
    ICMP(Vec<&'a str>), // icmp type list
}

fn parse_protocol_name<'a>(input: &mut &'a str) -> ModalResult<Protocol<'a>> {
    alt((
        literal("tcp").value(Protocol::TCP),
        literal("udp").value(Protocol::UDP),
        preceded(
            (literal("icmp"), multispace0),
            opt(preceded(
                (literal("icmptype"), multispace0),
                alt((
                    argument_list(parse_identifier),
                    parse_identifier.map(|i| vec![i]),
                )),
            )),
        )
        .map(|s: Option<Vec<&str>>| Protocol::ICMP(s.unwrap_or(vec![]))),
    ))
    .parse_next(input)
}

fn parse_protocol<'a>(input: &mut &'a str) -> ModalResult<Vec<Protocol<'a>>> {
    preceded(
        literal("proto"),
        preceded(
            multispace0,
            alt((
                argument_list(parse_protocol_name),
                parse_protocol_name.map(|p| vec![p]),
            )),
        ),
    )
    .parse_next(input)
}

#[test]
fn parse_protocol_test() {
    assert_eq!(
        Ok(("", vec![Protocol::TCP])),
        parse_protocol.parse_peek("proto tcp")
    );
    assert_eq!(
        Ok(("", vec![Protocol::UDP])),
        parse_protocol.parse_peek("proto udp")
    );
    assert_eq!(
        Ok(("", vec![Protocol::ICMP(vec![])])),
        parse_protocol.parse_peek("proto icmp")
    );
    assert_eq!(
        Ok(("", vec![Protocol::TCP, Protocol::UDP])),
        parse_protocol.parse_peek("proto { tcp udp }")
    );
    assert_eq!(
        Ok(("", vec![Protocol::ICMP(vec!["6"])])),
        parse_protocol.parse_peek("proto icmp icmptype 6")
    );
}

// #[derive(Debug, Eq, PartialEq, Clone)]
// pub enum Specifier<'a> {
//     FilterOption(FilterOption<'a>),
//     Negated(Box<Specifier<'a>>),
//     Direction(Direction<'a>),
//     Host(HostSpecifier<'a>),
//     Port(Port<'a>),
//     Target(Target),
//     Protocol(Vec<Protocol<'a>>),
//     Compound(Vec<Vec<Specifier<'a>>>),
//     ChainGroup((Option<&'a str>, Vec<Vec<Specifier<'a>>>)),
// }

// fn parse_specifier(input: &str) -> IResult<&str, Specifier, VerboseError<&str>> {
//     alt((
//         map(parse_direction, Specifier::Direction),
//         map(parse_host, Specifier::Host),
//         map(parse_port, Specifier::Port),
//         map(parse_target, Specifier::Target),
//         map(parse_protocol, Specifier::Protocol),
//         map(parse_option, Specifier::FilterOption),
//         map(
//             preceded(pair(tag("!"), multispace0), parse_specifier),
//             |s| Specifier::Negated(Box::new(s)),
//         ),
//         map(
//             delimited(
//                 terminated(tag("{"), multispace0),
//                 separated_list0(
//                     tuple((multispace0, tag(";"), multispace0)),
//                     separated_list0(multispace1, parse_specifier),
//                 ),
//                 context("closing brace", cut(preceded(multispace0, tag("}")))),
//             ),
//             Specifier::Compound,
//         ),
//         map(
//             delimited(
//                 terminated(tag("["), multispace0),
//                 pair(
//                     opt(terminated(parse_string_literal, multispace1)),
//                     separated_list0(
//                         tuple((multispace0, tag(";"), multispace0)),
//                         separated_list0(multispace1, parse_specifier),
//                     ),
//                 ),
//                 context(
//                     "closing square bracket",
//                     cut(preceded(multispace0, tag("]"))),
//                 ),
//             ),
//             Specifier::ChainGroup,
//         ),
//     ))
//     .parse(input)
// }

// #[test]
// fn parse_specifier_test() {
//     assert_eq!(
//         parse_specifier("local"),
//         Ok(("", Specifier::FilterOption(FilterOption::Local)))
//     );
//     assert_eq!(
//         parse_specifier("!local"),
//         Ok((
//             "",
//             Specifier::Negated(Box::new(Specifier::FilterOption(FilterOption::Local)))
//         ))
//     );
//     assert_eq!(
//         parse_specifier("accept"),
//         Ok(("", Specifier::Target(Target::Accept),))
//     );
//     assert_eq!(
//         parse_specifier("sport ssh"),
//         Ok((
//             "",
//             Specifier::Port(Port::Source(vec![PortRange {
//                 min: "ssh",
//                 max: None
//             }]))
//         ))
//     );
//     assert_eq!(
//         parse_specifier("proto tcp"),
//         Ok(("", Specifier::Protocol(vec![Protocol::TCP])))
//     );
//     assert_eq!(
//         parse_specifier("{ proto tcp; proto udp }"),
//         Ok((
//             "",
//             Specifier::Compound(vec![
//                 vec![Specifier::Protocol(vec![Protocol::TCP])],
//                 vec![Specifier::Protocol(vec![Protocol::UDP])]
//             ])
//         ))
//     );
//     assert_eq!(
//         parse_specifier("[ proto tcp dport 123 accept; drop ]"),
//         Ok((
//             "",
//             Specifier::ChainGroup((
//                 None,
//                 vec![
//                     vec![
//                         Specifier::Protocol(vec![Protocol::TCP]),
//                         Specifier::Port(Port::Dest(vec![PortRange {
//                             min: "123",
//                             max: None
//                         }])),
//                         Specifier::Target(Target::Accept)
//                     ],
//                     vec![Specifier::Target(Target::Drop)]
//                 ]
//             ))
//         ))
//     );
// }

// fn parse_rule(input: &str) -> IResult<&str, Vec<Specifier>, VerboseError<&str>> {
//     terminated(
//         many0(preceded(multispace0, parse_specifier)),
//         preceded(multispace0, tag(";")),
//     )
//     .parse(input)
// }

// #[test]
// fn parse_rule_test() {
//     assert_eq!(
//         parse_rule("local forward !local;"),
//         Ok((
//             "",
//             vec![
//                 Specifier::FilterOption(FilterOption::Local),
//                 Specifier::FilterOption(FilterOption::Forward),
//                 Specifier::Negated(Box::new(Specifier::FilterOption(FilterOption::Local))),
//             ]
//         ))
//     );
//     assert_eq!(
//         parse_rule("input eth0 forward;"),
//         Ok((
//             "",
//             vec![
//                 Specifier::Direction(Direction::Input(vec!["eth0"])),
//                 Specifier::FilterOption(FilterOption::Forward),
//             ],
//         ))
//     );
//     assert_eq!(
//         parse_rule("input eth0 accept;"),
//         Ok((
//             "",
//             vec![
//                 Specifier::Direction(Direction::Input(vec!["eth0"])),
//                 Specifier::Target(Target::Accept),
//             ],
//         ))
//     );
//     assert_eq!(
//         parse_rule("input eth0 source internet accept;"),
//         Ok((
//             "",
//             vec![
//                 Specifier::Direction(Direction::Input(vec!["eth0"])),
//                 Specifier::Host(HostSpecifier::Source(vec![Host {
//                     host: "internet",
//                     mask: None
//                 }])),
//                 Specifier::Target(Target::Accept),
//             ]
//         ))
//     );
//     assert_eq!(
//         parse_rule("output eth0 dest ns dport 53 proto {tcp udp} accept;"),
//         Ok((
//             "",
//             vec![
//                 Specifier::Direction(Direction::Output(vec!["eth0"])),
//                 Specifier::Host(HostSpecifier::Dest(vec![Host {
//                     host: "ns",
//                     mask: None
//                 }])),
//                 Specifier::Port(Port::Dest(vec![PortRange {
//                     min: "53",
//                     max: None
//                 }])),
//                 Specifier::Protocol(vec![Protocol::TCP, Protocol::UDP]),
//                 Specifier::Target(Target::Accept),
//             ]
//         ))
//     );
//     assert_eq!(
//         parse_rule(
//             "input eth0 source wwwserv dest dbserv
//  [ \"web_to_dbserv\"
// proto tcp dport 1521 accept;
// proto tcp dport appserv1 accept;
// proto tcp dport appserv2 accept;
// proto tcp dport appserv3 accept;
// drop;
// ];"
//         ),
//         Ok((
//             "",
//             vec![
//                 Specifier::Direction(Direction::Input(vec!["eth0"])),
//                 Specifier::Host(HostSpecifier::Source(vec![Host {
//                     host: "wwwserv",
//                     mask: None
//                 }])),
//                 Specifier::Host(HostSpecifier::Dest(vec![Host {
//                     host: "dbserv",
//                     mask: None
//                 }])),
//                 Specifier::ChainGroup((
//                     Some("web_to_dbserv"),
//                     vec![
//                         vec![
//                             Specifier::Protocol(vec![Protocol::TCP]),
//                             Specifier::Port(Port::Dest(vec![PortRange {
//                                 min: "1521",
//                                 max: None
//                             }])),
//                             Specifier::Target(Target::Accept),
//                         ],
//                         vec![
//                             Specifier::Protocol(vec![Protocol::TCP]),
//                             Specifier::Port(Port::Dest(vec![PortRange {
//                                 min: "appserv1",
//                                 max: None
//                             }])),
//                             Specifier::Target(Target::Accept),
//                         ],
//                         vec![
//                             Specifier::Protocol(vec![Protocol::TCP]),
//                             Specifier::Port(Port::Dest(vec![PortRange {
//                                 min: "appserv2",
//                                 max: None
//                             }])),
//                             Specifier::Target(Target::Accept),
//                         ],
//                         vec![
//                             Specifier::Protocol(vec![Protocol::TCP]),
//                             Specifier::Port(Port::Dest(vec![PortRange {
//                                 min: "appserv3",
//                                 max: None
//                             }])),
//                             Specifier::Target(Target::Accept),
//                         ],
//                         vec![Specifier::Target(Target::Drop)],
//                         vec![],
//                     ]
//                 ))
//             ]
//         ))
//     );
// }

// pub fn parse(input: &str) -> IResult<&str, Vec<Vec<Specifier>>, VerboseError<&str>> {
//     many0(preceded(multispace0, parse_rule)).parse(input)
// }

// #[test]
// fn parse_test() {
//     assert_eq!(
//         parse(
//             "local forward;
// forward !local;"
//         ),
//         Ok((
//             "",
//             vec![
//                 vec![
//                     Specifier::FilterOption(FilterOption::Local),
//                     Specifier::FilterOption(FilterOption::Forward),
//                 ],
//                 vec![
//                     Specifier::FilterOption(FilterOption::Forward),
//                     Specifier::Negated(Box::new(Specifier::FilterOption(FilterOption::Local))),
//                 ],
//             ]
//         ))
//     )
// }
