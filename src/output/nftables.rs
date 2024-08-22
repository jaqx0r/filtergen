use std::os::raw::c_char;
use std::ffi::CString;

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

extern {
    /// Output a string, and appends a newline, to the target file.
    /// The target file is a global in the called object.
    fn oputs(s: *const c_char);
}

fn write(s: &str) {
    let c_str = CString::new(s).expect("c string new failed");
    unsafe {
        oputs(c_str.as_ptr());
    }
}



#[no_mangle]
pub extern "C" fn fg_nftables(_filter: *const Filter, _flags: u32) -> i32 {
    0
}


#[no_mangle]
pub extern "C" fn flush_nftables(_policy: FilterType) -> i32 {
    write("nft");
    
    0
}
