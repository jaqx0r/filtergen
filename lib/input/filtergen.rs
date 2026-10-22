use winnow::{
    ascii::{alpha1, multispace0, multispace1},
    combinator::{alt, cut_err, delimited, opt, preceded, repeat, separated, terminated},
    error::{ContextError, ErrMode},
    token::{literal, take_until, take_while},
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

fn parse_string_literal<'a>(input: &mut &'a str) -> ModalResult<&'a str> {
    delimited('"', take_until(0.., '"'), '"').parse_next(input)
}

#[test]
fn parse_string_literal_test() {
    assert_eq!(Ok(("", "")), parse_string_literal.parse_peek("\"\""));
    assert_eq!(Ok(("", "a")), parse_string_literal.parse_peek("\"a\""));
    assert_eq!(Ok(("", "a1")), parse_string_literal.parse_peek("\"a1\""));
    assert_eq!(Ok(("", "a 1")), parse_string_literal.parse_peek("\"a 1\""));
    assert_eq!(Ok(("", "a_1")), parse_string_literal.parse_peek("\"a_1\""));
}

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
        ('{', multispace0),
        //alt((
        separated(0.., arg_parser, multispace1),
        //  preceded(peek('}'), cut_err(empty.value(Vec::new()))).context(StrContext::Expected(
        //      StrContextValue::Description("at least one element required inside the braces"),
        //  )),
        //)),
        cut_err((multispace0, '}')),
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

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Specifier<'a> {
    FilterOption(FilterOption<'a>),
    Negated(Box<Specifier<'a>>),
    Direction(Direction<'a>),
    Host(HostSpecifier<'a>),
    Port(Port<'a>),
    Target(Target),
    Protocol(Vec<Protocol<'a>>),
    Compound(Vec<Vec<Specifier<'a>>>),
    ChainGroup((Option<&'a str>, Vec<Vec<Specifier<'a>>>)),
}

fn parse_specifier_list<'a>(input: &mut &'a str) -> ModalResult<Vec<Specifier<'a>>> {
    repeat(0.., parse_specifier).parse_next(input)
}

fn parse_subrule_list<'a>(input: &mut &'a str) -> ModalResult<Vec<Vec<Specifier<'a>>>> {
    separated(0.., parse_specifier_list, ';').parse_next(input)
}

fn parse_specifier<'a>(input: &mut &'a str) -> ModalResult<Specifier<'a>> {
    preceded(
        multispace0,
        alt((
            parse_direction.map(Specifier::Direction),
            parse_host.map(Specifier::Host),
            parse_port.map(Specifier::Port),
            parse_target.map(Specifier::Target),
            parse_protocol.map(Specifier::Protocol),
            parse_option.map(Specifier::FilterOption),
            preceded(("!", multispace0), parse_specifier).map(|s| Specifier::Negated(Box::new(s))),
            delimited('{', parse_subrule_list, cut_err((multispace0, '}')))
                .map(Specifier::Compound),
            delimited(
                ("[", multispace0),
                (
                    opt(terminated(parse_string_literal, multispace1)),
                    parse_subrule_list,
                ),
                cut_err((multispace0, "]")),
            )
            .map(Specifier::ChainGroup),
        )),
    )
    .parse_next(input)
}

#[test]
fn parse_specifier_test() {
    assert_eq!(
        Ok(("", Specifier::FilterOption(FilterOption::Local))),
        parse_specifier.parse_peek("local")
    );
    assert_eq!(
        Ok((
            "",
            Specifier::Negated(Box::new(Specifier::FilterOption(FilterOption::Local)))
        )),
        parse_specifier.parse_peek("!local")
    );
    assert_eq!(
        Ok(("", Specifier::Target(Target::Accept),)),
        parse_specifier.parse_peek("accept")
    );
    assert_eq!(
        Ok((
            "",
            Specifier::Port(Port::Source(vec![PortRange {
                min: "ssh",
                max: None
            }]))
        )),
        parse_specifier.parse_peek("sport ssh")
    );
    assert_eq!(
        Ok(("", Specifier::Protocol(vec![Protocol::TCP]))),
        parse_specifier.parse_peek("proto tcp")
    );
    assert_eq!(
        Ok((
            "",
            Specifier::Compound(vec![
                vec![Specifier::Protocol(vec![Protocol::TCP])],
                vec![Specifier::Protocol(vec![Protocol::UDP])]
            ])
        )),
        parse_specifier.parse_peek("{ proto tcp; proto udp }")
    );
    assert_eq!(
        Ok((
            "",
            Specifier::ChainGroup((
                None,
                vec![
                    vec![
                        Specifier::Protocol(vec![Protocol::TCP]),
                        Specifier::Port(Port::Dest(vec![PortRange {
                            min: "123",
                            max: None
                        }])),
                        Specifier::Target(Target::Accept)
                    ],
                    vec![Specifier::Target(Target::Drop)]
                ]
            ))
        )),
        parse_specifier.parse_peek("[ proto tcp dport 123 accept; drop ]")
    );
}

