# Download specific mdbook version to build mdbook with
mkdir bin
curl -sSL https://github.com/rust-lang/mdBook/releases/download/v0.4.18/mdbook-v0.4.18-x86_64-unknown-linux-gnu.tar.gz | tar -xz --directory=bin
bin/mdbook build