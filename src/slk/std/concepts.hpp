
import std;

namespace slk {

template <typename T>
concept EnumerationType = std::is_enum_v<T>;

}
