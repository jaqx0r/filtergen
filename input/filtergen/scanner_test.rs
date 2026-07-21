use std::ffi::{c_char, c_int, c_void, CStr, CString};

#[repr(C)]
pub struct SourceFile {
    pub f: *mut c_void, // FILE*
    pub pathname: *const c_char,
    pub lineno: i32,
    pub column: i32,
    pub next: *mut SourceFile,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct ExpectedToken {
    pub token: c_int,
    pub spelling: *const c_char,
}

#[repr(C)]
pub struct TestCase {
    pub name: *const c_char,
    pub input: *const c_char,
    pub expected: *const ExpectedToken,
    pub expected_len: usize,
}

unsafe extern "C" {
    pub static mut filtergen_text: *const c_char;
}

#[link(name = "in_filtergen")]
unsafe extern "C" {
    static mut current_srcfile: *mut SourceFile;
    fn filtergen__scan_string(str: *const c_char) -> *mut c_void;
    fn filtergen_lex(lvalp: *mut c_void, llocp: *mut c_void) -> i32;
    fn filtergen_lex_destroy();
}

#[unsafe(no_mangle)]
pub extern "C" fn filter_error(_locp: *const c_void, fmt: *const c_char, _ap: *mut c_void) {
    unsafe {
        let fmt_str = std::ffi::CStr::from_ptr(fmt).to_string_lossy();
        eprintln!("Filter error: {}", fmt_str);
    }
}

pub struct Scanner {
    ptr: *mut c_void,
}

impl Scanner {
    pub fn new(input: &str) -> Result<Self, ScannerError> {
        let c_str = CString::new(input).map_err(|_| ScannerError::InitError)?;

        let _dummy_file = Box::into_raw(Box::new(SourceFile {
            f: std::ptr::null_mut(),
            pathname: CString::new("test").unwrap().into_raw() as *const c_char,
            lineno: 1,
            column: 1,
            next: std::ptr::null_mut(),
        }));

        unsafe {
            current_srcfile = _dummy_file;
        }

        let ptr = unsafe { filtergen__scan_string(c_str.as_ptr()) };
        if ptr.is_null() {
            return Err(ScannerError::InitError);
        }

        Ok(Scanner { ptr })
    }

    pub fn next_token(&self) -> Result<(Token, String), ScannerError> {
        if self.ptr.is_null() {
            return Err(ScannerError::InitError);
        }
        let lvalp: *mut c_void = std::ptr::null_mut();
        let llocp: *mut c_void = std::ptr::null_mut();

        let token_id = unsafe { filtergen_lex(lvalp, llocp) };
        if token_id == 0 {
            return Ok((Token::EOF, "".to_string()));
        }

        let token = Token::from_i32(token_id).map_err(|_| ScannerError::LexError(token_id))?;

        // Capture spelling from filtergen_text
        let spelling = unsafe {
            if filtergen_text.is_null() {
                "".to_string()
            } else {
                CStr::from_ptr(filtergen_text)
                    .to_string_lossy()
                    .into_owned()
            }
        };

        if spelling.len() > 255 {
            return Err(ScannerError::Mismatch("Spelling too long".to_string()));
        }

        if !token.validate_spelling(&spelling) {
            return Err(ScannerError::Mismatch(format!(
                "Invalid spelling for token: {}",
                spelling
            )));
        }

        Ok((token, spelling))
    }
}

impl Drop for Scanner {
    fn drop(&mut self) {
        unsafe {
            filtergen_lex_destroy();

            // Free SourceFile and pathname
            if !current_srcfile.is_null() {
                let srcfile = Box::from_raw(current_srcfile);
                if !srcfile.pathname.is_null() {
                    let _ = CString::from_raw(srcfile.pathname as *mut c_char);
                }
                current_srcfile = std::ptr::null_mut();
            }
        }
    }
}

#[derive(Debug, PartialEq, Clone, Copy)]
#[repr(i32)]
pub enum Token {
    Unexpected = 258,
    Accept = 259,
    Dest = 260,
    Dport = 261,
    Drop = 262,
    Forward = 263,
    Icmptype = 264,
    Input = 265,
    Lcurly = 266,
    Local = 267,
    Log = 268,
    Lsquare = 269,
    Masq = 270,
    Oneway = 271,
    Output = 272,
    Proto = 273,
    Proxy = 274,
    Rcurly = 275,
    Redirect = 276,
    Reject = 277,
    Rsquare = 278,
    Semicolon = 279,
    Source = 280,
    Sport = 281,
    Text = 282,
    Identifier = 283,
    Dot = 284,
    Slash = 285,
    Bang = 286,
    Colon = 287,
    Star = 288,
    EOF = 0,
}

impl Token {
    pub fn from_i32(val: i32) -> Result<Self, i32> {
        match val {
            258 => Ok(Token::Unexpected),
            259 => Ok(Token::Accept),
            260 => Ok(Token::Dest),
            261 => Ok(Token::Dport),
            262 => Ok(Token::Drop),
            263 => Ok(Token::Forward),
            264 => Ok(Token::Icmptype),
            265 => Ok(Token::Input),
            266 => Ok(Token::Lcurly),
            267 => Ok(Token::Local),
            268 => Ok(Token::Log),
            269 => Ok(Token::Lsquare),
            270 => Ok(Token::Masq),
            271 => Ok(Token::Oneway),
            272 => Ok(Token::Output),
            273 => Ok(Token::Proto),
            274 => Ok(Token::Proxy),
            275 => Ok(Token::Rcurly),
            276 => Ok(Token::Redirect),
            277 => Ok(Token::Reject),
            278 => Ok(Token::Rsquare),
            279 => Ok(Token::Semicolon),
            280 => Ok(Token::Source),
            281 => Ok(Token::Sport),
            282 => Ok(Token::Text),
            283 => Ok(Token::Identifier),
            284 => Ok(Token::Dot),
            285 => Ok(Token::Slash),
            286 => Ok(Token::Bang),
            287 => Ok(Token::Colon),
            288 => Ok(Token::Star),
            0 => Ok(Token::EOF),
            _ => Err(val),
        }
    }