fn parse_rule<'a>(input: &mut &'a str) -> ModalResult<Vec<Specifier<'a>>> {
    terminated(parse_specifier_list, (multispace0, ';')).parse_next(input)
}

#[test]
fn parse_rule_test() {
    assert_eq!(
        Ok((
            "",
            vec![
                Specifier::FilterOption(FilterOption::Local),
                Specifier::FilterOption(FilterOption::Forward),
                Specifier::Negated(Box::new(Specifier::FilterOption(FilterOption::Local))),
            ]
        )),
        parse_rule.parse_peek("local forward !local;")
    );
    assert_eq!(
        Ok((
            "",
            vec![
                Specifier::Direction(Direction::Input(vec!["eth0"])),
                Specifier::FilterOption(FilterOption::Forward),
            ],
        )),
        parse_rule.parse_peek("input eth0 forward;")
    );
    assert_eq!(
        Ok((
            "",
            vec![
                Specifier::Direction(Direction::Input(vec!["eth0"])),
                Specifier::Target(Target::Accept),
            ],
        )),
        parse_rule.parse_peek("input eth0 accept;")
    );
    assert_eq!(
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
        )),
        parse_rule.parse_peek("input eth0 source internet accept;")
    );
    assert_eq!(
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
                Specifier::Protocol(vec![Protocol::TCP, Protocol::UDP]),
                Specifier::Target(Target::Accept),
            ]
        )),
        parse_rule.parse_peek("output eth0 dest ns dport 53 proto {tcp udp} accept;")
    );
    assert_eq!(
        Ok((
            "",
            vec![
                Specifier::Direction(Direction::Input(vec!["eth0"])),
                Specifier::Host(HostSpecifier::Source(vec![Host {
                    host: "wwwserv",
                    mask: None
                }])),
                Specifier::Host(HostSpecifier::Dest(vec![Host {
                    host: "dbserv",
                    mask: None
                }])),
                Specifier::ChainGroup((
                    Some("web_to_dbserv"),
                    vec![
                        vec![
                            Specifier::Protocol(vec![Protocol::TCP]),
                            Specifier::Port(Port::Dest(vec![PortRange {
                                min: "1521",
                                max: None
                            }])),
                            Specifier::Target(Target::Accept),
                        ],
                        vec![
                            Specifier::Protocol(vec![Protocol::TCP]),
                            Specifier::Port(Port::Dest(vec![PortRange {
                                min: "appserv1",
                                max: None
                            }])),
                            Specifier::Target(Target::Accept),
                        ],
                        vec![
                            Specifier::Protocol(vec![Protocol::TCP]),
                            Specifier::Port(Port::Dest(vec![PortRange {
                                min: "appserv2",
                                max: None
                            }])),
                            Specifier::Target(Target::Accept),
                        ],
                        vec![
                            Specifier::Protocol(vec![Protocol::TCP]),
                            Specifier::Port(Port::Dest(vec![PortRange {
                                min: "appserv3",
                                max: None
                            }])),
                            Specifier::Target(Target::Accept),
                        ],
                        vec![Specifier::Target(Target::Drop)],
                        vec![],
                    ]
                ))
            ]
        )),
        parse_rule.parse_peek(
            "input eth0 source wwwserv dest dbserv
 [ \"web_to_dbserv\"
proto tcp dport 1521 accept;
proto tcp dport appserv1 accept;
proto tcp dport appserv2 accept;
proto tcp dport appserv3 accept;
drop;
];"
        ),
    );
}

pub fn parse<'a>(input: &mut &'a str) -> ModalResult<Vec<Vec<Specifier<'a>>>> {
    repeat(0.., parse_rule).parse_next(input)
}

#[test]
fn parse_test() {
    assert_eq!(
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
        )),
        parse.parse_peek(
            "local forward;
forward !local;"
        )
    )
}
