# MGPCL #
## What is it? ##
This is a **static** general purpose C++0x library I've been working on. It works on both Linux and Windows, and provides a few
basic things you might need in a C++ project, like basic containers (Lists, Maps), threading primitives, message boxes, internet basics (sockets and HTTP requests), etc... It also has OpenSSL wrappers for crypto and well... SSL...

## License? ##
MIT/X11.

## Dependencies? ##
 * OpenSSL (for crypto and SSL)
 * gtk3 (Linux only; for GUI)

Both of these dependencies can be disabled through CMake using `MGPCL_ENABLE_GUI` and `MGPCL_ENABLE_SSL`

## Building ##
MGPCL uses CMake under Linux. It may work on Windows but you'll have to add OpenSSL manually to the build; it is better to use the provided VS solution file.
Anyway, on Linux, simply run:
```
git clone https://github.com/montoyo/mgpcl.git
cd mgpcl
cmake .
make
```

## Why not boost? ##
Boost would be indeed a better choice. One could say I re-invented the wheel. But I wanted to make my own library, where I can easily add whatever I want, with my coding style. I also believe I learned a few things by writing this library, and I'm pretty sure that knowing how your stuff works makes you 'performance-aware' (like, what is slow, under which circumstances, what would be better etc...)
__TL;DR:__ Not the best, but this is what I want to work with.

## Functionality list ##
 * Containers (String, List, HashMap, FlatMap, Queue, Variant, SharedPtr, Bitfield)
 * Threading (Thread, Mutex, Atomic, Cond, RWLock)
 * Math (Complex, FFT, Vectors, Matrix, Quaternion, Ray, Shapes)
 * IO with Java-like stream system
 * OS Interaction (File & dirs, Processes, SerialIO, Shared objects, Console/terminal utils, CPU Infos)
 * Parsing misc (JSON, INI-like configs, Program args)
 * Logging (Basic logger, logging over network)
 * Internet (Sockets, SSLSockets, TCPServer, TCPClient, Packet system, HTTP requests)
 * GUI (Message boxes, and **very basic** Window management)
 * Crypto (OpenSSL wrappers for Big Numbers, HMAC, AES, RSA, SHA)
 * and others (Date, Nifty counter macros, Random, basic signal & slot system, Singletons, Time handling, CRC32, Base64, etc..)

## TODO List ##
 * Patterns (aka Regexp)
 * ZLib and LZ4 compression I/O streams
 * Networking: Complete UDP model
 * Async serial I/O
 * SSL Certificate Management
 * Noise techniques (Perlin?)
 * Shapes: Cylinder & cube
 * RoundRobin class
 * Faster & better FFT
 * DLL/SO build
 * **The aim is to completely remove the stdlib/stl dependency**
