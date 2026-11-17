#include <simcore/simcore.h>

#include <gtest/gtest.h>

namespace {

constexpr double kEpsilon = 1e-9;

/// @brief Entity that records the time steps it receives.
struct Probe {
    void update(double dt) {
        ++updates;
        last_dt = dt;
        total += dt;
    }

    int updates{0};
    double last_dt{0.0};
    double total{0.0};
};

}  // namespace

// T1: a fixed-step run advances the total simulated time by exactly `duration`
// (within 1e-9) and delivers that same total to the entities.
TEST(StepperTest, FixedStepRunMatchesElapsedTime) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    auto probe = registry.spawn<Probe>();

    stepper.run(1.0, 0.1);

    EXPECT_EQ(probe->updates, 10);
    EXPECT_NEAR(stepper.elapsed(), 1.0, kEpsilon);
    EXPECT_NEAR(probe->total, 1.0, kEpsilon);
}

// T1 (continued): when duration is not a multiple of the fixed step, the final
// frame receives the remainder instead of a full step.
TEST(StepperTest, FinalFrameReceivesRemainder) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    auto probe = registry.spawn<Probe>();

    stepper.run(0.25, 0.1);

    EXPECT_EQ(probe->updates, 3);
    EXPECT_NEAR(stepper.elapsed(), 0.25, kEpsilon);
    EXPECT_NEAR(probe->total, 0.25, kEpsilon);
    EXPECT_NEAR(probe->last_dt, 0.05, 1e-12);
}

// T1 (continued): explicit steps accumulate into elapsed() as well.
TEST(StepperTest, ExplicitStepsAccumulate) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);
    auto probe = registry.spawn<Probe>();

    stepper.step(0.5);
    stepper.step(0.25);

    EXPECT_EQ(probe->updates, 2);
    EXPECT_NEAR(stepper.elapsed(), 0.75, kEpsilon);
}
