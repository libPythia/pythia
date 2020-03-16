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

inline auto value(LeafId id) { return id.value(); }

struct NodeId {
  public:
    using underlying_t = unsigned int;

  private:
    constexpr static auto invalid_value = static_cast<underlying_t>(-1);

  public:
    constexpr NodeId(underlying_t v)
          : _value(v) {}

    auto value() const { return _value; }

    auto operator<(NodeId o) const { return _value < o._value; }
    auto operator==(NodeId o) const { return _value == o._value; }
    auto operator!=(NodeId o) const { return _value != o._value; }

    static auto invalid() { return NodeId(invalid_value); }
    auto isValid() const { return _value != invalid_value; }

  private:
    underlying_t _value;
};

