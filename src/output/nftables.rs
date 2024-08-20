#[repr(C)]
pub struct Filter {
}

#[repr(C)]
pub enum FilterType {
    EOF,
    Direction,
    Target, Source,
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
    Accept ,
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
    0
}
