#include "friend_list.h"
#include <iterator>
#include <ostream>
#include <string>

FriendList::FriendList() : _friends(nullptr), _size(0), _capacity(0) {}

void FriendList::add(const std::string &name) {
  if (_size == _capacity) {
    _capacity = 2 * _capacity + 1;
    std::string *newFriends = new std::string[_capacity];
    for (size_t i = 0; i < _size; ++i) {
      newFriends[i] = _friends[i];
    }
    delete[] _friends;
    _friends = newFriends;
  }

  _friends[_size++] = name;
}

void FriendList::copy_helper(const FriendList &flist) {
  _friends = nullptr;

  if (_capacity) {
    _friends = new std::string[_capacity];
    for (auto i = 0; i < _size; i++) {
      _friends[i] = flist._friends[i];
    }
  }
}

size_t FriendList::size() const { return _size; }

void FriendList::set(size_t index, const std::string &name) {
  _friends[index] = name;
}

FriendList::~FriendList() { delete[] _friends; }

FriendList::FriendList(const FriendList &flist)
    : _friends(nullptr), _size(flist._size), _capacity(flist._capacity) {
  copy_helper(flist);
}
FriendList &FriendList::operator=(const FriendList &flist) {
  if (this == &flist) {
    return *this;
  }

  delete[] _friends;

  _size = flist._size;
  _capacity = flist._capacity;

  copy_helper(flist);

  return *this;
}

std::ostream &operator<<(std::ostream &os, const FriendList &flist) {
  os << "[";
  for (size_t i = 0; i < flist._size; ++i) {
    if (i) {
      os << ", ";
    }
    os << flist._friends[i];
  }
  return os << "]";
}
