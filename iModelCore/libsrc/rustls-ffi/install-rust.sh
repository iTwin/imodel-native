#!/bin/sh

# Install rustup and cargo-c. This script is intended to be run by the build system, so it does not modify the PATH.
# If they are already installed, this script will do nothing. If they are not installed, this script will install them
# for the current user.

curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --no-modify-path
~/.cargo/bin/cargo install cargo-c
if [ "$(uname -s)" = "Darwin" ]; then
    ~/.cargo/bin/rustup target add aarch64-apple-ios
fi
