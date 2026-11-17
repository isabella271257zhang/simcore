#include <simcore/simcore.h>

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

namespace {

/// @brief Entity used to observe scheduling and view behaviour.
struct TestParticle {
    explicit TestParticle(int id) : id(id) {}

    void update(double dt) {
        ++updates;
        last_dt = dt;
    }

    int id;
    int updates{0};
    double last_dt{0.0};
};

/// @brief Second entity type, proving that heterogeneous types coexist.
struct TestBox {
    explicit TestBox(double size) : size(size) {}

    void update(double dt) {
        ++updates;
        last_dt = dt;
    }

    double size;
    int updates{0};
    double last_dt{0.0};
};

}  // namespace

// T2: multiple entity types coexist in one registry, share the same time step,
// and each view yields only entities of the requested type.
TEST(RegistryTest, MultipleEntityTypesCoexist) {
    simcore::Registry registry;
    simcore::Stepper stepper(registry);

    auto particle = registry.spawn<TestParticle>(1);
    auto box = registry.spawn<TestBox>(2.0);
    EXPECT_EQ(registry.entity_count(), 2U);

    stepper.step(0.25);
    EXPECT_EQ(particle->updates, 1);
    EXPECT_EQ(box->updates, 1);
    EXPECT_DOUBLE_EQ(particle->last_dt, 0.25);
    EXPECT_DOUBLE_EQ(box->last_dt, 0.25);

    std::size_t particles = 0;
    for (auto& entity : registry.view<TestParticle>()) {
        ++particles;
        EXPECT_EQ(entity.id, 1);
    }
    std::size_t boxes = 0;
    for (auto& entity : registry.view<TestBox>()) {
        ++boxes;
        EXPECT_DOUBLE_EQ(entity.size, 2.0);
    }
    EXPECT_EQ(particles, 1U);
    EXPECT_EQ(boxes, 1U);
}

// T3: despawning entities inside a view traversal neither crashes nor
// invalidates the traversal, and entity_count() is correct afterwards.
TEST(RegistryTest, DespawnInsideViewKeepsTraversalValid) {
    simcore::Registry registry;
    std::vector<simcore::EntityHandle<TestParticle>> handles;
    for (int id = 0; id < 3; ++id) {
        handles.push_back(registry.spawn<TestParticle>(id));
    }

    std::size_t visited = 0;
    for (auto& particle : registry.view<TestParticle>()) {
        ++visited;
        // The current entity is still alive for the whole iteration.
        particle.last_dt = 42.0;
        for (auto& handle : handles) {
            if (handle.get() == &particle) {
                registry.despawn(handle);
            }
        }
    }

    EXPECT_EQ(visited, 3U);
    EXPECT_EQ(registry.entity_count(), 0U);
    for (auto& handle : handles) {
        EXPECT_FALSE(handle.valid());
    }
}

// T3 (continued): an entity despawned before the iterator reaches it is simply
// skipped, while the remaining entities are still visited normally.
TEST(RegistryTest, DespawnedUnvisitedEntityIsSkipped) {
    simcore::Registry registry;
    auto first = registry.spawn<TestParticle>(1);
    auto second = registry.spawn<TestParticle>(2);

    std::size_t visited = 0;
    for (auto& particle : registry.view<TestParticle>()) {
        (void)particle;
        ++visited;
        registry.despawn(second);
    }

    EXPECT_EQ(visited, 1U);
    EXPECT_EQ(registry.entity_count(), 1U);
    EXPECT_TRUE(first.valid());
    EXPECT_FALSE(second.valid());
}

// T4: defined behaviour for stale handles. Policy: despawn() is idempotent and
// stale handles (destroyed entity or reused slot) are ignored without effect.
TEST(RegistryTest, DespawnIsIdempotentForStaleHandles) {
    simcore::Registry registry;
    auto handle = registry.spawn<TestParticle>(1);

    registry.despawn(handle);
    EXPECT_EQ(registry.entity_count(), 0U);
    EXPECT_FALSE(handle.valid());

    registry.despawn(handle);
    registry.despawn(handle);
    EXPECT_EQ(registry.entity_count(), 0U);

    auto replacement = registry.spawn<TestParticle>(2);
    EXPECT_TRUE(replacement.valid());

    // The stale handle must not remove the entity that recycled its slot.
    registry.despawn(handle);
    EXPECT_EQ(registry.entity_count(), 1U);
    EXPECT_TRUE(replacement.valid());
}
