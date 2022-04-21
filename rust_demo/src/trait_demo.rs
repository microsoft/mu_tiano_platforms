// Copyright (c) Microsoft Corporation
//
// SPDX-License-Identifier: BSD-2-Clause-Patent

use core::fmt::Debug;

//
// Let's start by defining some structs...
//

#[derive(Debug)]
pub struct MySalami<'a> {
    pub firstname: &'a str,
    pub secondname: &'a str,
}

pub struct MyBologna<'a> {
    pub name_the_first: &'a str,
    pub name_the_second: &'a str,
}

//
// You'll note that MySalami derives the Debug trait.
// Trait derivation is a little complicated for now, so just
// know that it means that the compiler will do its best to create
// a "standard" implementation of this trait. Most traits cannot be
// derived, but the most commonly used and simple ones can.
//
// Now, we'll implement a custom version of Debug for MyBologna.
//

impl<'a> Debug for MyBologna<'a> {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        if self.name_the_first.len() > 0 {
            let mut all_chars = self.name_the_first.chars();
            write!(
                f,
                "My bologna has a first name, it's {}",
                all_chars.next().unwrap()
            )?;
            for char in all_chars {
                write!(f, "-{}", char)?;
            }
            writeln!(f, ".")?;
        }
        if self.name_the_second.len() > 0 {
            let mut all_chars = self.name_the_second.chars();
            write!(
                f,
                "My bologna has a second name, it's {}",
                all_chars.next().unwrap()
            )?;
            for char in all_chars {
                write!(f, "-{}", char)?;
            }
            write!(f, ".")?;
        }
        Ok(())
    }
}
