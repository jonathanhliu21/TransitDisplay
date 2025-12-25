#include "Whitelist.h"

#include <vector>
#include <unordered_set>
#include <string>

Whitelist::Whitelist() : m_isActive{false} {}

Whitelist::Whitelist(const std::vector<std::string> &whitelist, bool active)
    : m_whitelist(whitelist.begin(), whitelist.end()), m_isActive{active} {}

Whitelist::Whitelist(const std::unordered_set<std::string> &whitelist, bool active)
    : m_whitelist(whitelist), m_isActive{active} {}

void Whitelist::addItem(const std::string &item)
{
  m_whitelist.insert(item);
}

bool Whitelist::inWhitelist(const std::string &item) const
{
  return m_whitelist.find(item) != m_whitelist.end();
}

void Whitelist::removeItem(const std::string &item)
{
  m_whitelist.erase(item);
}

void Whitelist::setActive(const bool active) { m_isActive = active; }

bool Whitelist::isActive() const { return m_isActive; }