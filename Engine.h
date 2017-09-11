#include <random>
#include <string>
#include <unordered_map>

#include "BigInteger.h"
#include "Macros.h"
#include "Utility.h"

namespace E2EE {

class Engine
{
    SINGLETON_DECL(Engine)

public:
    bool prepareToPairWith(const std::string& user);
    BigInteger getKeyToSend(const std::string& user) const;
    void setReceivedKey(const std::string& user, const BigInteger& key);
    BigInteger getHash(const std::string& user) const;
    ByteArray serialize() const;
    bool deserialize(const ByteArray& data);

private:
    int randomInteger();

private:
    using UserToHash = std::unordered_map<std::string, BigInteger>;

    UserToHash                              m_temporary;
    UserToHash                              m_permanent;
    BigInteger                              m_prime;
    BigInteger                              m_generator;
    std::mt19937                            m_gen;
    std::extreme_value_distribution<double> m_dis;
};

} // namespace E2EE