    pub fn validate_spelling(&self, spelling: &str) -> bool {
        match self {
            Token::Accept => spelling == "accept",
            Token::Dest => spelling == "dest",
            Token::Dport => spelling == "dport",
            Token::Drop => spelling == "drop",
            Token::Forward => spelling == "forward",
            Token::Icmptype => spelling == "icmptype",
            Token::Input => spelling == "input",
            Token::Local => spelling == "local",
            Token::Log => spelling == "log",
            Token::Masq => spelling == "masq",
            Token::Oneway => spelling == "oneway",
            Token::Output => spelling == "output",
            Token::Proto => spelling == "proto",
            Token::Proxy => spelling == "proxy",
            Token::Redirect => spelling == "redirect",
            Token::Reject => spelling == "reject",
            Token::Source => spelling == "source",
            Token::Sport => spelling == "sport",
            Token::Text => spelling == "text",
            Token::Lcurly => spelling == "{",
            Token::Rcurly => spelling == "}",
            Token::Lsquare => spelling == "[",
            Token::Rsquare => spelling == "]",
            Token::Semicolon => spelling == ";",
            Token::Colon => spelling == ":",
            Token::Dot => spelling == ".",
            Token::Slash => spelling == "/",
            Token::Bang => spelling == "!",
            Token::Star => spelling == "*",
            Token::Identifier | Token::Unexpected | Token::EOF => true,
        }
    }
}

#[derive(Debug, PartialEq)]
pub enum ScannerError {
    LexError(i32),
    Mismatch(String),
    InitError,
}

pub fn run_test_c(test: &TestCase) -> Result<(), ScannerError> {
    // Convert C strings to Rust
    let name = unsafe { CStr::from_ptr(test.name).to_string_lossy() };
    let input = unsafe { CStr::from_ptr(test.input).to_string_lossy() };

    let scanner = Scanner::new(&input)?;

    for i in 0..test.expected_len {
        let expected = unsafe { *test.expected.add(i) };
        if expected.token == 0 {
            break;
        }

        let (token, spelling) = scanner.next_token()?;
        if token == Token::EOF {
            return Err(ScannerError::Mismatch(format!(
                "Test '{}' failed: expected token, got EOF",
                name
            )));
        }

        // Validate token type
        let expected_token =
            Token::from_i32(expected.token).map_err(|_| ScannerError::LexError(expected.token))?;

        if token != expected_token {
            return Err(ScannerError::Mismatch(format!(
                "Test '{}' failed: expected token {:?}, got {:?}",
                name, expected_token, token
            )));
        }

        // Validate spelling
        let expected_spelling = unsafe { CStr::from_ptr(expected.spelling).to_string_lossy() };

        if spelling != expected_spelling {
            return Err(ScannerError::Mismatch(format!(
                "Test '{}' failed: expected spelling '{}', got '{}'",
                name, expected_spelling, spelling
            )));
        }
    }

    // Check if there are still more tokens
    let (token, _) = scanner.next_token()?;
    if token != Token::EOF {
        return Err(ScannerError::Mismatch(format!(
            "Test '{}' failed: expected EOF, got more tokens",
            name
        )));
    }

    Ok(())
}

#[unsafe(no_mangle)]
pub extern "C" fn run_scanner_tests() -> i32 {
    unsafe {
        let tests = [
            TestCase {
                name: b"c-style comment\0".as_ptr() as *const c_char,
                input: b"/* blah */ word\r\0".as_ptr() as *const c_char,
                expected: [
                    ExpectedToken { token: 283, spelling: b"word\0".as_ptr() as *const c_char },
                ].as_ptr(),
                expected_len: 1,
            },
            TestCase {
                name: b"shell-style hash comment\0".as_ptr() as *const c_char,
                input: b"# blah\rword\r\0".as_ptr() as *const c_char,
                expected: [
                    ExpectedToken { token: 283, spelling: b"word\0".as_ptr() as *const c_char },
                ].as_ptr(),
                expected_len: 1,
            },
            TestCase {
                name: b"line ending with a hash comment\0".as_ptr() as *const c_char,
                input: b"word#blah\rwork\r\0".as_ptr() as *const c_char,
                expected: [
                    ExpectedToken { token: 283, spelling: b"word\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 283, spelling: b"work\0".as_ptr() as *const c_char },
                ].as_ptr(),
                expected_len: 2,
            },
            TestCase {
                name: b"keywords\0".as_ptr() as *const c_char,
                input: b"accept dest dport drop forward icmptype input local log masq oneway output proto proxy redirect reject source sport text\r\0".as_ptr() as *const c_char,
                expected: [
                    ExpectedToken { token: 259, spelling: b"accept\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 260, spelling: b"dest\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 261, spelling: b"dport\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 262, spelling: b"drop\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 263, spelling: b"forward\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 264, spelling: b"icmptype\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 265, spelling: b"input\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 267, spelling: b"local\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 268, spelling: b"log\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 270, spelling: b"masq\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 271, spelling: b"oneway\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 272, spelling: b"output\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 273, spelling: b"proto\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 274, spelling: b"proxy\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 276, spelling: b"redirect\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 277, spelling: b"reject\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 280, spelling: b"source\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 281, spelling: b"sport\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 282, spelling: b"text\0".as_ptr() as *const c_char },
                ].as_ptr(),
                expected_len: 19,
            },
            TestCase {
                name: b"identifiers\0".as_ptr() as *const c_char,
                input: b"37 69 0.0.0.0\r\0".as_ptr() as *const c_char,
                expected: [
                    ExpectedToken { token: 283, spelling: b"37\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 283, spelling: b"69\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 283, spelling: b"0.0.0.0\0".as_ptr() as *const c_char },
                ].as_ptr(),
                expected_len: 3,
            },
            TestCase {
                name: b"netmask\0".as_ptr() as *const c_char,
                input: b"127.0.0.1/255.255.255.255\r\0".as_ptr() as *const c_char,
                expected: [
                    ExpectedToken { token: 283, spelling: b"127.0.0.1\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 285, spelling: b"/\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 283, spelling: b"255.255.255.255\0".as_ptr() as *const c_char },
                ].as_ptr(),
                expected_len: 3,
            },
            TestCase {
                name: b"punctuation\0".as_ptr() as *const c_char,
                input: b"/ { } [ ] ; : ! *\r\0".as_ptr() as *const c_char,
                expected: [
                    ExpectedToken { token: 285, spelling: b"/\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 266, spelling: b"{\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 275, spelling: b"}\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 269, spelling: b"[\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 278, spelling: b"]\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 279, spelling: b";\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 287, spelling: b":\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 286, spelling: b"!\0".as_ptr() as *const c_char },
                    ExpectedToken { token: 288, spelling: b"*\0".as_ptr() as *const c_char },
                ].as_ptr(),
                expected_len: 9,
            },
        ];

        let mut failed = 0;
        for test in &tests {
            if let Err(e) = run_test_c(test) {
                eprintln!(
                    "Test '{}' failed: {:?}",
                    unsafe { CStr::from_ptr(test.name).to_string_lossy() },
                    e
                );
                failed += 1;
            }
        }

        failed
    }
}
