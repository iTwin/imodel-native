#!/bin/sh

# Install rustup and cargo-c. This script will be run for a developer at build time if they set INSTALL_RUST_CARGO.
# If they are not installed, this script will install them for the current user. Note that while it will also add the
# cargo bin directory to the PATH, that won't be used by Bentley Make, which instead uses cargo.sh.

curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
~/.cargo/bin/cargo install cargo-c
if [ "$(uname -s)" = "Darwin" ]; then
    ~/.cargo/bin/rustup target add aarch64-apple-ios
fi
