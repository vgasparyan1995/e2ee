# E2EE

E2EE is an end to end encryption engine providing basic functionality with a user-friendly interface. Encryption is based on [Diffie-Hellman key exchange](https://en.m.wikipedia.org/wiki/Diffie–Hellman_key_exchange) algorithm, using 2048 bit prime number. Random values are generated with [extreme value distribution](https://en.m.wikipedia.org/wiki/Generalized_extreme_value_distribution), and the location of distribution is chosen empirically to escape both too short and too long calculations.
The main `Engine.h` header file defines a singleton class called `E2EE::Engine`.
It provides 6 functions:
- prepareToPairWith
- getKeyToSend
- setReceivedKey
- getHash
- serialize
- deserialize
