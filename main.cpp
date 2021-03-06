#include "Engine.h"

int main()
{
    std::string username1("user1");
    std::string username2("user2");
    auto engine = E2EE::Engine::get_instance();
    if (!engine->prepareToPairWith(username1)) {
        std::cout << "Already paired." << std::endl;
        return -1;
    }
    if (!engine->prepareToPairWith(username2)) {
        std::cout << "Already paired." << std::endl;
        return -1;
    }
    const auto key1 = engine->getKeyToSend(username1);
    const auto key2 = engine->getKeyToSend(username2);
    engine->setReceivedKey(username1, key2);
    engine->setReceivedKey(username2, key1);

    const auto hash1 = engine->getHash(username1);
    const auto hash2 = engine->getHash(username2);

    std::cout << "Hash1: " << hash1.to_string() << std::endl;
    std::cout << "Hash2: " << hash2.to_string() << std::endl;
    std::cout << "Are hashes equal: " << (hash1 == hash2) << std::endl;

    auto data = engine->serialize();
    if (!engine->deserialize(data)) {
        std::cout << "Deserialization failed." << std::endl;
        return -1;
    }
    const auto new_hash1 = engine->getHash(username1);
    const auto new_hash2 = engine->getHash(username2);
    std::cout << "New hash1: " << new_hash1.to_string() << std::endl;
    std::cout << "New hash2: " << new_hash2.to_string() << std::endl;
    std::cout << "Are hashes equal(1): " << (hash1 == new_hash1) << std::endl;
    std::cout << "Are hashes equal(2): " << (hash2 == new_hash2) << std::endl;
}
