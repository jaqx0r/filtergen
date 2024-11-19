use output::write;

#[repr(C)]
pub struct Filter {}

#[repr(C)]
pub enum FilterType {
    EOF,
    Direction,
    Target,
    Source,
    Dest,
    SPort,
    DPort,
    IcmpType,
    Proto,
    Neg,
    SibList,
    SubGroup,
    Log,
    RType,
    OneWay,
    FilterMax,
    Input,
    Output,
    Accept,
    Drop,
    Reject,
    Masq,
    Redirect,
    OpenBrace,
    CloseBrace,
    Semicolon,
    String,
    Include,
    LocalOnly,
    RoutedOnly,
    Text,
}


#[no_mangle]
pub extern "C" fn fg_nftables(_filter: *const Filter, _flags: u32) -> i32 {
    0
}

#[no_mangle]
pub extern "C" fn flush_nftables(_policy: FilterType) -> i32 {
    write("#!/usr/bin/env nft -f");
    let pol_str = match _policy {
        FilterType::Accept => "accept",
        FilterType::Drop => "drop",
        FilterType::Reject => "",
        _ => "drop",
    };

    let chains = vec!["input", "output", "forward"];

    let mut count = 0;

    write("flush ruleset");
    write("table filter {");
    for chain in chains.iter() {
        write(format!("  chain {} {{", chain).as_str());
        write(
            format!(
                "    type filter hook {} priority 0; policy {};",
                chain, pol_str
            )
            .as_str(),
        );
        write("  }");
        count += 1;
    }
    write("}");

    count
}
