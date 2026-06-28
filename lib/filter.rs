use ipnet::IpNet;

/// Filter is an intermediate representation of filter policy rules.
#[derive(Debug, Eq, PartialEq, Clone)]
pub enum Filter {
    /// Represents a interface, in or outbound.
    Interface { direction: Direction, name: &str },

    /// Action to perform when a match occurs.
    Target(Target),

    /// A message to log when a match occurs.
    LogMsg(&str),

    /// A routing action to perform when a match occurs.
    RouteType,

    ///
    SourceHost(AddrSpec),
    DestHost(AddrSpec),

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


pub struct AddrSpec {
    net: IpNet,
    network: &str,
    mask: &str,
}
