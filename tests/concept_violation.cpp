// T5: this translation unit must NOT compile.
//
// NotAnEntity has no member callable as `void update(double dt)`, so
// Registry::spawn must reject it with a readable static_assert diagnostic.
// tests/expect_compile_failure.cmake builds this target and checks the output.

#include <simcore/simcore.h>

namespace {

struct NotAnEntity {
    void tick(double) {}
};

}  // namespace

int main() {
    simcore::Registry registry;
    auto handle = registry.spawn<NotAnEntity>();
    (void)handle;
    return 0;
}
