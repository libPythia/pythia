#pragma once

struct LeafId {
  public:
    using underlying_t = unsigned int;

  public:
    constexpr LeafId(underlying_t v)
          : _value(v) {}

    auto value() const { return _value; }

    auto operator<(LeafId o) const { return _value < o._value; }
    auto operator==(LeafId o) const { return _value == o._value; }
    auto operator!=(LeafId o) const { return _value != o._value; }

  private:
    underlying_t _value;
};
