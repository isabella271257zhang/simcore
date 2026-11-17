#pragma once

#include <concepts>
#include <cstdint>
#include <typeinfo>
#include <utility>

namespace simcore {

/// @brief Stable identifier of an entity inside a Registry.
using EntityId = std::uint64_t;

/// @brief Constrains a type to be usable as a simulation entity.
///
/// A type models Entity when it exposes a member function callable as
/// `void update(double dt)`.
///
/// @tparam T Candidate entity type.
///
/// @code
/// struct Particle {
///     void update(double dt) { position += velocity * dt; }
/// };
/// static_assert(simcore::Entity<Particle>);
/// @endcode
template <class T>
concept Entity = requires(T& entity, double dt) {
    { entity.update(dt) } -> std::same_as<void>;
};

/// @brief Type-erased base class of every stored entity.
///
/// Registry stores entities through this interface so that entities of
/// different concrete types can share one container.
class EntityBase {
public:
    /// @brief Constructs the type-erased base object.
    EntityBase() = default;

    /// @brief Copying is disabled; entities are owned by a Registry.
    EntityBase(const EntityBase&) = delete;

    /// @brief Copy assignment is disabled; entities are owned by a Registry.
    EntityBase& operator=(const EntityBase&) = delete;

    /// @brief Moving is disabled so stored entity addresses stay stable.
    EntityBase(EntityBase&&) = delete;

    /// @brief Move assignment is disabled so stored entity addresses stay stable.
    EntityBase& operator=(EntityBase&&) = delete;

    /// @brief Destroys the stored entity.
    virtual ~EntityBase() = default;

    /// @brief Advances the entity by a time step.
    /// @param dt Step length in seconds.
    virtual void update(double dt) = 0;

    /// @brief Reports the runtime type of the concrete entity.
    /// @return Type information of the stored entity type.
    [[nodiscard]] virtual const std::type_info& type() const noexcept = 0;
};

/// @brief Adapter that stores one concrete entity behind EntityBase.
/// @tparam T Concrete entity type; must satisfy simcore::Entity.
template <class T>
class EntityHolder final : public EntityBase {
public:
    /// @brief Constructs the stored entity in place.
    /// @tparam Args Argument types forwarded to T's constructor.
    /// @param args Arguments forwarded to T's constructor.
    template <class... Args>
    explicit EntityHolder(Args&&... args) : value_(std::forward<Args>(args)...) {}

    /// @brief Forwards the time step to the stored entity's update().
    /// @param dt Step length in seconds.
    void update(double dt) override { value_.update(dt); }

    /// @brief Reports the stored entity's runtime type.
    /// @return Type information of T.
    [[nodiscard]] const std::type_info& type() const noexcept override { return typeid(T); }

    /// @brief Accesses the stored entity.
    /// @return Reference to the stored entity.
    [[nodiscard]] T& value() noexcept { return value_; }

private:
    T value_;
};

}  // namespace simcore
