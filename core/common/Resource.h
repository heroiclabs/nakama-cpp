#pragma once

namespace Nakama {
struct disarmed_t {};

inline constexpr disarmed_t disarmed{};

// like unique_ptr but for non-pointer types
template <typename T, int (*d)(T)> class Resource {
public:
  explicit Resource(T v) : r(v) {}
  explicit Resource(disarmed_t) : r(), armed(false) {}
  ~Resource() { armed ? d(r) : 0; }

  Resource(Resource&) = delete;
  Resource(const Resource&) = delete;
  Resource& operator=(const Resource&) = delete;
  Resource& operator=(Resource&&) = delete;

  Resource(Resource&& o) noexcept : r(o.r), armed(o.armed) {
    o.armed = false;
    o.r = T();
  }

  explicit operator bool() const { return armed; }
  T& get() { return r; }

  void reset(T v) {
    armed ? d(r) : 0;
    armed = true;
    r = v;
  }

  void reset(disarmed_t) {
    armed ? d(r) : 0;
    armed = false;
    r = T();
  }

  // Call this only after making sure the resource is invalid or you handle destruction manually. This will not call the
  // delete function on the resource!
  void disarm() { armed = false; }

private:
  T r;
  bool armed = true;
};
} // namespace Nakama
