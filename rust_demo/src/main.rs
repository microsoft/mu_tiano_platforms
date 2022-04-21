// Copyright (c) Microsoft Corporation
//
// SPDX-License-Identifier: BSD-2-Clause-Patent

#![cfg_attr(not(test), no_main)]
#![cfg_attr(not(test), no_std)]

mod pio;
mod print;
mod trait_demo;

use core::result::Result;
use heapless::String;
use r_efi::efi;

use trait_demo::{MyBologna, MySalami, Tupleize};

#[export_name = "efi_main"]
pub extern "C" fn app_entry(_h: efi::Handle, _st: *mut efi::SystemTable) -> efi::Status {
    // NOTE: First, before anything, we need a panic handler (see below).
    //       There are some ongoing efforts (e.g. https://github.com/Rust-for-Linux)
    //       that are trying to address this requirement, but it's not complete yet.

    //
    // DEMO 1
    //
    // Leverages pio and print
    println!("\n\n\n======================================== DEMO 1");
    println!("Hello, world!");

    //
    // DEMO 2
    //
    // Leverages trait_demo
    println!("\n\n\n======================================== DEMO 2");
    let salami = MySalami {
        firstname: &"Edna",
        secondname: &"Krabapple",
    };

    let bologna = MyBologna {
        name_the_first: &"Homer",
        name_the_second: &"Simpson",
    };

    println!("{:#?}", salami);
    println!("{:#?}", bologna);

    //
    // DEMO 3
    //
    // Leverages trait_demo and frobnicate (below)
    println!("\n\n\n======================================== DEMO 3");
    let mut out_str: String<0x4000> = String::new();
    if let Err(e) = frobnicate(&salami, &mut out_str) {
        return e;
    }
    println!("{}", out_str);
    out_str.clear();
    if let Err(e) = frobnicate(&bologna, &mut out_str) {
        return e;
    }
    println!("{}", out_str);

    println!("\n\n\nTHANKS FOR ATTENDING!!\n");

    efi::Status::SUCCESS
}

//
// Panic Handler
// Make sure this doesn't allocate, otherwise you'll play hell with your allocator.
//

#[cfg(not(test))]
#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

//
// Generic Code
// The `frobnicate` function doesn't need to know anything about the types
// that are being passed in. Only that the parameter `tupleizable` implements
// the `Tupleize` trait and that it will return str references, and that the
// parameter `output` implements the `Write` trait which can be used to
// concatenate those references.
//

fn frobnicate(
    tupleizable: &dyn Tupleize<Element = &str>,
    output: &mut dyn core::fmt::Write,
) -> Result<usize, efi::Status> {
    let (thingone, thingtwo) = tupleizable.make_tuple();
    let total_size = thingone.as_bytes().len() + thingtwo.as_bytes().len() + " ".as_bytes().len();

    match write!(output, "{}, {}", thingtwo, thingone).map_err(|_| efi::Status::BAD_BUFFER_SIZE) {
        Ok(_) => Ok(total_size),
        Err(_) => Err(efi::Status::BAD_BUFFER_SIZE),
    }
}
