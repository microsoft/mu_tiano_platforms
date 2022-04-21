// Copyright (c) Microsoft Corporation
//
// SPDX-License-Identifier: BSD-2-Clause-Patent

#![cfg_attr(not(test), no_main)]
#![cfg_attr(not(test), no_std)]

mod pio;
mod print;
mod trait_demo;

use r_efi::efi;

use trait_demo::{MyBologna, MySalami};

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
