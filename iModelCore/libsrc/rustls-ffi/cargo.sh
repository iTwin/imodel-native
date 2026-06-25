#!/bin/bash

if type -P cargo >/dev/null 2>&1; then
    cargo "$@"
elif type -P ~/.cargo/bin/cargo >/dev/null 2>&1; then
    # Run the user-installed cargo with all arguments passed through.
    ~/.cargo/bin/cargo "$@"
else
    echo "Error: cargo is not installed. Please set INSTALL_RUST_CARGO to install it for the current user." >&2
    exit 1
fi
