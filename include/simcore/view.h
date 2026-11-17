#pragma once

#include <cstddef>
#include <iterator>
#include <typeinfo>

#include <simcore/registry.h>

namespace simcore {

/// @brief Forward iterator over the live entities of one concrete type.
///
/// The iterator walks the registry's slot storage by index, so slot reallocation
/// caused by spawn() during traversal does not invalidate it. Entities that are
/// despawned while iterating are skipped.
///
/// @tparam T Concrete entity type produced by the iterator.
template <class T>
class ViewIterator {
public:
    /// @brief Iterator category (forward iterator).
    using iterator_category = std::forward_iterator_tag;

    /// @brief Type produced when dereferencing the iterator.
    using value_type = T;

    /// @brief Type of an iterator difference.
    using difference_type = std::ptrdiff_t;

    /// @brief Pointer type returned by operator->().
    using pointer = T*;

    /// @brief Reference type returned by operator*().
    using reference = T&;

    /// @brief Constructs an end-like iterator that belongs to no registry.
    ViewIterator() noexcept = default;

    /// @brief Accesses the current entity.
    /// @return Mutable reference to the current entity.
    [[nodiscard]] reference operator*() const noexcept {
        return static_cast<EntityHolder<T>*>(registry_->entity_base_at(index_))->value();
    }

    /// @brief Accesses the current entity.
    /// @return Pointer to the current entity.
    [[nodiscard]] pointer operator->() const noexcept {
        return &static_cast<EntityHolder<T>*>(registry_->entity_base_at(index_))->value();
    }

    /// @brief Advances to the next matching entity.
    /// @return Reference to this iterator.
    ViewIterator& operator++() noexcept {
        ++index_;
        skip_to_match();
        return *this;
    }

    /// @brief Advances to the next matching entity.
    /// @return Copy of the iterator before advancing.
    ViewIterator operator++(int) noexcept {
        ViewIterator copy = *this;
        ++index_;
        skip_to_match();
        return copy;
    }

    /// @brief Compares two iterators.
    /// @param lhs Left-hand iterator.
    /// @param rhs Right-hand iterator.
    /// @return true when both iterators refer to the same position in the same registry.
    [[nodiscard]] friend bool operator==(const ViewIterator& lhs,
                                         const ViewIterator& rhs) noexcept {
        return lhs.registry_ == rhs.registry_ && lhs.index_ == rhs.index_;
    }

private:
    friend class View<T>;
    friend class Registry;

    /// @brief Builds an iterator positioned at a slot index.
    /// @param registry Registry being traversed.
    /// @param index Starting slot index.
    ViewIterator(Registry* registry, std::size_t index) noexcept
        : registry_(registry), index_(index) {
        skip_to_match();
    }

    /// @brief Advances until a live entity of type T or the end is reached.
    void skip_to_match() noexcept {
        if (registry_ == nullptr) {
            return;
        }
        const std::size_t count = registry_->slot_count();
        while (index_ < count && !registry_->slot_matches(index_, typeid(T))) {
            ++index_;
        }
    }

    Registry* registry_{nullptr};
    std::size_t index_{0};
};

/// @brief Range over the live entities of one concrete type.
///
/// A view does not own entities; it only filters the registry. Holding a view
/// keeps deferred destruction pending, which guarantees that despawn() during
/// traversal never invalidates the traversal itself.
///
/// @tparam T Concrete entity type to iterate.
///
/// @code
/// for (auto& particle : registry.view<Particle>()) {
///     particle.update(0.016);
/// }
/// @endcode
template <class T>
class View {
public:
    /// @brief Iterator type produced by begin() and end().
    using Iterator = ViewIterator<T>;

    /// @brief Copying is disabled; a view guards a live traversal.
    View(const View&) = delete;

    /// @brief Copy assignment is disabled; a view guards a live traversal.
    View& operator=(const View&) = delete;

    /// @brief Transfers the view to another object.
    /// @param other View being moved from.
    View(View&& other) noexcept : registry_(other.registry_) { other.registry_ = nullptr; }

    /// @brief Move assignment is disabled; a view guards a live traversal.
    View& operator=(View&&) = delete;

    /// @brief Ends the traversal and releases deferred entities if this was the last one.
    ~View() {
        if (registry_ != nullptr) {
            registry_->end_iteration();
        }
    }

    /// @brief Returns an iterator to the first live entity of type T.
    /// @return Iterator to the first match, or end() when there is none.
    [[nodiscard]] Iterator begin() const noexcept { return Iterator(registry_, 0); }

    /// @brief Returns the past-the-end iterator.
    /// @return Iterator positioned after the last slot.
    [[nodiscard]] Iterator end() const noexcept {
        return Iterator(registry_, registry_ != nullptr ? registry_->slot_count() : 0);
    }

private:
    friend class Registry;

    /// @brief Starts a traversal on a registry.
    /// @param registry Registry to traverse.
    explicit View(Registry& registry) noexcept : registry_(&registry) {
        registry_->begin_iteration();
    }

    Registry* registry_{nullptr};
};

/// @brief Creates a view over all live entities of type T.
/// @tparam T Concrete entity type to iterate.
/// @return View over the matching entities.
template <class T>
[[nodiscard]] View<T> Registry::view() noexcept {
    static_assert(Entity<T>,
                  "simcore::Registry::view<T>: T does not satisfy the simcore::Entity concept.");
    return View<T>(*this);
}

}  // namespace simcore
