#!/bin/sh

# Run the user-installed cargo with all arguments passed through. Our install-rust.sh script does not modify the PATH,
# so we need to call cargo directly from the ~/.cargo/bin directory.
~/.cargo/bin/cargo $*
