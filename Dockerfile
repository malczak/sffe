# Minimal Alpine image for building, testing and developing sffe.
FROM alpine:3.20

# build-base -> gcc, make, musl-dev, binutils, ...
# check-dev  -> Check unit-testing framework, headers and pkg-config metadata
# pkgconf    -> provides pkg-config, used by the Makefile to locate Check
RUN apk add --no-cache build-base check-dev pkgconf

WORKDIR /sffe
COPY . .

# Default command: build the library and run the test suite.
CMD ["make", "test"]
