use std::ffi::CString;
use std::os::raw::c_char;

extern "C" {
    /// Output a string, and appends a newline, to the target file.
    /// The target file is a global in the called object.
    fn oputs(s: *const c_char);
}

pub fn write(s: &str) {
    let c_str = CString::new(s).expect("c string new failed");
    unsafe {
        oputs(c_str.as_ptr());
    }
}
