// This file is part of corral, a lightweight C++20 coroutine library.
//
// Copyright (c) 2024-2025 Hudson River Trading LLC
// <opensource@hudson-trading.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// SPDX-License-Identifier: MIT

#pragma once
#include "config.h"

namespace corral {
namespace detail {}

using Handle = detail::CoroutineHandle<void>;

class EventLoopID {
    const void* id_;

  public:
    explicit constexpr EventLoopID(const void* id) : id_(id) {}
    explicit constexpr operator const void*() const { return id_; }

    const void* get() const noexcept { return id_; }

    constexpr bool operator==(const EventLoopID& other) const = default;
    constexpr auto operator<=>(const EventLoopID& other) const = default;
};


/// Specialize this to adapt corral to a new eventLoop type.
template <class T, class = void> struct EventLoopTraits {
    static_assert(!std::is_same_v<T, T>, "event loop type unknown to corral");

    /// Returns a value identifying the event loop.
    /// Traits for any sort of wrappers should return ID
    /// of the wrapped event loop.
    static EventLoopID eventLoopID(T&);

    /// Runs the event loop.
    static void run(T&);

    /// Tells the event loop to exit.
    /// run() should return shortly thereafter.
    static void stop(T&);

    /// Tests whether we're inside this event loop.
    /// Only used for preventing nested runs using the same event loop;
    /// if no suitable implementation is available, may always return false,
    /// or leave undefined for the same effect.
    static bool isRunning(T&) noexcept;
};


/// Specialize this class for an event loop type to allow ThreadPool
/// to be constructed around this event loop.
template <class EventLoopT> class ThreadNotification {
  public:
    /// Prepare to be able to invoke `fn(arg)` on the current thread, which is
    /// running the given event loop. The same arguments are passed to this
    /// constructor and to `post()` to allow for a variety of notification
    /// architectures.
    ThreadNotification(EventLoopT&, void (*fn)(void*), void* arg) {
        static_assert(!std::is_same_v<EventLoopT, EventLoopT>,
                      "Specialize this class for your event loop type "
                      "to allow constructing ThreadPool around the loop");
    }

    /// Called from worker threads to wake up the main thread.
    ///
    /// Arrange for `fn(arg)` to be called on the thread that created the
    /// ThreadNotification object, which was running `eventLoop` when it was
    /// created.
    /// Multiple `post()` calls may be coalesced into a single call to `fn`.
    ///
    /// The `eventLoop`, `fn` and `args` are guaranteed match those passed to
    /// the object constructor (to facilitate stateless implementations).
    void post(EventLoopT& eventLoop, void (*fn)(void*), void* arg) noexcept;
};

template <class T> class Task; // forward declaration

} // namespace corral
