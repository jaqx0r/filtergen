use ipnet::IpNet;

/// Filter is an intermediate representation of filter policy rules.
#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Filter<'a> {
    /// Represents a interface, in or outbound.
    Interface { direction: Direction, name: &'a str },

    /// Action to perform when a match occurs.
    Target(Target),

    /// A message to log when a match occurs.
    LogMsg(&'a str),

    /// A routing action to perform when a match occurs.
    RouteType,

    ///
    SourceHost(AddrSpec<'a>),
    DestHost(AddrSpec<'a>),

}

#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Direction {
    Input,
    Output,
}


#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Target {
    Accept,
    Reject,
    Drop,
}


#[derive(Debug, Eq, PartialEq, Clone)]
pub struct AddrSpec<'a> {
    net: IpNet,
    network: &'a str,
    mask: &'a str,
}
