#include <array>
#define EVAL_PARAM(name, value) const int name = value;
#define EVAL_PARAM_ARRAY(size, name, ...) const std::array<int, size> name = { __VA_ARGS__ };
