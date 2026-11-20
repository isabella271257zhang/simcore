#include <simcore/stepper.h>

#include <cstddef>
#include <stdexcept>

namespace simcore {

Stepper::Stepper(Registry& registry) noexcept : registry_(&registry) {}

void Stepper::run(double duration, double fixed_step) {
    if (!(duration >= 0.0)) {
        throw std::invalid_argument("simcore::Stepper::run: duration must be non-negative");
    }
    if (!(fixed_step > 0.0)) {
        throw std::invalid_argument("simcore::Stepper::run: fixed_step must be positive");
    }
    // Absorbs floating-point drift so that an exact multiple of fixed_step does
    // not produce a spurious near-zero final step.
    const double tolerance = fixed_step * 1e-9;
    double remaining = duration;
    while (remaining > fixed_step + tolerance) {
        step(fixed_step);
        remaining -= fixed_step;
    }
    if (remaining > tolerance) {
        step(remaining);
    }
}

void Stepper::step(double dt) {
    if (!(dt >= 0.0)) {
        throw std::invalid_argument("simcore::Stepper::step: dt must be non-negative");
    }
    registry_->begin_iteration();
    try {
        const std::size_t count = registry_->slots_.size();
        for (std::size_t index = 0; index < count; ++index) {
            const Registry::Slot& slot = registry_->slots_[index];
            if (!slot.alive) {
                continue;
            }
            slot.entity->update(dt);
        }
    } catch (...) {
        registry_->end_iteration();
        throw;
    }
    registry_->end_iteration();
    registry_->events().dispatch();
    elapsed_ += dt;
}

double Stepper::elapsed() const noexcept { return elapsed_; }

}  // namespace simcore
