// simcore example: particles moving inside a bounded box.
//
// The example demonstrates the four core APIs: spawn(), view(), despawn() and
// run(). It is deliberately domain specific; simcore itself knows nothing
// about particles or boxes.

#include <simcore/simcore.h>

#include <cstddef>
#include <cstdio>
#include <vector>

namespace {

/// @brief A moving particle that expires after a fixed lifetime.
struct Particle {
    Particle(double x, double vx, double lifetime) : x(x), vx(vx), lifetime(lifetime) {}

    void update(double dt) {
        x += vx * dt;
        lifetime -= dt;
    }

    double x;
    double vx;
    double lifetime;
};

/// @brief A box that counts the particles currently inside its bounds.
struct BoundedBox {
    BoundedBox(simcore::Registry& registry, double half_width)
        : registry_(&registry), half_width_(half_width) {}

    void update(double) {
        ++steps_;
        inside_ = 0;
        for (const auto& particle : registry_->view<Particle>()) {
            if (particle.x > -half_width_ && particle.x < half_width_) {
                ++inside_;
            }
        }
    }

    [[nodiscard]] int steps() const { return steps_; }
    [[nodiscard]] int inside() const { return inside_; }

private:
    simcore::Registry* registry_;
    double half_width_;
    int steps_{0};
    int inside_{0};
};

constexpr double kHalfWidth = 5.0;

void print_state(const char* label, const simcore::Registry& registry,
                 const simcore::Stepper& stepper) {
    std::printf("%-26s | elapsed=%5.2f s | entities=%zu\n", label, stepper.elapsed(),
                registry.entity_count());
}

}  // namespace

int main() {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);

    std::puts("simcore particle demo");
    std::puts("---------------------");

    auto box = registry.spawn<BoundedBox>(registry, kHalfWidth);

    std::vector<simcore::EntityHandle<Particle>> particles;
    particles.push_back(registry.spawn<Particle>(-4.0, 1.5, 0.8));
    particles.push_back(registry.spawn<Particle>(-2.0, 2.5, 1.4));
    particles.push_back(registry.spawn<Particle>(0.0, -1.0, 2.0));
    particles.push_back(registry.spawn<Particle>(2.0, 0.5, 0.35));
    particles.push_back(registry.spawn<Particle>(4.0, -2.0, 1.1));

    print_state("after spawning", registry, stepper);

    // 1.0 s is not a multiple of 0.3 s, so the last step uses the 0.1 s remainder.
    stepper.run(1.0, 0.3);
    print_state("after run(1.0, 0.3)", registry, stepper);

    // Remove expired particles while traversing the view; this is safe by design.
    std::size_t removed = 0;
    for (auto& particle : registry.view<Particle>()) {
        if (particle.lifetime > 0.0) {
            continue;
        }
        for (auto& handle : particles) {
            if (handle.get() == &particle) {
                registry.despawn(handle);
                ++removed;
            }
        }
    }
    std::printf("removed %zu expired particles during view traversal\n", removed);
    print_state("after cleanup", registry, stepper);

    stepper.run(0.5, 0.1);
    print_state("after run(0.5, 0.1)", registry, stepper);

    std::size_t alive = 0;
    double farthest = 0.0;
    for (const auto& particle : registry.view<Particle>()) {
        ++alive;
        const double distance = particle.x < 0.0 ? -particle.x : particle.x;
        if (distance > farthest) {
            farthest = distance;
        }
    }

    std::puts("---------------------");
    std::printf("simulated time : %.2f s in %d fixed steps\n", stepper.elapsed(), box->steps());
    std::printf("particles alive: %zu (removed %zu)\n", alive, removed);
    std::printf("inside box now : %d\n", box->inside());
    std::printf("farthest |x|   : %.2f (half width %.1f)\n", farthest, kHalfWidth);
    return 0;
}
