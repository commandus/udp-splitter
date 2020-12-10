# UDP splitter

Simple program to forward incoming UDP packets to N many targets.

## Example

```
udp-splitter 84.237.104.128:2000 84.237.104.128:2001 84.237.104.128:2002
```

## Build

automake:

```
autogen.sh
./confiugure
make
sudo make install
```

cmake (Clang):

```
mkdir build
cd build
export CC=/usr/bin/clang;export CXX=/usr/bin/clang++;cmake ..
make
```
