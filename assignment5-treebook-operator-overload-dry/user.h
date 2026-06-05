/*
 * Assignment 4: TreeBook
 * Adapted by Tinkercademy from Stanford CS106L Assignment 5
 * (originally by Fabio Ibanez, with modifications by Jacob Roberts-Baca).
 */

#include <iostream>
#include <string>

class User {
public:
  User(const std::string &name);
  void add_friend(const std::string &name);
  std::string get_name() const;
  size_t size() const;
  void set_friend(size_t index, const std::string &name);

  /**
   * STUDENT TODO:
   * Your custom operators and special member functions will go here!
   */

  ~User();                               // destructor
  User(const User &user);                // copy ctor
  User &operator=(const User &user);     // copy assignment
  User(User &&user) = delete;            // move ctor
  User &operator=(User &&user) = delete; // move assignment

  friend std::ostream &operator<<(std::ostream &os, const User &user);

private:
  std::string _name;
  std::string *_friends;
  size_t _size;
  size_t _capacity;

  void copy_helper(const User &user);
};
