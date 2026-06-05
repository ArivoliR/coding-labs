#pragma once

#include <iostream>
#include <string>

class FriendList {
public:
  FriendList();
  ~FriendList();
  FriendList(const FriendList &flist);
  FriendList &operator=(const FriendList &flist);
  void add(const std::string &name);
  size_t size() const;
  void set(size_t index, const std::string &name);

  friend std::ostream &operator<<(std::ostream &os, const FriendList &flist);

private:
  std::string *_friends;
  size_t _size;
  size_t _capacity;

  void copy_helper(const FriendList &flist);
};
