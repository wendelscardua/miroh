#pragma once

class Set {
public:
  Set *parent;

  Set() : parent(this) {}
  Set *representative() {
    Set *temp = this;
    while (temp != temp->parent) {
      temp = temp->parent;
    }
    return temp;
  }
  void join(Set *other) {
    Set *x = this->representative();
    Set *y = other->representative();
    if (x != y)
      y->parent = x;
  }
};