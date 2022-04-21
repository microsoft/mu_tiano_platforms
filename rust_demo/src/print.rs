// Copyright (c) Microsoft Corporation
//
// SPDX-License-Identifier: BSD-2-Clause-Patent

use core::fmt::Write;
use spin::Mutex;

use crate::pio::{Io, Pio};

//
// Construct a simple structure
// We'll leverage the pio structures and traits to
// build an object that can be used to talk to the serial
// port.
//

struct ConOut(Pio<u8>);
impl ConOut {
    pub fn out_bytes(&mut self, out_string: &str) {
        for utf8byte in out_string.bytes() {
            self.0.write(match utf8byte {
                // printable ASCII byte or newline
                0x20..=0x7e | b'\n' | b'\r' => utf8byte,
                // not part of printable ASCII range
                _ => 0xfe,
            });
        }
    }
}

//
// Now for the fun part...
// By implementing this one, special function as part
// of the `Write` Trait, we get access to the entire
// library of formatters in core Rust.
// https://doc.rust-lang.org/core/fmt/trait.Write.html
//
// Don't worry about what a trait is yet, we'll get to that in
// the other demos.
//

impl core::fmt::Write for ConOut {
    fn write_str(&mut self, s: &str) -> core::fmt::Result {
        self.out_bytes(s);
        Ok(())
    }
}

//
// Now we just need to instantiate it somewhere globally...
//

static CON_OUT: Mutex<ConOut> = Mutex::new(unsafe {
    // 0x402 is the Debug Port for Q35
    ConOut(Pio::<u8>::new(0x402))
});

//
// And add some macros to make it easy to call...
//

#[doc(hidden)]
pub fn _print(args: core::fmt::Arguments) {
    let _ = CON_OUT.lock().write_fmt(args);
}

#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => ($crate::print::_print(format_args!($($arg)*)));
}

#[macro_export]
macro_rules! println {
    () => ($crate::print!("\r\n"));
    ($($arg:tt)*) => ($crate::print!("{}\r\n", format_args!($($arg)*)));
}
