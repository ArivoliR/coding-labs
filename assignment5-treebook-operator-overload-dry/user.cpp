#include "user.h"
#include <ostream>
#include <string>

/**
 * Creates a new User with the given name and no friends.
 */
User::User(const std::string &name) : _name(name) {}

/**
 * Adds a friend to this User's list of friends.
 * @param name The name of the friend to add.
 */
void User::add_friend(const std::string &name) { _friends.add(name); }

/**
 * Returns the name of this User.
 */
std::string User::get_name() const { return _name; }

/**
 * Returns the number of friends this User has.
 */
size_t User::size() const { return _friends.size(); }

/**
 * Sets the friend at the given index to the given name.
 * @param index The index of the friend to set.
 * @param name The name to set the friend to.
 */
void User::set_friend(size_t index, const std::string &name) {
  _friends.set(index, name);
}

/**
 * STUDENT TODO:
 * The definitions for your custom operators and special member functions will
 * go here!
 */

User::~User() = default;

User::User(const User &user) = default;

User &User::operator=(const User &user) = default;

User &User::operator+=(User &rhs) {
  add_friend(rhs._name);
  rhs.add_friend(_name);
  return *this;
}

bool User::operator<(const User &rhs) const { return _name < rhs._name; }

std::ostream &operator<<(std::ostream &os, const User &user) {
  return os << "User(name=" << user._name << ", friends=" << user._friends
            << ")";
}
