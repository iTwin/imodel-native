#!/bin/bash

if type -P cargo >/dev/null 2>&1; then
    cargo "$@"
else
    # Run the user-installed cargo with all arguments passed through.
    ~/.cargo/bin/cargo "$@"
fi
