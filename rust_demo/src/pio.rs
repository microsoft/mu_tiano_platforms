/* @file
Rust-based IO.

Based almost entirely off of:
https://gitlab.redox-os.org/redox-os/syscall/-/blob/master/src/io/io.rs
https://gitlab.redox-os.org/redox-os/syscall/-/blob/master/src/io/pio.rs

Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent

*/

use core::arch::asm;
use core::marker::PhantomData;

//
// The basic structure is very simple.
// It stores the port address so that any struct methods
// (or Trait methods! foreshadowing...) can get to it.
//

/// Generic PIO
#[derive(Copy, Clone)]
pub struct Pio<T> {
    port: u16,
    value: PhantomData<T>,
}
impl<T> Pio<T> {
    // [unsafe] Requires that the caller confirm that the port number
    //          is a valid IO port on the given architecture and that
    //          it supports the access width associated with this type.
    // Create a PIO from a given port
    pub const unsafe fn new(port: u16) -> Self {
        Pio::<T> {
            port,
            value: PhantomData,
        }
    }
}

//
// Now we'll abstract access behind a Trait.
// Don't worry about what a trait is yet; we'll get to that later.
// For now, just think of it as extra methods on the struct.
//
// NOTE: The `write` interface requires `mut` access (exclusive access),
//       but the `read` interface can use shared access.
//

pub trait Io {
    type Value: Copy;

    fn read(&self) -> Self::Value;
    fn write(&mut self, value: Self::Value);
}

/// Read/Write for byte PIO
impl Io for Pio<u8> {
    type Value = u8;

    /// Read
    #[inline(always)]
    fn read(&self) -> u8 {
        let value: u8;
        // [unsafe] Safe as long as the port was created with a valid port address.
        //          http://www.randomhacks.net/2015/11/09/bare-metal-rust-cpu-port-io/
        unsafe {
            asm!("in al, dx", in("dx") self.port, out("al") value, options(nostack, nomem, preserves_flags));
        }
        value
    }

    /// Write
    #[inline(always)]
    fn write(&mut self, value: u8) {
        // [unsafe] Safe as long as the port was created with a valid port address.
        //          http://www.randomhacks.net/2015/11/09/bare-metal-rust-cpu-port-io/
        unsafe {
            asm!("out dx, al", in("dx") self.port, in("al") value, options(nostack, nomem, preserves_flags));
        }
    }
}
