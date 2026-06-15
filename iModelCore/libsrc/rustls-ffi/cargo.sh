#!/bin/bash

if type -P cargo >/dev/null 2>&1; then
    cargo "$@"
elif type -P ~/.cargo/bin/cargo >/dev/null 2>&1; then
    # Run the user-installed cargo with all arguments passed through.
    ~/.cargo/bin/cargo "$@"
    if [ $? -ne 0 ]; then
        echo "Error: cargo is installed, but one of the dependencies that we need may be" >&2
        echo "missing. Please set INSTALL_RUST_CARGO to install it for the current user." >&2
        exit 1
    fi
else
    echo "Error: cargo is not installed. Please set INSTALL_RUST_CARGO to install it for the current user." >&2
    exit 1
fi
