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

## Description

```
bool prepareToPairWith(const std::string& user);
```
Takes a username as an input parameter, generates a random number for `user`, stores it in temporary key storage. Returns false if the temporary storage already has a key for `user` or permanent storage has a hash for `user`. Otherwise, everything is fine and it returns true.

```
BigInteger getKeyToSend(const std::string& user) const;
```
Takes a username as an input parameter, returns corresponding (random) key from the temporary storage. If there is no key generated for `user`, a default constructed `BigInteger` object is returned, which is equal to 0.

```
void setReceivedKey(const std::string& user, const BigInteger& key);
```
Takes a username and a `BigInteger` key as input parameters, returns nothing. If there is a generated key for `user` in temporary storage, it is removed from there, encrypted with `key` and the result is stored in the permanent storage. If there is no key for `user` in temporary storage, function does nothing.

```
BigInteger getHash(const std::string& user) const;
```
Takes a username as an input parameter, returns corresponding hash from the permanent storage. If there is no hash for `user`, a default constructed `BigInteger` object is returned, which is equal to 0.

```
ByteArray serialize() const;
```
Returns a byte array of temporary and permanent storages. It's purpose is to save and restore the state of engine.

```
bool deserialize(const ByteArray& data);
```
Takes a byte array (normally returned by `serialize()` function) and restores the state of engine.
