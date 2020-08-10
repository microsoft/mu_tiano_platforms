// @file -- lib.rs
// Implementation of the UefiVariablePolicyLib that is written
// natively in Rust.
//
// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: BSD-2-Clause-Patent
//

#![allow(unused)]
#![allow(non_snake_case)]

#![cfg_attr(not(test), no_std)]

extern crate alloc;

#[cfg(not(test))]
extern crate allocation;
extern crate panic;
extern crate variable_policy;

use alloc::slice;
use alloc::string::String;
use alloc::vec::Vec;
use core::mem;
use r_efi::efi;
use variable_policy::{VariablePolicyList, VariablePolicyEntry, RawVariablePolicyEntry, EfiGetVariable, init_get_var};

// TODO: Check for truncation in every cast.

//=====================================================================================================================
//
// LIBRARY IMPLEMENTATION
// This section is the UEFI-facing library interface.
// All transition to Rust (with the exception of VariablePolicyEntry::from_raw()) should be here.
//
struct LibState {
  policy_list: VariablePolicyList,
  interface_locked: bool,
  protection_disabled: bool,
}

static mut INITIALIZED_STATE: Option<LibState> = None;

// Quick helper to assist with CHAR16 -> String.
// TODO: Put this in a single impl.
unsafe fn char16_to_string (string_in: *const efi::Char16) -> String {
  let string_in_u16 = string_in as *const u16;

  // Count the chars in the string.
  let mut string_len: isize = 0;
  while *(string_in_u16.offset(string_len)) != 0 {
    string_len += 1;
  }

  String::from_utf16_lossy(slice::from_raw_parts(string_in_u16, string_len as usize))
}

#[no_mangle]
#[export_name = "RegisterVariablePolicy"]
pub extern "win64" fn register_variable_policy(policy_data: *const RawVariablePolicyEntry) -> efi::Status {
  let state = unsafe { &mut INITIALIZED_STATE };

  match state {
    Some(lib_state) => {
      if lib_state.interface_locked { return efi::Status::WRITE_PROTECTED; }

      // First, we need to turn the raw data into an entry.
      match VariablePolicyEntry::from_raw(policy_data) {
        Ok(new_policy) => {
          match lib_state.policy_list.add_policy(new_policy) {
            true => efi::Status::SUCCESS,
            false => efi::Status::ALREADY_STARTED
          }
        }
        Err(err_status) => err_status
      }
    },
    None => efi::Status::NOT_READY
  }
}

#[no_mangle]
#[export_name = "ValidateSetVariable"]
pub extern "win64" fn validate_set_variable (
    variable_name: *const efi::Char16,
    vendor_guid: *const efi::Guid,
    attributes: u32,
    data_size: usize,
    data: *const u8
    ) -> efi::Status {
  let state = unsafe { &INITIALIZED_STATE };

  match state {
    Some(lib_state) => {
      if lib_state.protection_disabled { return efi::Status::SUCCESS; }

      // Turn EFI params into Rust params.
      let my_vendor_guid: efi::Guid = unsafe { *vendor_guid };
      let my_data: &[u8] = unsafe { slice::from_raw_parts(data, data_size) };
      let my_variable_name = unsafe { char16_to_string(variable_name) };

      // Call policy_list.is_set_variable_valid().
      let is_valid = lib_state.policy_list.is_set_variable_valid(&my_variable_name, &my_vendor_guid, attributes, my_data);
      match is_valid {
        true => efi::Status::SUCCESS,
        false => efi::Status::WRITE_PROTECTED
      }
    },
    None => efi::Status::NOT_READY
  }
}

#[no_mangle]
#[export_name = "DisableVariablePolicy"]
pub extern "win64" fn disable_variable_policy (
    ) -> efi::Status {
  let state = unsafe { &mut INITIALIZED_STATE };

  match state {
    Some(lib_state) => {
      if lib_state.protection_disabled { return efi::Status::ALREADY_STARTED; }
      if lib_state.interface_locked { return efi::Status::WRITE_PROTECTED; }

      lib_state.protection_disabled = true;
      efi::Status::SUCCESS
    },
    None => efi::Status::NOT_READY
  }
}

