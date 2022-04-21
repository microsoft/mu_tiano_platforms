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

//
// Now let's take a look at the advantages of defining our own traits.
// As you can see, it doesn't matter what the internal fields are,
// by providing a trait implementation, we can abstract the differences
// between the structures and allow common code to access them the same way.
//

pub trait Tupleize {
    type Element;
    fn make_tuple(&self) -> (Self::Element, Self::Element);
}

impl<'a> Tupleize for MySalami<'a> {
    type Element = &'a str;
    fn make_tuple(&self) -> (Self::Element, Self::Element) {
        (self.firstname, self.secondname)
    }
}
impl<'a> Tupleize for MyBologna<'a> {
    type Element = &'a str;
    fn make_tuple(&self) -> (Self::Element, Self::Element) {
        (self.name_the_first, self.name_the_second)
    }
}
