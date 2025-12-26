#ifndef WHITELIST_H
#define WHITELIST_H

#include <vector>
#include <unordered_set>
#include <string>

class Whitelist
{
public:
  Whitelist();
  Whitelist(const std::vector<std::string> &whitelist, bool active = true);
  Whitelist(const std::unordered_set<std::string> &whitelist, bool active = true);

  void addItem(const std::string &item);
  void removeItem(const std::string &item);
  void setActive(const bool active);

  bool inWhitelist(const std::string &item) const;
  bool isActive() const;

  std::string getWhiteListStr() const;

private:
  std::unordered_set<std::string> m_whitelist;
  bool m_isActive;
};

#endif