#[no_mangle]
#[export_name = "DumpVariablePolicy"]
pub extern "win64" fn dump_variable_policy (
    policy: *mut u8,
    size: *mut u32
    ) -> efi::Status {
  let state = unsafe { &INITIALIZED_STATE };

  match state {
    Some(lib_state) => {
      unsafe {
        // Validate some initial state.
        if size.is_null()  || (*size > 0 && policy.is_null()) {
          return efi::Status::INVALID_PARAMETER;
        }

        // First, we need to serialize the policy list.
        let policy_buffer = lib_state.policy_list.to_raw();

        if (*size as usize) < policy_buffer.len() {
          *size = policy_buffer.len() as u32;
          efi::Status::BUFFER_TOO_SMALL
        }
        else {
          for index in 0..policy_buffer.len() {
            *(policy.offset(index as isize)) = policy_buffer[index];
          }
          *size = policy_buffer.len() as u32;
          efi::Status::SUCCESS
        }
      }
    },
    None => efi::Status::NOT_READY
  }
}

#[no_mangle]
#[export_name = "IsVariablePolicyEnabled"]
pub extern "win64" fn is_variable_policy_enabled (
    ) -> efi::Boolean {
  let state = unsafe { &INITIALIZED_STATE };

  match state {
    Some(lib_state) => {
      match lib_state.protection_disabled {
        true => efi::Boolean::FALSE,
        false => efi::Boolean::TRUE,
      }
    },
    None => efi::Boolean::FALSE
  }
}

#[no_mangle]
#[export_name = "LockVariablePolicy"]
pub extern "win64" fn lock_variable_policy (
    ) -> efi::Status {
  let state = unsafe { &mut INITIALIZED_STATE };

  match state {
    Some(lib_state) => {
      match lib_state.interface_locked {
        true => efi::Status::WRITE_PROTECTED,
        false => {
          lib_state.interface_locked = true;
          efi::Status::SUCCESS
        }
      }
    },
    None => efi::Status::NOT_READY
  }
}

#[no_mangle]
#[export_name = "IsVariablePolicyInterfaceLocked"]
pub extern "win64" fn is_variable_policy_interface_locked (
    ) -> efi::Boolean {
  let state = unsafe { &INITIALIZED_STATE };

  match state {
    Some(lib_state) => {
      match lib_state.interface_locked {
        true => efi::Boolean::TRUE,
        false => efi::Boolean::FALSE,
      }
    },
    None => efi::Boolean::FALSE
  }
}

#[no_mangle]
#[export_name = "InitVariablePolicyLib"]
pub extern "win64" fn init_variable_policy_lib (
    get_variable_helper: EfiGetVariable
    ) -> efi::Status {
  let state = unsafe { &INITIALIZED_STATE };

  match state {
    Some(_) => efi::Status::ALREADY_STARTED,
    None => {
      init_get_var(get_variable_helper);
      let new_state = Some(LibState {
        policy_list: VariablePolicyList::new(),
        interface_locked: false,
        protection_disabled: false,
      });
      unsafe { INITIALIZED_STATE = new_state; }

      efi::Status::SUCCESS
    }
  }
}

#[no_mangle]
#[export_name = "IsVariablePolicyLibInitialized"]
pub extern "win64" fn is_variable_policy_lib_initialized (
    ) -> efi::Boolean {
  let state = unsafe { &INITIALIZED_STATE };
  match state {
    Some(_) => efi::Boolean::TRUE,
    None => efi::Boolean::FALSE
  }
}

#[no_mangle]
#[export_name = "DeinitVariablePolicyLib"]
pub extern "win64" fn deinit_variable_policy_lib (
    ) -> efi::Status {
  let state = unsafe { &INITIALIZED_STATE };
  match state {
    Some(_) => {
      // TODO: Make sure that this deinits everything.
      unsafe{ INITIALIZED_STATE = None; }
      efi::Status::SUCCESS
    },
    None => efi::Status::NOT_READY
  }
}
