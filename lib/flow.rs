use ipnet::IpNet;

/// Node describes an entity in the network.
///
/// Nodes can contain other Nodes. A containing Node represents a shared
/// security domain, e.g. a network partition or tagged zone. Nodes can begin or
/// end a Flow.
///
/// For example, a Node can represent a single machine, and within that Node be
/// a Node for an application. A Flow to that application informs the filter at
/// the machine (layers 4 and below) and at the application (layer 7 and below).
/// A Node could instead represent a hosting cluster, with a service mesh, in
/// which case the filter is produced accordingly.
///
/// A receiving Node describes the access controls required for that Node. For
/// example, a flow on TCP port 443 suggests a filter for that port and
/// protocol, and accepts the source Node's source IP range if any. But for
/// application Nodes we can also specify that the flow is HTTPS traffic and
/// thus requires TLS 1.3 or higher to secure the flow, and the service
/// listening uses OIDC to authenticate the caller.
///
/// A transit Node sits between two other Nodes with a Flow going in and out.
/// Transit nodes include firewalls, gateways, load balancers, service meshes,
/// VPNs. The type of node determines the filter produced, e.g. a Linux firewall
/// may emit masq rules at layer 3.
// Node information is the location on the network layer.
// interface, mac address
// ip net
pub struct Node<'a> {
    /// name is the identifier of this Node.
    name: Option<&'a str>,
    /// IP Address/range/subnet of this node.
    ip_address: IpNet,
    /// Interface of this node, e.g. `eth0`
    interface: &'a str,
}

/// Flow describes a communication flow between two Nodes. Flows go from
/// initiator to receiver only.
// Flow information is the transport and protocols
// ip proto
// tcp proto, port
// port
// auth
// allowed users/groups
pub struct Flow<'a> {
    /// name is the identifier of this Flow.
    name: Option<&'a str>,
    /// action is the action to take on this flow.  defaults to Accept.
    action: Action,
    /// log a message when this flow is encountered.
    log: Option<&'a str>,

    /// Protocol.
    protocol: Protocol,
    port: u32,
}

/// Action describes actions to take when a flow is sent or received.
pub enum Action {
    Accept,
    Reject,
    Drop,
}

/// Protocol describes the transport protocol.
pub enum Protocol {
    ICMP,
    TCP,
    UDP,
}
